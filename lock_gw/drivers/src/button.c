#include "button.h"
#include "GlobalVar.h"
// PA3  ---  manufacture set button

struct BUTTON button;

void init_button(void){
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
  
  NVIC_InitStructure.NVIC_IRQChannel 		               = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd 	             = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  EXTI_InitStructure.EXTI_Line    = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init( &EXTI_InitStructure );
      
  EXTI_ClearITPendingBit( EXTI_Line3 );
	
}
	
void start_button_pressed_time_count(void){
	button.pressed_time = local_ticktime();
	button.pressed = 1;
}

void get_button_value(void){
	if(button.pressed == 1){
		if( timeout(button.pressed_time, LONG_PRESSED_TIME) ){			
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == RESET){			
				button.type = LONG_PRESSED;
			}else{
				button.type = SHORT_PRESSED;
			}
			while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == RESET);
			button.pressed = 0;
		}		
	}
}

//		----			end			----
