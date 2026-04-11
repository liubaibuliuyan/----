#ifndef __POS_LOOP_H
#define __POS_LOOP_H

#include "main.h"

#define POS_KP               0.005f
#define POS_OUT_MAX          5.0f
#define POS_OUT_MIN         -5.0f

#define POS_DONE_THRESHOLD   3      // 最终到位死区 ±5脉冲

#define POS_BRAKE1_SPEED     1.0f   // 第一制动段限速
#define POS_BRAKE2_SPEED     0.20f  // 第二制动段限速

// ★ 制动锁定区：距目标≤此值时强制速度指令=0，靠惯性滑入死区
// 设为15：速度0.20rps时滑行距离≈0.20×130×0.1≈2.6脉冲，能进入±5
#define POS_LOCK_DIST        35

#define POS_MIN_SPEED        0.18f  // 制动区外的最低有效速度

void    PosLoop_Init(void);
void    PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps);
void    PosLoop_Process(int32_t current_pulse);
void    PosLoop_Reset(void);
uint8_t PosLoop_IsDone(void);

#endif
