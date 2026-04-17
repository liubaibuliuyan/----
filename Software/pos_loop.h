#ifndef __POS_LOOP_H
#define __POS_LOOP_H

#include "main.h"
#include "pid.h"

#define POS_OUT_MAX         3.0f
#define POS_OUT_MIN        -3.0f
#define POS_FF_K    				0.00012f  // 更小前馈

void    PosLoop_Init(void);
void    PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps);
void    PosLoop_Process(int32_t current_pulse);
void    PosLoop_Reset(void);
uint8_t PosLoop_IsDone(void);

#endif
