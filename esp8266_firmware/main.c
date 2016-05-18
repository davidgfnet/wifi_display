#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "user_config.h"
#include "uart_register.h"

#include "httpd.h"

// Global settings, stored in RTC memory (164 bytes)
char conf_essid[33] __attribute__ ((aligned (4)));  // Wireless ESSID
char conf_passw[33] __attribute__ ((aligned (4)));  // WPA/WEP password
char conf_hostn[33] __attribute__ ((aligned (4)));  // Server hostname
int  conf_port;                                     // Server port for the service
int  conf_path [65] __attribute__ ((aligned (4)));  // Server path to the servlet URI

struct espconn host_conn;
ip_addr_t host_ip;
esp_tcp host_tcp;


static volatile os_timer_t sleep_timer;
void ICACHE_FLASH_ATTR put_back_to_sleep() {
	os_printf("Goto sleep\n");
	system_deep_sleep_set_option(1); // Full wakeup!
	system_deep_sleep(10*60*1000*1000);
	os_printf("Gone to sleep\n");
}

void ICACHE_FLASH_ATTR power_gate_screen(int enable) {
	// Set GPIO2 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);

	// Set GPIO2 high!
	if (enable)
		gpio_output_set(BIT14, 0, BIT14, 0);
	else
		gpio_output_set(0, BIT14, BIT14, 0);
}

enum ScreenCommand {
	DispApSetup = 0x00,
	DispLostConnection = 0x01,
	DispLowBat = 0x02,
	DispSleepMode = 0x03,
	DispDHCPErr = 0x04,
	DispDNSErr = 0x05,
	DispConnectError = 0x06,
	DispImageError = 0x07
};

// Keep the WDT happy! :D
void ICACHE_FLASH_ATTR delay_ms(int ms) {
	while (ms > 0) {
		system_soft_wdt_feed();
		os_delay_us(10000);
		ms -= 10;
	}
}

void ICACHE_FLASH_ATTR screen_update(unsigned char screen_id) {
	// Update the screen!

	// Flush the FIFO, to make sure no characters are in the buffer
	UART_ResetFifo(0);
	delay_ms(10);

	// Turn off the daughter board
	power_gate_screen(1);

	// Wait around half a second for it to boot properly
	delay_ms(150);

	// Send the command through the UART
	uart_tx_one_char(screen_id, 0);

	// Wait around 4s for it to display the image properly
	delay_ms(3500);

	// Now turn screen off
	power_gate_screen(0);
}

void ICACHE_FLASH_ATTR data_received( void *arg, char *pdata, unsigned short len) {
	struct espconn *conn = arg;
	
	os_printf("%s: %d\n", __FUNCTION__, len);
	while (len--) {
		uart_tx_one_char(*pdata++, 0);
		if (!(len & 4095))
			system_soft_wdt_feed();
	}
	os_printf("%s: %d --\n", __FUNCTION__, len);
}

void nullwr(char c) {}

void ICACHE_FLASH_ATTR tcp_connected(void *arg)
{
	struct espconn *conn = arg;
	
	//os_printf( "%s\n", __FUNCTION__ );
	espconn_regist_recvcb(conn, data_received);

	// Prepare screen!
	//os_install_putc1((void *)nullwr);
	//system_set_os_print(0);
	UART_ResetFifo(0);
	power_gate_screen(1);
	delay_ms(150);
	uart_tx_one_char(0x40, 0);

	char buffer[256];
	os_sprintf(buffer, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", conf_path, conf_hostn);
	
	espconn_send(conn, buffer, os_strlen(buffer));
}

void ICACHE_FLASH_ATTR tcp_disconnected(void *arg) {
	struct espconn *conn = arg;
	
	// Disconnect board after 3.5 seconds
	delay_ms(3500);
	power_gate_screen(0);

	os_printf( "%s\n", __FUNCTION__ );
	wifi_station_disconnect();

	put_back_to_sleep();
}

void ICACHE_FLASH_ATTR tcp_error (void *arg, sint8 err) {
	// TCP error, just show a message and back to sleep!
	screen_update(DispConnectError);
	put_back_to_sleep();
}

void ICACHE_FLASH_ATTR dns_done_cb( const char *name, ip_addr_t *ipaddr, void *arg ) {
	struct espconn *conn = arg;
	
	os_printf("%s\n", __FUNCTION__);
	
	if (ipaddr == NULL) {
		os_printf("DNS lookup failed\n");
		wifi_station_disconnect();

		// Show the DNS error screen
		screen_update(DispDNSErr);

		// Go back to sleep!
		put_back_to_sleep();
	}
	else {
		os_printf("Connecting...\n" );
		
		conn->type = ESPCONN_TCP;
		conn->state = ESPCONN_NONE;
		conn->proto.tcp = &host_tcp;
		conn->proto.tcp->local_port = espconn_port();
		conn->proto.tcp->remote_port = conf_port;
		os_memcpy( conn->proto.tcp->remote_ip, &ipaddr->addr, 4 );

		espconn_regist_connectcb(conn, tcp_connected);
		espconn_regist_disconcb (conn, tcp_disconnected);
		espconn_regist_reconcb  (conn, tcp_error);
		
		espconn_connect(conn);
	}
}

void ICACHE_FLASH_ATTR wifi_callback( System_Event_t *evt ) {
	os_printf("%s: %d\n", __FUNCTION__, evt->event);
	
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED: {
			os_printf("connect to ssid %s, channel %d\n",
						evt->event_info.connected.ssid,
						evt->event_info.connected.channel);
			break;
		}

		case EVENT_STAMODE_DISCONNECTED: {
			os_printf("disconnect from ssid %s, reason %d\n",
						evt->event_info.disconnected.ssid,
						evt->event_info.disconnected.reason);		   
			break;
		}

		case EVENT_STAMODE_GOT_IP: {
			os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
						IP2STR(&evt->event_info.got_ip.ip),
						IP2STR(&evt->event_info.got_ip.mask),
						IP2STR(&evt->event_info.got_ip.gw));
			os_printf("\n");
			
			espconn_gethostbyname (&host_conn, conf_hostn, &host_ip, dns_done_cb);
			break;
		}

		case EVENT_STAMODE_DHCP_TIMEOUT: {
			// DHCP failed! Show error and retry later
			screen_update(DispDHCPErr);
			put_back_to_sleep();
			break;
		}
	}
}

int ICACHE_FLASH_ATTR recover_settings() {
	// Try to recover data from the RTC memory
	// We have 512 bytes of data we can use (regs 64 to 192)
	// We are gonna save two magic words in 64 and 191 and check for them
	// if the battery was gone, the value will be random

	uint32_t magic1, magic2;
	int r = system_rtc_mem_read(64,  &magic1, 4);
	int w = system_rtc_mem_read(191, &magic2, 4);

	if (magic1 != 0xDEADBEEF || magic2 != 0x00C0FFEE)
		return 0;

	// Read essid + pass + server config
	system_rtc_mem_read(68, conf_essid, 32);
	system_rtc_mem_read(76, conf_passw, 32);
	system_rtc_mem_read(84, conf_hostn, 32);
	system_rtc_mem_read(92, conf_path,  64);
	system_rtc_mem_read(108,&conf_port,  4);

	conf_essid[32] = 0;
	conf_passw[32] = 0;
	conf_hostn[32] = 0;
	conf_path [64] = 0;

	return 1; // Done!
}

void ICACHE_FLASH_ATTR store_settings() {
	// First write the config and then the magic
	os_printf("Store %s %s\n", conf_essid, conf_passw);

	system_rtc_mem_write(68, conf_essid, 32);
	system_rtc_mem_write(76, conf_passw, 32);
	system_rtc_mem_write(84, conf_hostn, 32);
	system_rtc_mem_write(92, conf_path,  32);
	system_rtc_mem_write(108,&conf_port,  4);

	uint32_t magic1 = 0xDEADBEEF, magic2 = 0x00C0FFEE;
	system_rtc_mem_write(64,  &magic1, 4);
	system_rtc_mem_write(191, &magic2, 4);
}


void ICACHE_FLASH_ATTR processing_timeout(void * arg) {
	// Shut down the thing! for now...
	int n = (int)arg;
	switch (n) {
	case 0:
		// Refresh timeout, normal wakeup
		power_gate_screen(0);
		put_back_to_sleep();
		break;
	case 1:
		// The user to reset the board after this manually
		screen_update(DispSleepMode);
		break;
	case 3:
		// This is a quick reboot
		system_deep_sleep_set_option(1); // Full wakeup!
		system_deep_sleep(1000);  // 1ms
		break;
	}

}


// Web server for config setting
static const char *index_200 = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
	"<html><head><title>E-ink Wifi Setup</title></head><body>"
	"<form action=\"push\" method=POST>"
	"<center><table>"
	"<tr><td>ESSID:</td><td><input type=\"text\" name=\"essid\"></td></tr>"
	"<tr><td>Password:</td><td><input type=\"text\" name=\"pass\"></td></tr>"
	"<tr><td>Server:</td><td><input type=\"text\" name=\"host\"></td></tr>"
	"<tr><td>Port:</td><td><input type=\"text\" name=\"port\"></td></tr>"
	"<tr><td>URL:</td><td><input type=\"text\" name=\"path\"></td></tr>"
	"</table><input type=\"submit\"></center>"
	"</form>"
	"</body></html>";

static const char *push_200 = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
	"<html><head><title>E-ink Wifi Setup</title></head><body>"
	"<center>"
	"Config saved! Rebooting ... <br/> You can safely disconnect from this wifi access point"
	"</center>"
	"</body></html>";

static volatile os_timer_t reboot_timer;
unsigned ICACHE_FLASH_ATTR get_index(char * buffer, const char * body) {
	// Just return a form!
	strcpy(buffer, index_200);
	return strlen(index_200);
}
unsigned ICACHE_FLASH_ATTR push_settings(char * buffer, const char * body) {
	os_printf("TOPARSE %s\n", body);
	// Parse body variables
	parse_form_s(body, conf_essid, "essid");
	parse_form_s(body, conf_passw, "pass");
	parse_form_s(body, conf_hostn, "host");
	parse_form_s(body, conf_path,  "path");
	conf_port = parse_form_d(body, "port");

	os_printf("Parsed %s %s %s\n", conf_essid, conf_passw, conf_hostn);

	// Update settings
	store_settings();

	// Schedule a reboot in 5 seconds
	os_timer_setfn(&reboot_timer, processing_timeout, 3);
	os_timer_arm(&reboot_timer, 5000, 0);
	
	strcpy(buffer, push_200);
	return strlen(push_200);
}

t_url_desc urls[] = {
	{ "/",     get_index },
	{ "/push", push_settings },
	{ NULL, NULL },
};


void ICACHE_FLASH_ATTR start_web_server() {
	// Set SoftAP+station mode
	if (wifi_get_opmode() != 3)
		wifi_set_opmode(3);

	// Setup AP mode
	struct softap_config config;
	strcpy(config.ssid, "EINKWIFI");
	config.password[0] = 0;
	config.channel = 1;
	config.ssid_hidden = 0;
	config.authmode = AUTH_OPEN;
	config.ssid_len = 0;
	config.beacon_interval = 100;
	config.max_connection = 4;

	wifi_softap_set_config(&config);// Set ESP8266 softap config .

	httpd_start(80, urls);
}


void ICACHE_FLASH_ATTR user_init( void ) {
	// Init GPIO stuff
	gpio_init();

	// Set UART0 to high speed!
	uart_div_modify( 0, UART_CLK_FREQ / ( 460800 ) );  // 921600

	os_printf("Booting...\n");

	// Use GPIO2 as UART0 output as well :)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U0TXD_BK);

	// First of all read the RTC memory and check whether data is valid.
	if (recover_settings()) {
		// We got some settings, now go and connect
		static struct station_config config;
		wifi_station_set_hostname("einkdisp");
		wifi_set_opmode_current(STATION_MODE);

		os_printf("Info %s, %s, %s, %d\n", conf_passw, conf_essid, conf_hostn, conf_port);
	
		config.bssid_set = 0;
		os_memcpy(&config.ssid,     conf_essid, 32);
		os_memcpy(&config.password, conf_passw, 33);
		wifi_station_set_config(&config);

		// Connect to the server, get some stuff and process it!
		wifi_set_event_handler_cb(wifi_callback);

		// To prevent battery going nuts, add a failback timer of 20 seconds
		// which should be more than enough for the whole process
		os_timer_setfn(&sleep_timer, processing_timeout, (void*)0);
		os_timer_arm(&sleep_timer, 20000, 0);
	}
	else {
		// Start web server and wait for connections!
		start_web_server();

		// Display the AP setup screen
		screen_update(DispApSetup);

		// To prevent battery drain, schedule sleep in 5 minutes
		// Present a "sleeping" screen after that :)
		os_timer_setfn(&sleep_timer, processing_timeout, (void*)1);
		os_timer_arm(&sleep_timer, 5*60*1000, 0);
	}
}


