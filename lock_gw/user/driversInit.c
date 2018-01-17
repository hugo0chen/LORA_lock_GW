#include <stm32f10x.h>
#include "usart.h"
#include "led.h"
#include "GlobalVar.h"
#include "driversInit.h"
#include "spi.h"
#include "sx1276.h"
#include "ethernet.h"

extern int receiveByteFromEth( enum BOARD_USART_TYPE usart_no, unsigned char recv);

uint16_t BOARD_NVIC_Init(uint16_t priority_group){
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);//将中断矢量放到Flash的0地址
  NVIC_PriorityGroupConfig(priority_group);//设置优先级配置的模式，详情请阅读原材料中的文章
	return 0;
}
void RCC_Configuration(){
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );
}

void init_gpio_remap(){
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_JTAGDisable, ENABLE );	
}
 
void init_sys_tick(){
    RCC_ClocksTypeDef RCC_Clocks;
	
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 100);
    NVIC_SetPriority (SysTick_IRQn, 1);	
}

//Tout=((4*2^prer)*rlr)/40 (ms).
void init_wtd(void){
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  
	IWDG_SetPrescaler(IWDG_Prescaler_256);  
	IWDG_SetReload(0x00FF);  								
	IWDG_ReloadCounter();  
	IWDG_Enable(); 	
}

void init_uart(){
	struct USARTDefine conf[] = {
		{ BOARD_USART_1, USART1, GPIOA, GPIO_Pin_9, GPIOA, GPIO_Pin_10, 115200, RCC_APB2Periph_USART1, USART1_IRQn, receiveByteFromEth }, //eth
		{ BOARD_USART_2, USART2, GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3,  115200, RCC_APB1Periph_USART2, USART2_IRQn, 0 },	 //debug
	};
	USART_Configuration( conf, ARRAY_SIZE( conf, struct USARTDefine ) );
}

void init_led(){
	struct LedDefine conf[] = {
		{ LED_NO_0, GPIOB, GPIO_Pin_7, LEDOFF },  
	};
	LED_Configuration( conf, ARRAY_SIZE( conf, struct LedDefine ) );
}

void init_spi(void){
		SPI1_Init();
}

void init_lora(void){
		Lora_Init();
}

void init_eth(){
	struct EthDefine conf = {
		GPIOC, GPIO_Pin_9, //RST
		GPIOA, GPIO_Pin_8  //CFG
	};
	
	ETH_Configuration(conf);
}
