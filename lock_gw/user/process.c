#include "process.h"
#include "usart.h"
#include "ethernet.h"
#include "sx1276.h"

uint8_t process_data_from_server(uint8_t* buf, uint8_t buf_len){
	Lora_Send(buf, buf_len);
	return 0;
}

uint8_t process_data_from_node(uint8_t* buf, uint8_t buf_len){
	ETH_send(buf, buf_len);
	return 0;
}
