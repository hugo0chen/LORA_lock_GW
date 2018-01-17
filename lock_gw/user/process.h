#ifndef _PROCESS_H
#define _PROCESS_H
#include "stm32f10x.h"

uint8_t process_data_from_server(uint8_t* buf, uint8_t buf_len);

uint8_t process_data_from_node(uint8_t* buf, uint8_t buf_len);
	
#endif
