#ifndef __DRIVERS_INIT_H
#define __DRIVERS_INIT_H

void RCC_Configuration(void);
uint16_t BOARD_NVIC_Init(uint16_t priority_group);
void init_sys_tick(void);
void init_gpio_remap(void);
void init_wtd(void);
void init_led(void);
void init_uart(void);
void init_spi(void);
void init_lora(void);
void init_eth(void);
#endif 

