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
#include "aes.h"
#include "main.h"
#include "lora_com.h"
#include "temp.h"
#include "button.h"
#include "flash.h"
/****************************** Type Definition ******************************/

/****************************** Function Declaration ******************************/
void initDrivers(void);
static uint8_t flash_running_led(void );

/****************************** Variable Declaration ******************************/
void feed_watchdog(void) {
	//Feed Dog
	IWDG->KR = 0XAAAA;
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
	init_button();
	__enable_irq();
}    

uint8_t flash_running_led(void){
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
		return 1;
	}
	return 0;
}

void heart_beat(void){
	static uint32_t heartTick = 0;
	uint8_t data[] = {0x5A, 0x5A, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0x52, 0x00, 0x00, 0x04, 0x58};
	
	if(timeout(heartTick, HEARTBEAT_PERIOD)){
		ETH_send(data, sizeof(data));
		heartTick = local_ticktime();
	}
}

void mcu_reset(void){	
	__disable_fault_irq(); 
	NVIC_SystemReset();
}

int main( void ){
	uint16_t temp_flash_flag[2] = {0};
	
	initDrivers();	

	getHalfWordData(USER_FLASH_START_ADDR, temp_flash_flag,2 );
	if(temp_flash_flag[0] == 1){
		temp_flash_flag[0] = 0;
		temp_flash_flag[1] = 0;	
		putHalfWordData(USER_FLASH_START_ADDR, temp_flash_flag, 2);
		eth_soft_manufacture_set();
	}

	while (1){
		flash_running_led();
		heart_beat();
		feed_watchdog();
		lora_com_fsm(0);
		
		get_button_value();
	  switch(button.type){
			case LONG_PRESSED:
				temp_flash_flag[0] = 1;
				temp_flash_flag[1] = 1;						
				putHalfWordData(USER_FLASH_START_ADDR, temp_flash_flag, 2);		
				button.type = NO_PRESSED;
				mcu_reset();				
				break;
			case SHORT_PRESSED:
				button.type = NO_PRESSED;		
				ETH_reset();				
				mcu_reset();				
				break;
			default:
				break;		
		}
		
		if(eth_data.Eth_data_arrived){
			process_data_from_server(eth_data.dataBufferFromETH, eth_data.dataBufferFromETH_len);
			eth_data.Eth_data_arrived = 0;
		}				
	}
}

// -----		end			-------
