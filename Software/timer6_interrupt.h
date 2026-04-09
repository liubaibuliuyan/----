#ifndef __TIMER6_INTERRUPT_H
#define __TIMER6_INTERRUPT_H

#include "tim.h"
#include "stdint.h"

/* 对外暴露的时基 */
extern volatile uint32_t g_tim6_1ms_cnt;

/* 对外暴露的闭环状态 */
extern volatile float    g_ctrl_speed_meas;   // 实测速度 r/s
extern volatile float    g_ctrl_speed_target; // 目标速度 r/s
extern volatile int32_t  g_ctrl_total_pulse;  // 累计脉冲
extern volatile uint8_t  g_ctrl_enable;       // 闭环使能

/* 参数设置接口 */
void Ctrl_Set_Target(float target_rps);
void Ctrl_Enable(void);
void Ctrl_Disable(void);

/* 初始化/启停 */
void Timer6_Interrupt_Init(void);
void Timer6_Interrupt_Stop(void);
void Timer6_Interrupt_Restart(void);

#endif
