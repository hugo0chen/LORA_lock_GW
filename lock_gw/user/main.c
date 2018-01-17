#include <stm32f10x.h>
#include <string.h>
#include "GlobalVar.h"
#include "usart.h"
#include "Delay.h"
#include "led.h"
#include "sx1276.h"
#include "driversInit.h"
#include "ethernet.h"
#include "process.h"

/****************************** Type Definition ******************************/
enum FsmStatus {
		POWER_ON,
		CONFIG,
    WORKING    
};

/****************************** Macro Definition ******************************/
#define LED_RUNNING_PERIOD		1000

/****************************** Function Declaration ******************************/
static void initDrivers(void);
static void flash_running_led(void );
static void fsm( void);

/****************************** Variable Declaration ******************************/
enum FsmStatus	_fsm_status	 = WORKING;

int main( void ){             
	initDrivers();
	
	printf("lock gateway starting");
	ETH_send( "lock gateway starting", sizeof("lock gateway starting"));	
	
	while ( 1 ){
		feed_watchdog();
		flash_running_led();

	  fsm();
	}
}

void initDrivers(void){
	RCC_Configuration();
	__disable_irq();
	BOARD_NVIC_Init(NVIC_PriorityGroup_2);	
	init_gpio_remap();	
	init_sys_tick();
	init_led();
	init_uart();
	init_spi();
	init_lora();
	init_eth();
	init_wtd();
	__enable_irq();
}    

void flash_running_led(){
	static uint8_t	ledStatus	= 0;
	static uint32_t runningTick	= 0;
    
	if ( timeout( runningTick, LED_RUNNING_PERIOD ) ){		
		if ( ledStatus == 0 ){
			LED_ON( LED_NO_0 );
			ledStatus = 1;
		} else {
			LED_OFF( LED_NO_0 );
			ledStatus = 0;
		}
		runningTick = local_ticktime();
	}
}
//void lora_send_test(void){
//	static uint32_t sentTick	= 0;
//
//	lora_data.lora_recv_data_buf_len = 10;
//	memset(lora_data.lora_recv_data_buf, 'A', lora_data.lora_recv_data_buf_len);     
//	if ( timeout( sentTick, 5000 ) ){		
//		_usart_sendMultiBytes(USART1, "LORA sending", sizeof("LORA sending"));
//		_usart_sendMultiBytes(USART1, lora_data.lora_recv_data_buf, lora_data.lora_recv_data_buf_len);
//		Lora_Send(lora_data.lora_recv_data_buf, lora_data.lora_recv_data_buf_len);
//		sentTick = local_ticktime();
//	}
//}

void fsm( void ){	
	switch ( _fsm_status ){
		case POWER_ON: {
				//todo 检测TCP建立连接的引脚，引脚拉低，tcp成功
				break;
		}
		case CONFIG: {
				//todo	
				break;
		}
		case WORKING: {
				if(eth_data.Eth_data_arrived){				
					process_data_from_server(eth_data.dataBufferFromETH, eth_data.dataBufferFromETH_len);
					eth_data.Eth_data_arrived = 0;
				}
				if(lora_data.lora_data_arrived){
					process_data_from_node(lora_data.lora_recv_data_buf, lora_data.lora_recv_data_buf_len);
					lora_data.lora_data_arrived = 0;
				}
				break;
		}
		default: 
			break;
	}
}
