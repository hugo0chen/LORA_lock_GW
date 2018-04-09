#include "ethernet.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "Globalvar.h"

struct EthDefine eth_def;
ETH_DAT_STRUCT eth_data;
uint32_t abnormalCountFromEth = 0;

uint8_t mode_sync = 0;
uint8_t at_cmd;

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
	Delay_nms(3500);// > 3s
	GPIO_SetBits(eth_def.pin_group_of_eth_cfg, eth_def.pin_of_eth_cfg);
}

void ETH_send(uint8_t*buf, uint8_t buf_len){
	_usart_sendMultiBytes(USART1, buf, buf_len);	
}

uint8_t eth_enter_AT_mode(void){
	uint8_t data[] = "+++";
	uint32_t mode_timetick = local_ticktime();
	
	Delay_nms(20);
	mode_sync = 0;
	ETH_send(data, 3);
	at_cmd = 1;
	while(1){
		if(timeout(mode_timetick, 1000)){
			at_cmd = 0;
			break;
		}
		if(mode_sync == 1){
			data[0] = 'a';
			ETH_send(data, 1);
			at_cmd = 1;
			mode_sync = 2;
		}else if(mode_sync == 3){						
			return 1;
		}		
	}	
	return 0;	
}

uint8_t eth_soft_manufacture_set(void){
	uint8_t cmd_buf[] = "AT+RELD\r";
	uint32_t timeTick ;
	
	if(eth_enter_AT_mode() == 1){
		ETH_send(cmd_buf, sizeof(cmd_buf)-1);
		timeTick = local_ticktime();
		at_cmd = 1;
		mode_sync = 0;
		while(!timeout(timeTick, 1000)){
			if(mode_sync == 1){				
				return 1;
			}
		}		
	}
	ETH_reset();
	return 0;
}
int receiveByteFromEth( enum BOARD_USART_TYPE usart_no, unsigned char recv){
	
	return 0;
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

uint8_t at_resp_check(char* data, uint8_t len){
	
	if(len < 4)
		return 0;
	
	if(( data[0] == 0x0D ) && ( data[1] == 0x0A) && \
		( data[len - 2] == 0x0D ) && ( data[len - 1] == 0x0A)){
		return 1;
	}
	
	return 0;
}

int receiveDMADataFromETH( char* dmaBuf, int length ) {
	int i = 0;
	
	if(at_cmd == 1){
		if(length == 1){
		if(dmaBuf[0] == 'a'){			
			mode_sync = 1;
		}
		}else if(length == 3){
			if(strcmp(dmaBuf, "+ok") == 0){
				mode_sync = 3;
			}
		}else{
			if(at_resp_check(dmaBuf, length) == 1){
				if(strstr(dmaBuf, "+OK=rebooting")!= NULL){
					mode_sync = 1;
				}
			}
		}
		at_cmd = 0;
	}	
	else{
		for( i = 0; i < length; i++ ) {
			receiveByteFromETH( dmaBuf[i] );
		}
	}
	return 0;
}
