#ifndef __CTRL_MODE_H
#define __CTRL_MODE_H

#include <stdint.h>

// 只保留两个有效模式
typedef enum {
    CTRL_MODE_SPEED,     // 纯速度
    CTRL_MODE_POS        // 纯位置
} CtrlMode_t;

void Ctrl_SetMode(CtrlMode_t mode);
CtrlMode_t Ctrl_GetMode(void);

void Ctrl_Start(void);
void Ctrl_Stop(void);

void Ctrl_SetSpeedTarget(float rps);
void Ctrl_SetPosTarget(int32_t pulse, float max_speed_rps);

#endif
