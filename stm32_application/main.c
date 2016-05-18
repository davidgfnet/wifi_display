
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include "gde043a2.h"
#include "sram.h"
#include "uart.h"
#include "imgdec.h"

// Add images
#include "ap_setup.h"
#include "lost_connection.h"
#include "low_battery.h"
#include "sleep.h"
#include "dhcp_error.h"
#include "dns_error.h"
#include "connection_error.h"
#include "invalid_image.h"

void initHW() {
	// Init basic stuff
	SystemInit();

	/* Debug support for low power modes: */
	DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
	DBGMCU_Config(DBGMCU_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_STANDBY, ENABLE);

	// Enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	// Disable the fucking JTAG!
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// Configure PB3 (LED)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void sync_blink() {
	// Some blink
	int i;
	GPIO_SetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++);
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++);
	GPIO_SetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++);
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++);
}

const void * image_table[8] =
{
	ap_setup,
	lost_connection,
	low_battery,
	sleep_mode,
	dhcp_error,
	dns_error,
	connection_error,
	invalid_image
};

unsigned char scratch[200*600] __attribute__((section(".extdata"), used));

int main() {
	// Init HW for the micro
	initHW();

	FSMC_SRAM_Init();

	USART1_Init();

	// Wait for the first byte, that tells us what to do:
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	unsigned char cmd = USART_ReceiveData(USART1);

	sync_blink(); // Received first byte!

	// Bit   7 defines direction hint (which can be ignored by the device)
	// Bit   6 tells whether to show a predefined picture (0) or to load a picture (1)
	//         If the bit is 1, it will be followed by 120000 bytes with the picture content
	// Bit   5 indicates whether the battery icon should be overlayed to the image
	// Bit 2,0 defines which preloaded picture to show (from the 4 in-ROM available)

	int direction = (cmd & 0x80) ? 1 : 0;
	int int_image = ((cmd & 0x40) == 0);
	int show_bat  = ((cmd & 0x20) == 0);
	int imageidx  =  cmd & 0x7;

	if (!int_image) {
		// Keep reading for external image!
		unsigned int spointer = 0;
		while (spointer < sizeof(scratch)) {
			// Read buffer to scratch!
			while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) {
				//__WFE();
			}
			scratch[spointer++] = USART_ReceiveData(USART1);
		}
	}
	else {
		// Decompress internal image
		image_decode(image_table[imageidx], scratch);
	}

	// Initialize tables (according to direction)
	einkd_init(direction);

	// Power ON, draw and OFF again!
	einkd_PowerOn();
	einkd_refresh(scratch);
	einkd_PowerOff();

	// Send ACK back?
	// USART_SendData(USART1, (uint8_t) ch);

	// Notify update!
	sync_blink();

	USART1_DeInit();
	einkd_deinit();

	// Turn ourselves OFF, hopefully save some power before final power gate off
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	while (1);

	return 0;
}


