#ifndef __TIMER7_INTERRUPT_H
#define __TIMER7_INTERRUPT_H

#include "tim.h"
#include <stdint.h>

// 双环频率配置，可调整
#define SPEED_LOOP_PERIOD_TICKS  10  // 速度环：10*100μs=1ms → 1kHz
#define POS_LOOP_PERIOD_TICKS    50  // 位置环：50*100μs=5ms → 200Hz

// 编码器全局累计变量
extern volatile int32_t g_enc_diff_acc;
extern volatile int32_t g_enc_total_pulse;

void Timer7_Interrupt_Init(void);
void Timer7_Interrupt_Stop(void);
void TIM7_Run(void);

#endif
