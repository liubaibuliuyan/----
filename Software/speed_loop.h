#ifndef __SPEED_LOOP_H
#define __SPEED_LOOP_H

#include "main.h"
#include "pid.h"
#include "encoder.h"
#include "motor.h"


#define CTRL_PERIOD_MS   1
#define SPEED_TAU_MS     7.0f
#define CTRL_PERIOD_S    ((float)CTRL_PERIOD_MS / 1000.0f)
#define FEEDFORWARD_Kf   2750.0f

extern volatile float    g_ctrl_speed_meas;
extern volatile float    g_ctrl_speed_target;
extern volatile int32_t  g_ctrl_total_pulse;
extern volatile uint8_t  g_ctrl_enable;

void SpeedLoop_Init(void);
void SpeedLoop_Process(int16_t diff_pulse, int32_t total_pulse);
void Ctrl_Set_Target(float target_rps);
void Ctrl_Enable(void);
void Ctrl_Disable(void);

#endif
