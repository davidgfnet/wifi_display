
// Routines to drive E-ink display GDE043A2 (Good-Display.com)
// 
// This has been mostly reversed engineered from a firmware found
// in one of Waveshare's boards
// YOu can get one of them at http://www.waveshare.com/4.3inch-e-paper.htm

// READ THIS!
// Assuming the device is clocked at ~72MHz (14ns / instruction)
// for timing purposes. Also pin mapping is specified here
// If you are on a different board you might want to revisit some stuff here

#include <stm32f10x/stm32f10x.h>
#include "imgdec.h"

#define EINK_STV_L  GPIO_ResetBits(GPIOF, GPIO_Pin_6);
#define EINK_STV_H  GPIO_SetBits  (GPIOF, GPIO_Pin_6);

#define EINK_CPV_L  GPIO_ResetBits(GPIOF, GPIO_Pin_7);
#define EINK_CPV_H  GPIO_SetBits  (GPIOF, GPIO_Pin_7);

#define EINK_LE_L   GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#define EINK_LE_H   GPIO_SetBits  (GPIOB, GPIO_Pin_10);

#define EINK_OE_L   GPIO_ResetBits(GPIOF, GPIO_Pin_9);
#define EINK_OE_H   GPIO_SetBits  (GPIOF, GPIO_Pin_9);

#define EINK_CL_L   GPIO_ResetBits(GPIOF, GPIO_Pin_8);
#define EINK_CL_H   GPIO_SetBits  (GPIOF, GPIO_Pin_8);

#define EINK_SPH_L  GPIO_ResetBits(GPIOB, GPIO_Pin_13);
#define EINK_SPH_H  GPIO_SetBits  (GPIOB, GPIO_Pin_13);

#define WRITE_DATA_PORT(c) GPIO_Write(GPIOC, c);


#define NOP1        asm volatile ("nop");
#define NOP8        NOP1; NOP1; NOP1; NOP1; NOP1; NOP1; NOP1; NOP1;
//#define WAIT_40NS   NOP1; NOP1; NOP1;               //  3 NOPS ~ 42ns
// No need for nops since we do function call, we end up consuming ~5 insts per I/O write
#define WAIT_40NS   {};
#define WAIT_500NS  NOP8; NOP8; NOP8; NOP8; NOP8;   // 40 NOPS ~ 560ns

// Constant data (lookup tables for building drawing tables)

#define CLEAR_WHITE 0xAA

#define FRAME_CLEAR_LEN 16
// These two tables seem consistent with random snippets on the internet
// No fucking clue on what they actually do :D
const unsigned char wtable2[4][4] = {
	{ 0x01, 0x01, 0x01, 0x01 },
	{ 0x01, 0x01, 0x00, 0x00 },
	{ 0x01, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00 },
};

// Global variables

// Actual drawing tables that need to be recomputed
// every time we change "direction"
unsigned char wave_table_end  [256][4];

// Misc
static void GenericDelay(int n) {
	while (n--) {
		asm volatile("nop");
	}
}

typedef struct {
	GPIO_TypeDef * port;
	unsigned short value;
} pseq_t;

pseq_t power_seq[4] = {
	{ GPIOG, GPIO_Pin_6 },
	{ GPIOA, GPIO_Pin_11 },
	{ GPIOB, GPIO_Pin_4 },
	{ GPIOA, GPIO_Pin_7 },
};

// Power UP/DOWN
void einkd_PowerOn() {
	// Turns on the following pins in this order:
	// PG6, PA11, PB4, PA7

	for (int i = 0; i < 4; i++) {
		GPIO_SetBits(power_seq[i].port, power_seq[i].value);
		GenericDelay(255);
	}
}

void einkd_PowerOff() {
	for (int i = 3; i >= 0; i--) {
		GPIO_ResetBits(power_seq[i].port, power_seq[i].value);
		GenericDelay(255);
	}
}


// Start scan routine marks the "line start" signaling for the
// screen device.
void einkd_scan_start() {
	// CPV requirement is: 500ns (high/low and low/high)
	// This doesn't guarantee it, we should have around 36 nops for each semiperiod

	EINK_STV_L;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;

	EINK_STV_H;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;

	EINK_STV_L;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;
	EINK_CPV_L; WAIT_500NS;
	EINK_CPV_H; WAIT_500NS;
}

// Presumably reads a buffer (200 bytes) and renders it
// (for the current line only)
void einkd_sendrow(const unsigned char * line) {
	// Seems that Clock needs a period of 40ns (25Mhz)
	// Should set outputs to 50Mhz and guarantee ~4 instructions per GPIO write
	// Other signals (OE, LE, SPH, CPV) have a much lowe hold time (<10ns), so no worries!

	EINK_LE_H;             // LE = 1
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;  // 2 clocks
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;
	EINK_LE_L;             // LE = 0

	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;  // 2 clocks
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;
	EINK_OE_H;             // OE = 1

	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;  // 2 clocks
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;
	EINK_SPH_L;            // SPH = 0

	for (int i = 0; i < 200; i++) {
		WRITE_DATA_PORT(line[i]);

		EINK_CL_L; WAIT_40NS;
		EINK_CL_H; WAIT_40NS;  // 1 clock
	}

	EINK_SPH_H;            // SPH = 1
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;  // 2 clocks
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;

	EINK_CPV_L;            // CPV = 0
	EINK_OE_L;             // OE = 0

	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;  // 2 clocks
	EINK_CL_L; WAIT_40NS;
	EINK_CL_H; WAIT_40NS;

	EINK_CPV_H;            // CPV = 1
}

void einkd_refresh(const unsigned char * buffer) {
	unsigned char linebuffer[200];

	// Send this 8 times (using table 1)
	for (int i = 0; i < FRAME_CLEAR_LEN; i++) {
		einkd_scan_start();
		for (int row = 0; row < 600; row++) {  //r5
			for (int col = 0; col < 200; col++) {  //r6
				linebuffer[col] = CLEAR_WHITE;
			}
			einkd_sendrow(linebuffer);
		}
		einkd_sendrow(linebuffer);
	}

	// Repeat 4 more times using table 2 :)
	for (int i = 0; i < 4; i++) {
		einkd_scan_start();
		for (int row = 0; row < 600; row++) {  //r5
			for (int col = 0; col < 200; col++) {  //r6
				int offset = col + 200*row;
				unsigned char b = buffer[offset];
				linebuffer[col] = wave_table_end[b][i];
			}
			einkd_sendrow(linebuffer);
		}
		einkd_sendrow(linebuffer);
	}
}

void einkd_refresh_compressed(const unsigned char * buffer) {
	unsigned char linebuffer[200];
	img_decoder decoder;

	// Send this 8 times (using table 1)
	for (int i = 0; i < FRAME_CLEAR_LEN; i++) {
		einkd_scan_start();
		for (int row = 0; row < 600; row++) {  //r5
			for (int col = 0; col < 200; col++) {  //r6
				linebuffer[col] = CLEAR_WHITE;
			}
			einkd_sendrow(linebuffer);
		}
		einkd_sendrow(linebuffer);
	}

	// Repeat 4 more times using table 2 :)
	for (int i = 0; i < 4; i++) {
		einkd_scan_start();
		init_decoder(&decoder, buffer);
		for (int row = 0; row < 600; row++) {  //r5
			for (int col = 0; col < 200; col++) {  //r6
				unsigned char b = decode_sample(&decoder);
				linebuffer[col] = wave_table_end[b][i];
			}
			einkd_sendrow(linebuffer);
		}
		einkd_sendrow(linebuffer);
	}
}


// This function computes the "wave tables" that are used as
// lookup tables when it comes to draw a line
void make_wave_table(int direction) {
	for (int i = 0; i < 4; i++ ) {
		for (int j = 0; j < 256; j++ ) {
			unsigned char tmp;
			tmp  = wtable2[((j>>6)&0x3)][i] << 6;
			tmp |= wtable2[((j>>4)&0x3)][i] << 4;
			tmp |= wtable2[((j>>2)&0x3)][i] << 2;
			tmp |= wtable2[((j   )&0x3)][i];

			if (direction)
				tmp = ((tmp<<6)&0xC0) | ((tmp<<2)&0x30) | ((tmp>>2)&0x0C) | ((tmp>>6)&0x03);

			wave_table_end[j][i] = tmp;
		}
	}
}

void einkd_set_direction(int d) {
	// This is probably setting pins SHR and L/R to set 
	// Shift register direction (and perform screen 180 rotation?)

	if (d) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);
		GPIO_SetBits  (GPIOB, GPIO_Pin_15);
	}
	else {
		GPIO_SetBits  (GPIOB, GPIO_Pin_11);
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	}

	make_wave_table(d);
}

void einkd_init(int direction) {
	// GPIO initialization
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF |
		RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0xFF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// Some reset/set GPIO
	// This is probably setting MODE bits, whatever that means :|
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	
	// Set direction
	einkd_set_direction(direction);

	// Make sure we are not powering the thing :)
	einkd_PowerOff();

	// Set some control signals initial values
	EINK_LE_L;
	EINK_CL_L;
	EINK_OE_L;
	EINK_SPH_H;
	EINK_STV_H;
	EINK_CPV_L;
}

void einkd_deinit() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF |
		RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, DISABLE);
}


