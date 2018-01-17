#ifndef __USART_H__
#define __USART_H__

#include <stdio.h>
#include <stm32f10x.h>

#define MAX_UART_DMA_BUFFER_SIZE 255
/*
 * @brief USART���߱��
 */
enum BOARD_USART_TYPE {
	BOARD_USART_1 = 1, BOARD_USART_2 = 2
};

typedef int (*USART_RECV_CALLBACK)(enum BOARD_USART_TYPE usart_no,
		unsigned char recv);

struct USARTDefine {
	enum BOARD_USART_TYPE boardUart;
	USART_TypeDef* usartdef;	
	GPIO_TypeDef* pin_of_txd_group;
	uint16_t pin_of_txd;
	GPIO_TypeDef* pin_of_rxd_group;
	uint16_t pin_of_rxd;
	uint32_t bandrate;
	uint32_t APBConfig;
	uint8_t  UARTIRQConfig;
	USART_RECV_CALLBACK recvcallback;
};

/*
 @brief ʹ��UART��ʼ���ӿ�
 @param config ��������
 @param len ��ʼ��������
 @return ���ý��(0=SUCCESS, 1=FAILED)
 @desc ��Ҫ������NVIC (@ref BOARD_NVIC_Init) ��RCC
 */
uint32_t USART_Configuration(const struct USARTDefine* configs,uint16_t len);
void _usart_sendchar(USART_TypeDef* usart_typedef, uint8_t sChar);
void _usart_sendByte(USART_TypeDef* usart_typedef, int8_t data);
void _usart_sendMultiBytes(USART_TypeDef* usart_typedef, uint8_t* data, uint8_t data_len);
#endif
