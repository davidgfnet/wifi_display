
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
	GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);

	//GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);
}

void sync_blink() {
	// Some blink
	int i;
	GPIO_SetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++) { asm volatile(""); }
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++) { asm volatile(""); }
	GPIO_SetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++) { asm volatile(""); }
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	for (i = 0; i < 3000000; i++) { asm volatile(""); }
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

unsigned char scratch[60*1024]; // __attribute__((section(".extdata"), used));

void SPI_Initialize() {
	// Configure PB3 & PB5 for SPI slave
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

	SPI_InitTypeDef SPI_InitStructure;
	SPI_StructInit(&SPI_InitStructure);
	SPI_I2S_DeInit(SPI1);

	/* SPI1 Config */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
  
	/* Configure SPI1 && enable */
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
}

int main() {
	// Init HW for the micro
	initHW();

	// Fuckin SRAM memory has stopped working
	// That means only 60KB RAM, either B/W images (1bit) or compressed images!
	//FSMC_SRAM_Init();

	SPI_Initialize();

	// Wait for the first byte, that tells us what to do:
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	unsigned char cmd = SPI_I2S_ReceiveData(SPI1);

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
			while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) { /*__WFE(); */ }
			scratch[spointer++] = SPI_I2S_ReceiveData(SPI1);
		}
	}
	else {
		// Copy the internal compressed image
		memcpy(scratch, image_table[imageidx], sizeof(scratch));
	}

	// Initialize tables (according to direction)
	einkd_init(direction);

	// Power ON, draw and OFF again!
	einkd_PowerOn();
	einkd_refresh_compressed(scratch);
	einkd_PowerOff();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
	einkd_deinit();

	// Turn ourselves OFF, hopefully save some power before final power gate off
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	while (1);

	return 0;
}


