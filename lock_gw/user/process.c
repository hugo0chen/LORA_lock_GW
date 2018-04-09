#include "process.h"
#include "usart.h"
#include "ethernet.h"
#include "sx1276.h"
#include "delay.h"
#include "string.h"
#include "schd.h"
#include "lora_com.h"
#include "main.h"
#include "flash.h"

void ack_server(uint8_t cmd, uint8_t ack){
	uint8_t data[] = {0x5A, 0x5A, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0x51, 0x01, 0x00, 0x04, 0x58};
	
	switch(cmd){
		case  GW_MANUFACTURE_CMD:
			data[7] = GW_MANUFACTURE_CMD;
			data[8] = ack;
			if(ack == 0){
				data[11] = 0x57;
			}else{
				data[11] = 0x58;
			}
			break;
		case GW_RESET_CMD:
			data[7] = GW_RESET_CMD;
			data[8] = ack;
			if(ack == 0){
				data[11] = 0x59;
			}else{			
				data[11] = 0x5A;
			}
			break;
		default:
			break;			
	}
	ETH_send(data, sizeof(data));
}

uint8_t process_data_from_server( uint8_t* buf, uint16_t buf_len ){	
	uint8_t i;
	uint16_t temp_flash_flag[2] = {1, 1};
	if(buf[CMD_POSITION] == GW_MANUFACTURE_CMD){	
		if(putHalfWordData(USER_FLASH_START_ADDR, temp_flash_flag, 2)){	
			ack_server(GW_MANUFACTURE_CMD, ACK_SUCCESS);
		}else{
			ack_server(GW_MANUFACTURE_CMD, ACK_FAILED);
		}
		mcu_reset();
	}else if(buf[CMD_POSITION] == GW_RESET_CMD){
		ack_server(GW_RESET_CMD, ACK_SUCCESS);		
		ETH_reset();				
		mcu_reset();		
		ack_server(GW_RESET_CMD, ACK_FAILED);
	}else{
		i = write_datas_to_txBuf( &lora_txBuf, buf, buf_len );
		if(buf[CMD_POSITION] == UPDATE_CONTENT_CMD){
			lora_txBuf.node[i].ack_enabled = 1;
		}
	}
	
	return 0;
}

uint8_t data_check(uint8_t*buf, uint8_t* buf_len){
	
	uint8_t i, j;	
	uint16_t checkSum = 0;
	uint8_t temp_checkSum = 0;
	uint8_t temp_len = *buf_len;
	uint8_t valid_frame_len;
	
	if( temp_len < 9)
		return 0xFF;
	
	for( i = 0; i < temp_len - 8; i++){
			if((buf[i] == TO_SERVER_HEADER_1) &&( buf[i+1] == TO_SERVER_HEADER_2)){					
				valid_frame_len =  buf[i + 2] + 2;
				for( j = 0; j <  valid_frame_len - 4; j++){
					checkSum += buf[i+2+j];
				}
				temp_checkSum += buf[i+2+j];
				temp_checkSum += ((temp_checkSum << 8) + buf[i+3+j]);
				if(temp_checkSum == checkSum){
					memmove(buf, &buf[i], valid_frame_len);
					*buf = valid_frame_len;
					return i;
				}
			}		
	}
	return 0xFF;
}

uint8_t process_data_from_node(uint8_t* buf, uint8_t buf_len){
	
	if(data_check(buf, &buf_len)){
		switch(buf[CMD_POSITION]){
			case ACK_CMD:
				if(buf[CMD_POSITION + 1] == 0xFF)
					set_ack_flag();
				break;
			case UPDATE_CONTENT_CMD:
				set_ack_flag();
			default:
				ETH_send(buf, buf_len); 
				break;
		}		
	}
	return 0;
}
// ----    end   ----
