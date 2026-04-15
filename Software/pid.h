#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// ===================== 增量式PID（速度环专用）=====================
typedef struct {
    // 连续域参数
    float kp;
    float ti;
    float td;
    // 离散系数
    float coeff_p;
    float coeff_i;
    float coeff_d;
    // 系统配置
    float ts;           // 采样周期(s)
    // 运行状态
    float target;
    float last_error;
    float prev_error;
    float output;
    // 输出限幅
    float out_min;
    float out_max;
} IncPID_t;

// ===================== 位置式PID（位置环专用）=====================
typedef struct {
    // 控制参数
    float kp;
    float ki;
    float kd;           // 位置环一般设0
    // 运行状态
    float target;
    float error;
    float integral;
    float last_error;
    float output;
    // 限幅配置
    float integral_limit;   // 积分限幅
    float out_min;
    float out_max;
    // 功能开关
    bool integral_sep_en;   // 积分分离使能
    float integral_thr;     // 积分分离阈值
} PosPID_t;

// ===================== 增量式PID 接口（速度环）=====================
void IncPID_Init(IncPID_t *pid, float kp, float ti, float td, float ts,
                 float out_min, float out_max);
void IncPID_SetTarget(IncPID_t *pid, float target);
float IncPID_Calc(IncPID_t *pid, float measured);
void IncPID_Reset(IncPID_t *pid);

// ===================== 位置式PID 接口（位置环）=====================
void PosPID_Init(PosPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float out_min, float out_max);
void PosPID_ConfigIntegralSep(PosPID_t *pid, bool enable, float threshold);
void PosPID_SetTarget(PosPID_t *pid, float target);
float PosPID_Calc(PosPID_t *pid, float measured);
void PosPID_Reset(PosPID_t *pid);

#ifdef __cplusplus
}
#endif

#endif
