#include "GlobalVar.h"
#include "usart.h"
#include "ethernet.h"

unsigned char receiveBuf[8];
unsigned char receiveCount;

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/************************** new *****************************/
uint8_t UART1_DMA_buffer[MAX_UART_DMA_BUFFER_SIZE];

USART_RECV_CALLBACK _usart_recv_callbacks[2] = { 0 };

#define UartIsConfigurated(rs485_no) (_usart_485_defines[rs485_no].usartdef!=NULL)
#define UartTypeDef(rs485_no) (_usart_485_defines[rs485_no].usartdef)

void _usart_nvic_config(uint32_t usart_irqn, uint32_t preemptionpriority,
		uint32_t subpriority) {
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = usart_irqn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemptionpriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subpriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void _usart_comm_irqhandler(USART_TypeDef* usarttypedef, enum BOARD_USART_TYPE usart_type) {
	unsigned char recv_char;
	if (USART_GetFlagStatus(usarttypedef, USART_FLAG_RXNE) != RESET) {
		USART_ClearITPendingBit(usarttypedef, USART_IT_RXNE);
		recv_char = USART_ReceiveData(usarttypedef);
		if (_usart_recv_callbacks[usart_type] != NULL)
			_usart_recv_callbacks[usart_type](usart_type, recv_char);
	}
}

// USART 1
// TX --- DMA1_CH4
// RX --- DMA1_CH5
void USART1_IRQHandler(void) {
	uint16_t dma_len;
	uint32_t i;

	if( USART_GetITStatus(USART1,USART_IT_IDLE) != RESET ){
		DMA_Cmd(DMA1_Channel5,DISABLE);
		dma_len = DMA_GetCurrDataCounter(DMA1_Channel5);
		DMA_ClearFlag(DMA1_FLAG_GL5 | DMA1_FLAG_TC5 | DMA1_FLAG_TE5 | DMA1_FLAG_HT5);
		DMA_SetCurrDataCounter(DMA1_Channel5,MAX_UART_DMA_BUFFER_SIZE);
		DMA_Cmd(DMA1_Channel5,ENABLE);
	}
	i = USART1->SR;
	i = USART1->DR;

	receiveDMADataFromETH((char*)UART1_DMA_buffer, MAX_UART_DMA_BUFFER_SIZE - dma_len );
}

void DMA1_Channel5_IRQHandler(void)	{
	uint16_t dma_len;
	
	DMA_ClearITPendingBit(DMA1_IT_TC5);
	DMA_ClearITPendingBit(DMA1_IT_TE5);
	DMA_Cmd(DMA1_Channel5,DISABLE);
	dma_len = DMA_GetCurrDataCounter(DMA1_Channel5);
	DMA_SetCurrDataCounter(DMA1_Channel5,MAX_UART_DMA_BUFFER_SIZE);
	DMA_Cmd(DMA1_Channel5,ENABLE);
	receiveDMADataFromETH( (char*)UART1_DMA_buffer, dma_len );
}

void USART2_IRQHandler(void) {
	_usart_comm_irqhandler(USART2, BOARD_USART_2);
}

void _usart_sendchar(USART_TypeDef* usart_typedef, uint8_t sChar) {
	// Write a character to the USART
	USART_SendData(usart_typedef, (uint8_t) sChar);
	while (USART_GetFlagStatus(usart_typedef, USART_FLAG_TXE) == RESET) {
	}
}

void _usart_sendByte(USART_TypeDef* usart_typedef, int8_t data) {
	USART_SendData(usart_typedef, data);
	while (USART_GetFlagStatus(usart_typedef, USART_FLAG_TXE) == RESET) {
	}
}

void _usart_sendMultiBytes(USART_TypeDef* usart_typedef, uint8_t* data, uint8_t data_len){
	uint8_t i;
	for( i = 0; i < data_len; i++){
		_usart_sendchar(usart_typedef, data[i]);
	}
}

// For printf function
PUTCHAR_PROTOTYPE {
	USART_SendData(USART2, (u8) ch);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET){}
	return ch;
}

uint32_t USART_Configuration(const struct USARTDefine* configs, uint16_t len) {
	uint16_t u = 0;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
	DMA_InitTypeDef DMA_InitStructure; 
	
	for( u = 0; u < len; ++u) {
		struct USARTDefine config = configs[u];
		if( config.usartdef == USART1 ) {
			RCC_APB2PeriphClockCmd(config.APBConfig, ENABLE);
		} else {
			RCC_APB1PeriphClockCmd(config.APBConfig, ENABLE);
		}	
		_usart_recv_callbacks[config.boardUart] = config.recvcallback;
		
		GPIO_InitStructure.GPIO_Pin 		= config.pin_of_txd;
		GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
		GPIO_Init(config.pin_of_txd_group, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin	 		= config.pin_of_rxd;
		GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
		GPIO_Init(config.pin_of_rxd_group, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate 		= config.bandrate;
		USART_InitStructure.USART_WordLength  = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits 		= USART_StopBits_1;
		USART_InitStructure.USART_Parity 			= USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(config.usartdef, &USART_InitStructure);
		if( config.recvcallback != 0 ) { 
			USART_ITConfig(config.usartdef, USART_IT_IDLE, ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel 									 = config.UARTIRQConfig;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority 			 = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd 							 = ENABLE;
			NVIC_Init(&NVIC_InitStructure);	
			USART_ClearFlag(config.usartdef,USART_FLAG_IDLE);			
		}
		// DMA configuration 
		if( config.usartdef == USART1 ) {	
				RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
				DMA_DeInit(DMA1_Channel5);
				DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; 
				DMA_InitStructure.DMA_MemoryBaseAddr 		 = (uint32_t)UART1_DMA_buffer;  
				DMA_InitStructure.DMA_DIR 							 = DMA_DIR_PeripheralSRC; 
				DMA_InitStructure.DMA_BufferSize			   = MAX_UART_DMA_BUFFER_SIZE; 
				DMA_InitStructure.DMA_PeripheralInc 		 = DMA_PeripheralInc_Disable; 
				DMA_InitStructure.DMA_MemoryInc 				 = DMA_MemoryInc_Enable; 
				DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
				DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte; 
				DMA_InitStructure.DMA_Mode 							 = DMA_Mode_Circular; 
				DMA_InitStructure.DMA_Priority 					 = DMA_Priority_Low; 
				DMA_InitStructure.DMA_M2M 							 = DMA_M2M_Disable; 
				DMA_Init(DMA1_Channel5, &DMA_InitStructure); 

				/* Enable the End of Transfer Interrupt after */
				DMA_ITConfig( DMA1_Channel5, DMA_IT_TC, ENABLE );
				DMA_ITConfig( DMA1_Channel6, DMA_IT_TE, ENABLE );

				USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
				/* Enable DMA Channel3 */
				DMA_Cmd(DMA1_Channel5, ENABLE);
				
				// Rx
				NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
				NVIC_InitStructure.NVIC_IRQChannel 								   = DMA1_Channel5_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority 			 = 0;
				NVIC_InitStructure.NVIC_IRQChannelCmd 						   = ENABLE;
				NVIC_Init(&NVIC_InitStructure);
				
				// Tx
				NVIC_InitStructure.NVIC_IRQChannel 									 = DMA1_Channel4_IRQn;
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
				NVIC_InitStructure.NVIC_IRQChannelSubPriority 			 = 1;
				NVIC_InitStructure.NVIC_IRQChannelCmd 							 = ENABLE;
				NVIC_Init(&NVIC_InitStructure);
		}
		USART_Cmd(config.usartdef, ENABLE);		
	}
	return 0;
}
