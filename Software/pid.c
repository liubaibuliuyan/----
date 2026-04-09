#include "pid.h"

// 内部静态函数：仅负责系数计算
static void _PID_Compute_Coeffs(PID_t *pid)
{
    pid->coeff_p = pid->kp;

    if (pid->ti > 0.00001f) {
        pid->coeff_i = pid->kp * (pid->ts / pid->ti);
    } else {
        pid->coeff_i = 0.0f;
    }

    if (pid->ts > 0.00001f) {
        pid->coeff_d = pid->kp * (pid->td / pid->ts);
    } else {
        pid->coeff_d = 0.0f;
    }
}

void PID_Init(PID_t *pid,
              float kp, float ti, float td, float ts,
              float out_min, float out_max)
{
    // 保存参数
    pid->kp = kp;
    pid->ti = ti;
    pid->td = td;
    pid->ts = ts;
    pid->out_min = out_min;
    pid->out_max = out_max;

    // 计算离散系数
    _PID_Compute_Coeffs(pid);

    // 重置状态
    pid->target = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

void PID_Set_Target(PID_t *pid, float target)
{
    pid->target = target;
}

float PID_Calc(PID_t *pid, float measured)
{
    float error = pid->target - measured;

    // 增量计算
    float delta_p = pid->coeff_p * (error - pid->last_error);
    float delta_i = pid->coeff_i * error;
    float delta_d = pid->coeff_d * (error - 2.0f * pid->last_error + pid->prev_error);
    
    float delta_output = delta_p + delta_i + delta_d;
    float output = pid->output + delta_output;

    // 输出限幅 (隐含积分抗饱和：因为 pid->output 保存的是限幅后的值)
    if (output > pid->out_max) output = pid->out_max;
    if (output < pid->out_min) output = pid->out_min;

    // 更新状态
    pid->prev_error = pid->last_error;
    pid->last_error = error;
    pid->output     = output;

    return output;
}

void PID_Reset(PID_t *pid)
{
    pid->prev_error = 0.0f;
    pid->last_error = 0.0f;
    pid->output     = 0.0f;
}
