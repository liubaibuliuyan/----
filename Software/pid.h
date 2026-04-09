#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef struct {
    /* --- 连续域物理参数 --- */
    float kp;              // 比例增益 [PWM / (r/s)]
    float ti;              // 积分时间常数 [秒]，越小积分越强
    float td;              // 微分时间常数 [秒]，速度环通常为0

    /* --- 离散域计算系数 (缓存) --- */
    float coeff_p;
    float coeff_i;
    float coeff_d;

    /* --- 系统配置 --- */
    float ts;              // 采样周期 [秒]

    /* --- 运行状态 --- */
    float target;
    float last_error;
    float prev_error;
    float output;

    /* --- 输出保护 --- */
    float out_min;
    float out_max;
} PID_t;

void PID_Init(PID_t *pid,
              float kp, float ti, float td, float ts,
              float out_min, float out_max);

void PID_Set_Target(PID_t *pid, float target);
float PID_Calc(PID_t *pid, float measured);
void PID_Reset(PID_t *pid);

#ifdef __cplusplus
}
#endif

#endif
