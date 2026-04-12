#ifndef __POS_LOOP_H
#define __POS_LOOP_H

#include "main.h"

// ==================== 终极精准版：定位±2脉冲 ====================
#define POS_KP              0.0070f
#define POS_KI              0.0005f
#define POS_INTEGRAL_LIMIT  500.0f

#define POS_OUT_MAX         4.5f
#define POS_OUT_MIN        -4.5f
#define POS_MIN_SPEED       0.08f

#define POS_DONE_THRESHOLD  2
#define POS_LOCK_DIST       15

#define POS_BRAKE1_SPEED    1.0f
#define POS_BRAKE2_SPEED    0.10f

#define POS_INTEGRAL_THRESHOLD  120
#define POS_ERROR_DEAD_ZONE      4

// ==========================================================

void    PosLoop_Init(void);
void    PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps);
void    PosLoop_Process(int32_t current_pulse);
void    PosLoop_Reset(void);
uint8_t PosLoop_IsDone(void);

#endif
