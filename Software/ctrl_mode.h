#ifndef __CTRL_MODE_H
#define __CTRL_MODE_H

#include "stdint.h"

/*==========================================================
 * 控制模式枚举
 *==========================================================*/
typedef enum {
    CTRL_MODE_SPEED     = 0,   // 纯速度环
    CTRL_MODE_POS       = 1,   // 纯位置环
    CTRL_MODE_SPEED_POS = 2,   // 速度+位置（梯形轨迹/限速跟位置）
} CtrlMode_t;

/*==========================================================
 * 统一对外控制接口
 *==========================================================*/
void Ctrl_SetMode(CtrlMode_t mode);
CtrlMode_t Ctrl_GetMode(void);

// 启动/停止（复用原有使能接口，此处做统一入口）
void Ctrl_Start(void);
void Ctrl_Stop(void);

// 设置目标（根据当前模式解释参数含义）
// SPEED模式：target_a = 目标转速(rps)，target_b 忽略
// POS模式：target_a = 目标脉冲数，target_b = 最大速度(rps)
// SPEED_POS模式：target_a = 目标脉冲数，target_b = 最大速度(rps)
void Ctrl_SetTarget(float target_a, float target_b);

#endif
