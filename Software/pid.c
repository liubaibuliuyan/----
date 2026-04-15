#include "pid.h"

// ===================== 增量式PID 私有函数 =====================
static void _IncPID_CalcCoeffs(IncPID_t *pid)
{
    pid->coeff_p = pid->kp;

    if(pid->ti > 0.00001f) {
        pid->coeff_i = pid->kp * (pid->ts / pid->ti);
    } else {
        pid->coeff_i = 0.0f;
    }

    if(pid->ts > 0.00001f) {
        pid->coeff_d = pid->kp * (pid->td / pid->ts);
    } else {
        pid->coeff_d = 0.0f;
    }
}

// ===================== 增量式PID 接口 =====================
void IncPID_Init(IncPID_t *pid, float kp, float ti, float td, float ts,
                 float out_min, float out_max)
{
    pid->kp = kp;
    pid->ti = ti;
    pid->td = td;
    pid->ts = ts;
    pid->out_min = out_min;
    pid->out_max = out_max;

    _IncPID_CalcCoeffs(pid);

    pid->target = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

void IncPID_SetTarget(IncPID_t *pid, float target)
{
    pid->target = target;
}

float IncPID_Calc(IncPID_t *pid, float measured)
{
    float error = pid->target - measured;

    float delta_p = pid->coeff_p * (error - pid->last_error);
    float delta_i = pid->coeff_i * error;
    float delta_d = pid->coeff_d * (error - 2.0f * pid->last_error + pid->prev_error);

    float delta_out = delta_p + delta_i + delta_d;
    float output = pid->output + delta_out;

    // 输出限幅 + 抗积分饱和
    if(output > pid->out_max) output = pid->out_max;
    if(output < pid->out_min) output = pid->out_min;

    pid->prev_error = pid->last_error;
    pid->last_error = error;
    pid->output = output;

    return output;
}

void IncPID_Reset(IncPID_t *pid)
{
    pid->prev_error = 0.0f;
    pid->last_error = 0.0f;
    pid->output = 0.0f;
}

// ===================== 位置式PID 接口 =====================
void PosPID_Init(PosPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float out_min, float out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral_limit = integral_limit;
    pid->out_min = out_min;
    pid->out_max = out_max;

    pid->integral_sep_en = false;
    pid->integral_thr = 0.0f;

    PosPID_Reset(pid);
}

// 积分分离配置
void PosPID_ConfigIntegralSep(PosPID_t *pid, bool enable, float threshold)
{
    pid->integral_sep_en = enable;
    pid->integral_thr = threshold;
}

void PosPID_SetTarget(PosPID_t *pid, float target)
{
    pid->target = target;
}

float PosPID_Calc(PosPID_t *pid, float measured)
{
    pid->error = pid->target - measured;
    float abs_err = (pid->error >= 0) ? pid->error : -pid->error;

    // 比例项
    float p_term = pid->kp * pid->error;

    // 积分项 + 积分分离
    float i_term = 0.0f;
    if(pid->ki > 0.00001f)
    {
        // 积分分离：大误差才积分
        if(pid->integral_sep_en)
        {
            if(abs_err > pid->integral_thr)
            {
                // 不积分
            }
            else
            {
                pid->integral += pid->error;
            }
        }
        else
        {
            pid->integral += pid->error;
        }

        // 积分限幅
        if(pid->integral > pid->integral_limit)  pid->integral = pid->integral_limit;
        if(pid->integral < -pid->integral_limit) pid->integral = -pid->integral_limit;

        i_term = pid->ki * pid->integral;
    }

    // 微分项
    float d_term = pid->kd * (pid->error - pid->last_error);
    pid->last_error = pid->error;

    // 总输出
    float output = p_term + i_term + d_term;

    // 输出限幅
    if(output > pid->out_max) output = pid->out_max;
    if(output < pid->out_min) output = pid->out_min;

    pid->output = output;
    return output;
}

void PosPID_Reset(PosPID_t *pid)
{
    pid->target = 0.0f;
    pid->error = 0.0f;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output = 0.0f;
}
