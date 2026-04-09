#ifndef __TIMER7_INTERRUPT_H
#define __TIMER7_INTERRUPT_H

#include "tim.h"
#include "stdint.h"

/* 对外暴露：100μs 时基 */
extern volatile uint32_t g_tim7_100us_cnt;

/* 初始化/停止 */
void Timer7_Interrupt_Init(void);
void Timer7_Interrupt_Stop(void);

#endif
