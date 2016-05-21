
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

	/* Enable USART1
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

	USART_Cmd(USART1, ENABLE);
}

void USART3_Init() {
	/* USART configuration structure for USART2 */
	USART_InitTypeDef usart3_init_struct;
	/* Bit configuration structure for GPIOA PIN9 and PIN10 */
	GPIO_InitTypeDef gpioc_init_struct;
	USART_ClockInitTypeDef USART_ClockInitStructure; 

	/* Enable clock for USART1, AFIO and GPIOA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOC, ENABLE);

	/* GPIOC PIN10 alternative function Tx */
	gpioc_init_struct.GPIO_Pin = GPIO_Pin_10;
	gpioc_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioc_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &gpioc_init_struct);
	/* GPIOC PIN11 alternative function Rx */
	gpioc_init_struct.GPIO_Pin = GPIO_Pin_11;
	gpioc_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioc_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &gpioc_init_struct);
	/* GPIOC PIN12 alternative function CK */
	gpioc_init_struct.GPIO_Pin = GPIO_Pin_12;
	gpioc_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioc_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &gpioc_init_struct);

	/* Enable USART2
	 * No parity, Do both Rx and Tx, No HW flow control
	**/
	USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_High;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
	USART_ClockInit(USART3, &USART_ClockInitStructure);

	usart3_init_struct.USART_BaudRate = 460800;
	usart3_init_struct.USART_WordLength = USART_WordLength_8b;  
	usart3_init_struct.USART_StopBits = USART_StopBits_1;
	usart3_init_struct.USART_Parity = USART_Parity_No;
	usart3_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart3_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* Configure USART1 */
	USART_Init(USART3, &usart3_init_struct);

	USART_Cmd(USART3, ENABLE);
}

void USART1_DeInit() {
	USART_Cmd(USART1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
}


