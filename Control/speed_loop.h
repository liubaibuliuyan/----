#ifndef __SPEED_LOOP_H
#define __SPEED_LOOP_H

#include "main.h"
#include "pid.h"
#include "encoder.h"
#include "motor.h"
#include "ctrl_mode.h"
#include "filter.h"   // 添加滤波头文件

#define CTRL_PERIOD_MS   1
#define SPEED_TAU_MS     4.0f
#define CTRL_PERIOD_S    ((float)CTRL_PERIOD_MS / 1000.0f)
#define FEEDFORWARD_Kf   2750.0f

void SpeedLoop_Init(void);
void SpeedLoop_Process(int32_t diff_pulse_1ms, int32_t total_pulse);
void Ctrl_Set_Target(float target_rps);
void Ctrl_Enable(void);
void Ctrl_Disable(void);

#endif
