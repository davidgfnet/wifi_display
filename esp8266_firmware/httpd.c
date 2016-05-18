
// HTTPD u-server for ESP8266, by David Guillen Fandos (david@davidgf.net)
// Ideas borrowed from all over the internet :) Use and share, keep this notice!

#include "mem.h"
#include "osapi.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "ip_addr.h"
#include "espconn.h"

#include "httpd.h"

#define MAX_HEADER_LENGTH 1024  // Usally headers are around 200 bytes (without cookies)
#define MAX_OPEN_CONN       16  // Takes a lot of RAM, careful there!
#define CTIMEOUT             5  // Five seconds without activity means connection drop!

#define tolower(c) ((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c)

// Listening connections
struct espconn listen_socket;
static esp_tcp listen_tcp;

// Aux stuff
const t_url_desc * url_descriptors = 0;
unsigned tick_counter = 0;

typedef enum { coEmpty, coWaitHeader, coWaitBody, coResponse, coDestroy } CoState;

static void ICACHE_FLASH_ATTR tcp_disconnect(struct espconn *conn);

const char *ok_404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nConnection: close\r\n\r\nPage not found!";
const char *ok_500 = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nConnection: close\r\n\r\nInternal Server Error!";

// Connection struct pool
typedef struct {
	// Req
	char request[MAX_HEADER_LENGTH];
	int request_len;
	int header_len;
	int body_len;
	int sent_data;

	// Connection id
	uint8 remote_ip[4];
	int remote_port;
	struct espconn * espc;

	// FSM
	CoState state;
	unsigned keepalive;
} t_conn_state;

t_conn_state carray[MAX_OPEN_CONN];

int ICACHE_FLASH_ATTR clookup(const struct espconn * conn) {
	// Lookup for this connection in the carray
	for (int i = 0; i < MAX_OPEN_CONN; i++) {
		if (carray[i].state != coEmpty &&
		    memcmp(carray[i].remote_ip, conn->proto.tcp->remote_ip, 4) == 0 &&
		    carray[i].remote_port == conn->proto.tcp->remote_port)
		return i;
	}
	return -1;
}

static int ICACHE_FLASH_ATTR cheap_atoi(const char * s) {
	int r = 0;
	while (*s >= '0' && *s <= '9')
		r = r * 10 + (*s++ - '0');
	return r;
}

static int ICACHE_FLASH_ATTR get_field_d(const char * msg, int len, const char * hname) {
	// Find something like "\r\n hname : [0-9]+ \r\n"
	char buf[len + 1];
	memset(buf, 0, len+1);

	for (int i = 0; i < len; i++)
		buf[i] = tolower(msg[i]);

	const char * start = buf;

	while(1) {
		start = strstr(start, hname);
		if (start) {
			start += strlen(hname);
			while (*start == ' ') start++;
			if (*start == ':') {
				while (*start == ' ') start++;
				return cheap_atoi(start);
			}
		}
		else
			return 0;
	}
}

static void ICACHE_FLASH_ATTR tcp_sent(struct espconn *conn) {
	int p = clookup(conn);
	if (p < 0) return; // WTF!

	os_printf("TCP sent!\n");

	// Keep sending responses
	int tosend = carray[p].request_len - carray[p].sent_data;
	if (tosend > 0) {
		// Send in 128 byte chunks (or less)
		if (tosend > 128) tosend = 128;
		espconn_send(conn, &carray[p].request[carray[p].sent_data], tosend);
		carray[p].sent_data += tosend;
	}
	if (carray[p].request_len == carray[p].sent_data) {
		// Close connection since there is no more data to show
		tcp_disconnect(conn);
	}

	carray[p].keepalive = tick_counter;
}

static void ICACHE_FLASH_ATTR tcp_recv(struct espconn *conn, char *data, unsigned short len) {
	int p = clookup(conn);
	if (p < 0)
		return; // WTF!

	// Check FSM
	if (carray[p].state != coWaitHeader && carray[p].state != coWaitBody) {
		tcp_disconnect(conn);
		return;
	}

	os_printf("TCP recv!\n");

	// Put data in the buffer, beware overflows!
	if (carray[p].request_len + len >= MAX_HEADER_LENGTH) {
		tcp_disconnect(conn);
		os_printf("Overflow!\n");
	}
	else {
		// Buffer data in
		memcpy(&carray[p].request[carray[p].request_len], data, len);
		carray[p].request_len += len;
		carray[p].request[carray[p].request_len] = 0; // Keep patching NULLs :)

		os_printf("Data received\n");

		if (len > 0)
			carray[p].keepalive = tick_counter;

		// Check whether we have a header already
		if (carray[p].state == coWaitHeader) {
			os_printf("Wait header\n");

			const char * body_start = strstr(carray[p].request, "\r\n\r\n");
			if (body_start) {
				body_start += 4; // Advance newline chars

				// We do have a proper header!
				// Do we have to wait for the body or what?
				carray[p].header_len = ((uintptr_t)body_start) - ((uintptr_t)carray[p].request);
				unsigned body_len = get_field_d(carray[p].request, carray[p].header_len, "content-length");

				if (body_len > 0) {
					// Wait for the body to be received (we may have it already actually!)
					carray[p].state = coWaitBody;
					carray[p].body_len = body_len;
				}
				else
					// No body, but header, so we are done receiving!
					carray[p].state = coResponse;
			}
		}
		if (carray[p].state == coWaitBody) {
			os_printf("Wait body\n");

			// Check buffer in :)
			if (carray[p].request_len >= carray[p].header_len + carray[p].body_len)
				carray[p].state = coResponse;
		}

		// This means we should start answering the client, go on!
		if (carray[p].state == coResponse) {
			carray[p].sent_data = 0;

			os_printf("Response create\n");

			int notfound = 1;
			// First parse the page the user accessed
			int rsize = memcmp(carray[p].request, "GET",  3) == 0 ? 3 :
			            memcmp(carray[p].request, "POST", 4) == 0 ? 4 : 0;
			while (rsize && carray[p].request[rsize] == ' ')
				rsize++;

			if (rsize) {
				const char * lineend = os_strstr(&carray[p].request[rsize], " ");
				unsigned psize = ((uintptr_t)lineend) - ((uintptr_t)carray[p].request) - rsize;
				
				const t_url_desc * desc = url_descriptors;
				while (desc && desc->path) {
					if (strlen(desc->path) == psize &&
					    memcmp(desc->path, &carray[p].request[rsize], psize) == 0) {

						// Matched!
						int rlen = desc->callback(carray[p].request, &carray[p].request[carray[p].header_len]);
						carray[p].request_len = rlen;

						// Fake data pump!
						tcp_sent(conn);

						// Break here!
						return;
					}
					desc++;
				}
			}
			else
				notfound = 0;

			// There must be an error, not found or some malformed request
			strcpy(carray[p].request, notfound ? ok_404 : ok_500);
			carray[p].request_len = strlen(carray[p].request);
			tcp_sent(conn);
		}
	}
}

static void ICACHE_FLASH_ATTR tcp_disconnect(struct espconn *conn) {
	// Find and deallocate the connection if necesary
	int p = clookup(conn);
	if (p >= 0)
		carray[p].state = coDestroy;

	os_printf("tcp_disconnect!\n");
}
static void ICACHE_FLASH_ATTR tcp_error(struct espconn *conn, sint8 err) {
	tcp_disconnect(conn);
}

void ICACHE_FLASH_ATTR client_tcp_connect_cb(struct espconn *conn) {
	// Store the connection in the array
	for (int i = 0; i < MAX_OPEN_CONN; i++) {
		if (carray[i].state == coEmpty) {
			// Clear state!
			carray[i].state = coWaitHeader;
			carray[i].request_len = 0;
			carray[i].keepalive = tick_counter;
			carray[i].espc = conn;

			memcpy(carray[i].remote_ip, conn->proto.tcp->remote_ip, 4);
			carray[i].remote_port = conn->proto.tcp->remote_port;

			// Register callbacks
			espconn_regist_recvcb  (conn, (espconn_recv_callback)tcp_recv);
			espconn_regist_reconcb (conn, (espconn_reconnect_callback)tcp_error);
			espconn_regist_disconcb(conn, (espconn_connect_callback)tcp_disconnect);
			espconn_regist_sentcb  (conn, (espconn_sent_callback)tcp_sent);
			return;
		}
	}

	// Got here, this means no empty connections, just close the connection!
	// FIXME!
}

void ICACHE_FLASH_ATTR connection_housekeeping() {
	// Check for Destroyed connections or connections that timed out!
	for (int i = 0; i < MAX_OPEN_CONN; i++) {
		// Timeout!
		if (carray[i].state != coEmpty && tick_counter > carray[i].keepalive + CTIMEOUT)
			carray[i].state = coDestroy;

		// Destroyed connection, destroy tcp esp conn and free resources
		if (carray[i].state == coDestroy) {
			os_printf("Destroying connection!\n");
			// Mark as free
			carray[i].state = coEmpty;
			// Release ESP resources!
			espconn_disconnect(carray[i].espc);
		}
	}
}

static volatile os_timer_t housekeeping_timer;

void ICACHE_FLASH_ATTR httpd_start(int port, const t_url_desc * descriptors) {
	// Inint internal stuff
	for (int i = 0; i < MAX_OPEN_CONN; i++)
		carray[i].state = coEmpty;

	url_descriptors = descriptors;

	// Create server
	listen_socket.type = ESPCONN_TCP;
	listen_socket.state = ESPCONN_NONE;
	listen_tcp.local_port = port;
	listen_socket.proto.tcp = &listen_tcp;

	espconn_regist_connectcb(&listen_socket, (espconn_connect_callback)client_tcp_connect_cb);
	espconn_accept(&listen_socket);

	// Set up a timer to clenup dead connections every 1s if needed
	os_timer_setfn(&housekeeping_timer, connection_housekeeping, NULL);
	os_timer_arm(&housekeeping_timer, 1000, 1);

	os_printf("Started web server!\n");
}

void ICACHE_FLASH_ATTR httpd_stop() {
	os_timer_disarm(&housekeeping_timer);
}

#define unhexify(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : (tolower(c) - 'a' + 10))

void ICACHE_FLASH_ATTR url_unscape(char * s) {
	for (int i = 0; i < strlen(s); i++) {
		if (s[i] == '%' && i+2 < strlen(s)) {
			// Unscape char
			int code = (unhexify(s[i+1]) << 4) | (unhexify(s[i+2]));
			s[i] = (char)code;
			// Copy over!
			memmove(&s[i+1], &s[i+3], strlen(s) - i - 3 + 1);
		}
	}
}

int ICACHE_FLASH_ATTR parse_form_s(const char * body, char * wb, const char * field) {
	int flen = strlen(field);
	while (body && *body != 0) {
		const char * eq = os_strstr(body, "=");
		if (!eq)
			return 0;
		const char * nextf = os_strstr(body, "&");

		int nlen = ((uintptr_t)eq) - ((uintptr_t)body);
		if (nlen == flen && memcmp(body, field, flen) == 0) {
			// Field match
			eq += 1;
			int vlen = strlen(eq);
			if (nextf)
				vlen = ((uintptr_t)nextf) - ((uintptr_t)eq);

			// Write to buffer and add null char
			memcpy(wb, eq, vlen);
			wb[vlen] = 0;

			// Decode URL params if needed!
			url_unscape(wb);

			return 1;
		}

		body = nextf; // Move to next field
		body++;
	}
}

int ICACHE_FLASH_ATTR parse_form_d(const char * body, const char * field) {
	char tmp[20];
	int r = parse_form_s(body, tmp, field);
	if (!r)
		return -1;

	return cheap_atoi(tmp);
}


