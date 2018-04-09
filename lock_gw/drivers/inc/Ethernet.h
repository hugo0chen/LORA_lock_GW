#ifndef _ETH_H_
#define _ETH_H_

#include "stm32f10x.h"
#include "usart.h"

#define MAX_DATABUFFER_FROM_ETH 	255
#define FRAME_HEAD_1							0xA5
#define FRAME_HEAD_2							0xA5

/*
 @brief ETH_Module∂®“Â
 */
struct EthDefine {
	GPIO_TypeDef* pin_group_of_eth_rst;
	uint16_t pin_of_eth_rst;
	GPIO_TypeDef* pin_group_of_eth_cfg;
	uint16_t pin_of_eth_cfg;
};
typedef struct {
	uint8_t dataBufferFromETH[MAX_DATABUFFER_FROM_ETH];
	uint8_t dataBufferFromETH_len;
	uint8_t Eth_data_arrived ;
}ETH_DAT_STRUCT;

enum UART_SYN_STATUS {
	UNSYNED = 0,
	SYNED
};

extern ETH_DAT_STRUCT eth_data;

void ETH_Configuration(struct EthDefine);
void ETH_reset(void);
void ETH_manufacture_setting(void);
uint8_t eth_soft_manufacture_set(void);
void ETH_send(uint8_t*buf, uint8_t buf_len);
int receiveDMADataFromETH( char* dmaBuf, int length );

#endif
