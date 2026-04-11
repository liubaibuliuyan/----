#ifndef __TIMER7_INTERRUPT_H
#define __TIMER7_INTERRUPT_H

#include "tim.h"
#include <stdint.h>
#include "speed_loop.h"
#include "pos_loop.h" 
void Timer7_Interrupt_Init(void);
void Timer7_Interrupt_Stop(void);
void TIM7_Run(void);

#endif
