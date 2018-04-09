#include "GlobalVar.h"
#include "schd.h"


/*
 SysTick����ʱ�䣬���ڼ���ʱ������ÿ10ms����һ�Σ�
 */
__IO uint32_t _localTimeTick_ms = 0;
#define SYSTEMTICK_PERIOD_MS  10

void SysTick_Handler(void) {
	_localTimeTick_ms += SYSTEMTICK_PERIOD_MS;
	
	// ���ö�ʱ������
	Schd_Run(_localTimeTick_ms);
}

uint32_t local_ticktime() {
	return _localTimeTick_ms;
}

bool timeout(uint32_t last_time, uint32_t ms) {
	return (bool)((_localTimeTick_ms - last_time) > ms);
}
