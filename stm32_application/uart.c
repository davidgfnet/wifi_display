
#include <stm32f10x/stm32f10x.h>

void USART1_Init() {
	/* USART configuration structure for USART1 */
	USART_InitTypeDef usart1_init_struct;
	/* Bit configuration structure for GPIOA PIN9 and PIN10 */
	GPIO_InitTypeDef gpioa_init_struct;

	/* Enable clock for USART1, AFIO and GPIOA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* GPIOA PIN9 alternative function Tx */
	gpioa_init_struct.GPIO_Pin = GPIO_Pin_9;
	gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioa_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpioa_init_struct);
	/* GPIOA PIN9 alternative function Rx */
	gpioa_init_struct.GPIO_Pin = GPIO_Pin_10;
	gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioa_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpioa_init_struct);

	// No need for this apparently!
	// GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

	/* Enable USART1 */
	/* Baud rate 9600, 8-bit data, One stop bit
	 * No parity, Do both Rx and Tx, No HW flow control
	**/
	usart1_init_struct.USART_BaudRate = 460800;
	usart1_init_struct.USART_WordLength = USART_WordLength_8b;  
	usart1_init_struct.USART_StopBits = USART_StopBits_1;   
	usart1_init_struct.USART_Parity = USART_Parity_No ;
	usart1_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart1_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* Configure USART1 */
	USART_Init(USART1, &usart1_init_struct);

	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1, ENABLE);
}

void USART1_DeInit() {
	USART_Cmd(USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
}


