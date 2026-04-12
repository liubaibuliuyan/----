#include "pos_loop.h"

extern void Ctrl_Set_Target(float target_rps);
extern volatile float g_ctrl_speed_meas;

// 位置环全局变量
static int32_t  s_target_pulse = 0;
static float    s_max_speed    = POS_OUT_MAX;
static uint8_t  s_done_flag    = 0;

// 位置式PI + 优化变量
static float    s_integral = 0.0f;
static float    s_pi_output = 0.0f;

void PosLoop_Init(void)
{
    s_target_pulse = 0;
    s_max_speed    = POS_OUT_MAX;
    s_done_flag    = 0;
    s_integral     = 0.0f;
    s_pi_output    = 0.0f;
}

void PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps)
{
    s_target_pulse = target_pulse;
    s_done_flag    = 0;
    s_integral     = 0.0f;
    s_pi_output    = 0.0f;

    // 最大速度限制
    float limit = (max_speed_rps > 0.1f) ? max_speed_rps : POS_OUT_MAX;
    s_max_speed = (limit > POS_OUT_MAX) ? POS_OUT_MAX : limit;
}

/**
 * @brief  精准定位版位置环：提前减速 + 积分衰减 + 死区防抖
 */
void PosLoop_Process(int32_t current_pulse)
{
    if (s_done_flag)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    // 1. 计算误差
    int32_t err     = s_target_pulse - current_pulse;
    int32_t abs_err = (err >= 0) ? err : -err;

    // 2. 到位判定
    if (abs_err <= POS_DONE_THRESHOLD)
    {
        s_done_flag = 1;
        Ctrl_Set_Target(0.0f);
        s_integral = 0.0f;
        return;
    }

    // 3. 极近距离直接停止
    if (abs_err <= POS_LOCK_DIST)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    // 4. 误差死区，消除抖动
    if (abs_err <= POS_ERROR_DEAD_ZONE)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    // ==================== 5. PI 计算 + 积分分离 ====================
    float p_term = POS_KP * (float)err;

    // 大误差积分，小误差积分衰减
    if (abs_err > POS_INTEGRAL_THRESHOLD)
    {
        s_integral += (float)err * POS_KI;
        if (s_integral >  POS_INTEGRAL_LIMIT)  s_integral =  POS_INTEGRAL_LIMIT;
        if (s_integral < -POS_INTEGRAL_LIMIT)  s_integral = -POS_INTEGRAL_LIMIT;
    }
    else
    {
        s_integral *= 0.8f; // 小误差时积分缓慢衰减，防冲过
    }

    s_pi_output = p_term + s_integral;

    // ==================== 6. 禁止反向输出 ====================
    if (err > 0 && s_pi_output < 0)
        s_pi_output = 0;
    if (err < 0 && s_pi_output > 0)
        s_pi_output = 0;

    // ==================== 7. 提前平滑减速，防止冲终点 ====================
    float speed_cmd = s_pi_output;
    float v_now = (g_ctrl_speed_meas >= 0) ? g_ctrl_speed_meas : -g_ctrl_speed_meas;

    // 制动距离加大，更早减速
    float brake_dist = v_now * 140.0f;
    if (brake_dist < 100.0f) brake_dist = 100.0f;

    if ((float)abs_err < brake_dist)
    {
        // S曲线柔和减速
        float ratio = (float)abs_err / brake_dist;
        float ratio_smooth = ratio * ratio * (3.0f - 2.0f * ratio);
        float dyn_limit = POS_BRAKE2_SPEED + (POS_BRAKE1_SPEED - POS_BRAKE2_SPEED) * ratio_smooth;

        // 速度限幅
        if (speed_cmd >  dyn_limit) speed_cmd =  dyn_limit;
        if (speed_cmd < -dyn_limit) speed_cmd = -dyn_limit;

        // 极小最小速度，精准到位
        if (speed_cmd > 0.0f && speed_cmd < POS_MIN_SPEED)
            speed_cmd = POS_MIN_SPEED;
        else if (speed_cmd < 0.0f && speed_cmd > -POS_MIN_SPEED)
            speed_cmd = -POS_MIN_SPEED;
    }
    else
    {
        // 正常区间限幅
        if (speed_cmd >  s_max_speed) speed_cmd =  s_max_speed;
        if (speed_cmd < -s_max_speed) speed_cmd = -s_max_speed;
    }

    // 输出速度指令
    Ctrl_Set_Target(speed_cmd);
}

void PosLoop_Reset(void)
{
    s_done_flag    = 0;
    s_target_pulse = 0;
    s_integral     = 0.0f;
    s_pi_output    = 0.0f;
}

uint8_t PosLoop_IsDone(void)
{
    return s_done_flag;
}
