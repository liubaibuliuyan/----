#ifndef __TIMER6_INTERRUPT_H
#define __TIMER6_INTERRUPT_H

#include "tim.h"
#include <stdint.h>

extern volatile uint32_t g_tim6_1ms_cnt;

void Timer6_Interrupt_Init(void);
void Timer6_Interrupt_Stop(void);
void TIM6_Run(void);

#endif
