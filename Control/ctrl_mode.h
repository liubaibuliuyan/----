#ifndef __CTRL_MODE_H
#define __CTRL_MODE_H

#include <stdint.h>

// 只保留两个有效模式
typedef enum {
    CTRL_MODE_SPEED,     // 纯速度
    CTRL_MODE_POS        // 纯位置
} CtrlMode_t;

// 电机状态
typedef struct {
    float speed_meas;      // 实际速度
    float speed_target;    // 目标速度
    int32_t total_pulse;   // 总脉冲
    uint8_t enable;        // 电机使能
    CtrlMode_t mode;       // 当前模式
} Ctrl_t;

// 全局唯一实例（只这一个）
extern Ctrl_t g_ctrl;

void Ctrl_SetMode(CtrlMode_t mode);
CtrlMode_t Ctrl_GetMode(void);

void Ctrl_Start(void);
void Ctrl_Stop(void);

void Ctrl_SetSpeedTarget(float rps);
void Ctrl_SetPosTarget(int32_t pulse, float max_speed_rps);

#endif
