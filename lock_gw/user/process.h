#ifndef _PROCESS_H
#define _PROCESS_H
#include "stm32f10x.h"

#define TO_SERVER_HEADER_1		0x5A
#define TO_SERVER_HEADER_2		0x5A
#define FROM_SERVER_FRAME_HEADER_1		0xA5
#define FROM_SERVER_FRAME_HEADER_2 		0xA5

#define CMD_POSITION					7
#define GW_MANUFACTURE_CMD		0x51
#define GW_RESET_CMD					0x53
#define UPDATE_CONTENT_CMD		0x13
#define SET_MANUFACTURE_CMD		0x33
#define ACK_CMD								0xEE

#define ACK_SUCCESS  0
#define ACK_FAILED   1

void ack_server(uint8_t cmd, uint8_t ack);
uint8_t process_data_from_server(uint8_t* buf, uint16_t buf_len);
uint8_t process_data_from_node(uint8_t* buf, uint8_t buf_len);
	
#endif
