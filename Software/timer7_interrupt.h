#ifndef __TIMER7_INTERRUPT_H
#define __TIMER7_INTERRUPT_H

#include "tim.h"
#include <stdint.h>
#include "encoder.h"
#include "motor.h"
#include "pid.h"

extern volatile float    g_ctrl_speed_meas;
extern volatile float    g_ctrl_speed_target;
extern volatile int32_t  g_ctrl_total_pulse;
extern volatile uint8_t  g_ctrl_enable;

void Ctrl_Set_Target(float target_rps);
void Ctrl_Enable(void);
void Ctrl_Disable(void);

void Timer7_Interrupt_Init(void);
void Timer7_Interrupt_Stop(void);
void TIM7_Run(void);

#endif
