#ifndef __LED_H__
#define __LED_H__

#include <stdio.h>
#include <stm32f10x.h>
/*
 @brief LED��״̬
 */
enum LED_STATE {
	LEDON, LEDOFF
};

/*
 @brief LED���
 */
enum LED_NO {
	LED_NO_0 = 0,
	LED_NO_1 = 1
};

/*
 @brief LED���������
 */
#define LED_MAX_SIZE (2)

/*
 @brief LED����
 */
struct LedDefine {
	enum LED_NO led_no;
	// PIN_GROUP��GPIOA ...��
	GPIO_TypeDef* pin_group;
	// PIN��GPIO_Pin_1 ...��
	uint16_t pin;
	// LEDON | LEDOFF
	enum LED_STATE defaultstatus;
};

/*
 @brief ��ʼ������LED
 @param leds LED���ò���
 @param len LED������
 */
extern uint8_t LED_Configuration(const struct LedDefine* leds, uint8_t len);

/*
 @brief ����LED
 @param led_no LED���
 */
extern uint8_t LED_ON(enum LED_NO led_no);

/*
 @brief Ϩ��LED
 @param led_no LED���
 */
extern uint8_t LED_OFF(enum LED_NO led_no);

extern bool LED_IS_ON(enum LED_NO led_no);

uint8_t LED_TOGGLE(enum LED_NO led_no);
#endif
