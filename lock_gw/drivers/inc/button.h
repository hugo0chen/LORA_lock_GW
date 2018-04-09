#ifndef _BUTTON_H
#define _BUTTON_H

#include "stm32f10x.h"

#define LONG_PRESSED_TIME  3000
#define SHORT_PRESSED_TIME	200

enum BUTTON_TYPE {
	NO_PRESSED = 0,
	SHORT_PRESSED = 1,
	LONG_PRESSED = 2
};

struct BUTTON{
	uint32_t pressed_time;
	uint8_t pressed;
	enum BUTTON_TYPE type;
};

extern struct BUTTON button;

void init_button(void);
void start_button_pressed_time_count(void);
void get_button_value(void);
#endif

//	-------			end 		-----
