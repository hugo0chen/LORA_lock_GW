#include "ethernet.h"
#include "delay.h"

struct EthDefine eth_def;
ETH_DAT_STRUCT eth_data;
uint32_t abnormalCountFromEth = 0;

void ETH_Configuration(struct EthDefine def){
		GPIO_InitTypeDef	GPIO_InitStructure;
	
		eth_def = def;
		GPIO_InitStructure.GPIO_Pin   = def.pin_of_eth_cfg;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(def.pin_group_of_eth_cfg, &GPIO_InitStructure);
		GPIO_SetBits(def.pin_group_of_eth_cfg, def.pin_of_eth_cfg);
	
		GPIO_InitStructure.GPIO_Pin   = def.pin_of_eth_rst;		
		GPIO_Init(def.pin_group_of_eth_rst, &GPIO_InitStructure);
		GPIO_SetBits(def.pin_group_of_eth_rst, def.pin_of_eth_rst);
}

void ETH_reset(void){
	GPIO_ResetBits(eth_def.pin_group_of_eth_rst, eth_def.pin_of_eth_rst);
	Delay_nms(320);
	GPIO_SetBits(eth_def.pin_group_of_eth_rst, eth_def.pin_of_eth_rst);
}

void ETH_manufacture_setting(void){
	GPIO_ResetBits(eth_def.pin_group_of_eth_cfg, eth_def.pin_of_eth_cfg);
	Delay_nms(320);
	GPIO_SetBits(eth_def.pin_group_of_eth_cfg, eth_def.pin_of_eth_cfg);
}

int receiveByteFromEth( enum BOARD_USART_TYPE usart_no, unsigned char recv){
	
	return 0;
}
void ETH_send(uint8_t*buf, uint8_t buf_len){
	_usart_sendMultiBytes(USART1, buf, buf_len);	
}

int receiveByteFromETH(unsigned char recv) {
	static int idx = 0;
	static enum UART_SYN_STATUS status = UNSYNED;
	static uint8_t frameLength;
	uint8_t i;
	uint16_t temp_sumCheck;
	uint16_t temp;
	
	if( eth_data.Eth_data_arrived == 1 ) {
		return 0;
	}
	
	if( idx < ( MAX_DATABUFFER_FROM_ETH - 1 ) ) {
		eth_data.dataBufferFromETH[idx++] = recv;
		if( idx >= 2 ) {
			if( status == UNSYNED ) {
				// Check UART Frame Head
				if( eth_data.dataBufferFromETH[idx - 1] == FRAME_HEAD_2 && eth_data.dataBufferFromETH[idx - 2] == FRAME_HEAD_1 ) {				
					status = SYNED;
				} else {
					idx = 0;
				}					
			} else if( status == SYNED ) {
				if( idx == 3 ) {
					frameLength = eth_data.dataBufferFromETH[2];
				} else if( idx == ( frameLength + 2 ) ) {		
					// Check Sum Check
					temp_sumCheck = 0;
					for( i = 0 ;i < (frameLength - 2); i++){
						temp_sumCheck	+= eth_data.dataBufferFromETH[i+2];						
					} 
					temp =  eth_data.dataBufferFromETH[idx - 2];
					temp = ( temp << 8) + eth_data.dataBufferFromETH[idx - 1];
					if ( temp_sumCheck == temp ) {
						eth_data.dataBufferFromETH_len = idx;
						eth_data.Eth_data_arrived = 1;
					}
					else {
						abnormalCountFromEth++;
					}
					idx = 0;
					status = UNSYNED;							
				}					
			}
		}
	} else {
		idx = 0;
	}
	return 0;
}

int receiveDMADataFromETH( char* dmaBuf, int length ) {
	int i = 0;
	
	for( i = 0; i < length; i++ ) {
		receiveByteFromETH( dmaBuf[i] );
	}
	return 0;
}
