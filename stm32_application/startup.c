
#include <stdint.h>
#include "except.h"

/* Specific handlers provided by application */
void ResetISR(void) __attribute__ ((interrupt));

/* Handlers provided by application */
extern void WWDG_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void PVD_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TAMPER_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void RTC_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void FLASH_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void RCC_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI0_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI4_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel4_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel5_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel6_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA1_Channel7_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void ADC1_2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USB_HP_CAN1_TX_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USB_LP_CAN1_RX0_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void CAN1_RX1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void CAN1_SCE_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI9_5_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM1_BRK_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM1_UP_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM1_TRG_COM_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM1_CC_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM4_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void I2C1_EV_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void I2C1_ER_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void I2C2_EV_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void I2C2_ER_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void SPI1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void SPI2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USART1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USART2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USART3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void EXTI15_10_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void RTCAlarm_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void USBWakeUp_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM8_BRK_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM8_UP_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM8_TRG_COM_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM8_CC_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void ADC3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void FSMC_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void SDIO_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM5_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void SPI3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void UART4_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void UART5_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM6_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void TIM7_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA2_Channel1_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA2_Channel2_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA2_Channel3_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));
extern void DMA2_Channel4_5_Handler(void) __attribute__ ((interrupt)) __attribute__ ((weak));

/* Main entry point */
extern void main(void);

#define STACK_SIZE			512
static unsigned long mainStack[STACK_SIZE] __attribute__ ((section (".stack")));

/* ****************************************************************************
 * Vector table for STM32F10x devices. Note that the proper constructs
 * must be placed on this to ensure that it ends up at physical address
 * 0x0800.0000
 * ***************************************************************************/
void (* const g_pfnVectors[])(void) __attribute__((section(".isr_vector"))) = {
	(void (*)(void))((unsigned long)mainStack+sizeof(mainStack)),
	ResetISR,						// The reset handler
	NmISR,							// The NMI handler
	FaultISR,						// The hard fault handler
	MPUFaultHander,					// The MPU fault handler
	BusFaultHandler,				// The bus fault handler
	UsageFaultHandler,				// The usage fault handler
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// SVCall handler
	IntDefaultHandler,				// Debug monitor handler
	0,								// Reserved
	0,								// The PendSV handler
	0,								// The SysTick handler
	WWDG_Handler,					// Window Watchdog
	PVD_Handler,					// PVD through EXTI Line detection
	TAMPER_Handler,					// Tamper
	RTC_Handler,					// RTC global
	FLASH_Handler,					// Flash global
	RCC_Handler,					// RCC global
	EXTI0_Handler,					// EXTI Line 0
	EXTI1_Handler,					// EXTI Line 1
	EXTI2_Handler,					// EXTI Line 2
	EXTI3_Handler,					// EXTI Line 3
	EXTI4_Handler,					// EXTI Line 4
	DMA1_Channel1_Handler,			// DMA 1 Channel 1
	DMA1_Channel2_Handler,			// DMA 1 Channel 2
	DMA1_Channel3_Handler,			// DMA 1 Channel 3
	DMA1_Channel4_Handler,			// DMA 1 Channel 4
	DMA1_Channel5_Handler,			// DMA 1 Channel 5
	DMA1_Channel6_Handler,			// DMA 1 Channel 6
	DMA1_Channel7_Handler,			// DMA 1 Channel 7
	ADC1_2_Handler,     	        /* ADC1 and ADC2 global Interrupt */
	USB_HP_CAN1_TX_Handler,         /* USB Device High Priority or CAN1 TX Interrupts  */
	USB_LP_CAN1_RX0_Handler,        /* USB Device Low Priority or CAN1 RX0 Interrupts  */
	CAN1_RX1_Handler,               /* CAN1 RX1 Interrupt                              */
	CAN1_SCE_Handler,               /* CAN1 SCE Interrupt                              */
	EXTI9_5_Handler,                /* External Line[9:5] Interrupts                   */
	TIM1_BRK_Handler,               /* TIM1 Break Interrupt                                 */
	TIM1_UP_Handler,                /* TIM1 Update Interrupt                                */
	TIM1_TRG_COM_Handler,           /* TIM1 Trigger and Commutation Interrupt               */
	TIM1_CC_Handler,                /* TIM1 Capture Compare Interrupt                       */
	TIM2_Handler,                   /* TIM2 global Interrupt                                */
	TIM3_Handler,                   /* TIM3 global Interrupt                                */
	TIM4_Handler,                   /* TIM4 global Interrupt                                */
	I2C1_EV_Handler,                /* I2C1 Event Interrupt                                 */
	I2C1_ER_Handler,                /* I2C1 Error Interrupt                                 */
	I2C2_EV_Handler,                /* I2C2 Event Interrupt                                 */
	I2C2_ER_Handler,                /* I2C2 Error Interrupt                                 */
	SPI1_Handler,                   /* SPI1 global Interrupt                                */
	SPI2_Handler,                   /* SPI2 global Interrupt                                */
	USART1_Handler,                 /* USART1 global Interrupt                              */
	USART2_Handler,                 /* USART2 global Interrupt                              */
	USART3_Handler,                 /* USART3 global Interrupt                              */
	EXTI15_10_Handler,              /* External Line[15:10] Interrupts                      */
	RTCAlarm_Handler,               /* RTC Alarm through EXTI Line Interrupt                */
	USBWakeUp_Handler,              /* USB Device WakeUp from suspend through EXTI Line Interrupt */
	TIM8_BRK_Handler,               /* TIM8 Break Interrupt                                 */
	TIM8_UP_Handler,                /* TIM8 Update Interrupt                                */
	TIM8_TRG_COM_Handler,           /* TIM8 Trigger and Commutation Interrupt               */
	TIM8_CC_Handler,                /* TIM8 Capture Compare Interrupt                       */
	ADC3_Handler,                   /* ADC3 global Interrupt                                */
	FSMC_Handler,                   /* FSMC global Interrupt                                */
	SDIO_Handler,                   /* SDIO global Interrupt                                */
	TIM5_Handler,                   /* TIM5 global Interrupt                                */
	SPI3_Handler,                   /* SPI3 global Interrupt                                */
	UART4_Handler,                  /* UART4 global Interrupt                               */
	UART5_Handler,                  /* UART5 global Interrupt                               */
	TIM6_Handler,                   /* TIM6 global Interrupt                                */
	TIM7_Handler,                   /* TIM7 global Interrupt                                */
	DMA2_Channel1_Handler,          /* DMA2 Channel 1 global Interrupt                      */
	DMA2_Channel2_Handler,          /* DMA2 Channel 2 global Interrupt                      */
	DMA2_Channel3_Handler,          /* DMA2 Channel 3 global Interrupt                      */
	DMA2_Channel4_5_Handler         /* DMA2 Channel 4 and Channel 5 global Interrupt        */
};

/* ****************************************************************************
 * The following are constructs created by the linker, indicating where the
 * the "data" and "bss" segments reside in memory.  The initializers for the
 * for the "data" segment resides immediately following the "text" segment.
 * ***************************************************************************/
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;

void ResetISR(void) {
	unsigned long *pulSrc, *pulDest;

	/* Copy the .data section from flash to SRAM. */
	pulSrc = &_etext;
	for (pulDest = &_data; pulDest < &_edata;) {
		*pulDest++ = *pulSrc++;
	}

	/* Zero fill the .bss sction.  This is done with inline assembly since this
	 * will clear the value of pulDest if it is not kept in a register. */
	__asm("    ldr     r0, =_bss\n"
		  "    ldr     r1, =_ebss\n"
		  "    mov     r2, #0\n"
		  "    .thumb_func\n"
		  "zero_loop:\n"
		  "    cmp     r0, r1\n"
		  "    it      lt\n"
		  "    strlt   r2, [r0], #4\n"
		  "    blt     zero_loop");

	/* Call recovery image main entry point: */
	main();
	/* Loop forever (If we get here there was an error in main) */
	while(1);
}

