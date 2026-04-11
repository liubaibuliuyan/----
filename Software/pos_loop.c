#include "pos_loop.h"

extern void Ctrl_Set_Target(float target_rps);
extern volatile float g_ctrl_speed_meas;

static int32_t  s_target_pulse = 0;
static float    s_max_speed    = POS_OUT_MAX;
static uint8_t  s_done_flag    = 0;

void PosLoop_Init(void)
{
    s_target_pulse = 0;
    s_max_speed    = POS_OUT_MAX;
    s_done_flag    = 0;
}

void PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps)
{
    s_target_pulse = target_pulse;
    s_done_flag    = 0;

    float limit = (max_speed_rps > 0.1f) ? max_speed_rps : POS_OUT_MAX;
    if (limit > POS_OUT_MAX) limit = POS_OUT_MAX;
    s_max_speed = limit;
}

void PosLoop_Process(int32_t current_pulse)
{
    if (s_done_flag) return;

    int32_t err     = s_target_pulse - current_pulse;
    int32_t abs_err = (err > 0) ? err : -err;

    // 到位死区
    if (abs_err <= POS_DONE_THRESHOLD)
    {
        s_done_flag = 1;
        Ctrl_Set_Target(0.0f);
        return;
    }

    // 制动锁定区：强制0速，靠惯性滑入死区
    if (abs_err <= POS_LOCK_DIST)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    // 纯P速度指令
    float speed_cmd = POS_KP * (float)err;

    // 动态制动距离
    float v_now = g_ctrl_speed_meas;
    if (v_now < 0.0f) v_now = -v_now;
    float brake_dist = v_now * 120.0f;
    if (brake_dist < 80.0f) brake_dist = 85.0f;  // 保底80脉冲

    if ((float)abs_err < brake_dist)
    {
        // ★ 二次曲线插值：ratio²让速度在制动区前段快速降下来
        // ratio=1.0(制动区入口) → limit=BRAKE1_SPEED=1.0
        // ratio=0.5(制动区中段) → limit²=0.25 → 快速降到低速
        // ratio=0.0(锁定区边界) → limit=BRAKE2_SPEED=0.20
        float ratio     = (float)abs_err / brake_dist;   // 0~1
        float ratio_sq  = ratio * ratio;                  // 二次，更激进
        float dyn_limit = POS_BRAKE2_SPEED
                        + (POS_BRAKE1_SPEED - POS_BRAKE2_SPEED) * ratio_sq;

        if (speed_cmd >  dyn_limit) speed_cmd =  dyn_limit;
        if (speed_cmd < -dyn_limit) speed_cmd = -dyn_limit;

        // 最低速保障（制动区外侧防卡死）
        if (speed_cmd > 0.0f && speed_cmd < POS_MIN_SPEED)
            speed_cmd = POS_MIN_SPEED;
        else if (speed_cmd < 0.0f && speed_cmd > -POS_MIN_SPEED)
            speed_cmd = -POS_MIN_SPEED;
    }
    else
    {
        if (speed_cmd >  s_max_speed) speed_cmd =  s_max_speed;
        if (speed_cmd < -s_max_speed) speed_cmd = -s_max_speed;
    }

    Ctrl_Set_Target(speed_cmd);
}

void PosLoop_Reset(void)
{
    s_done_flag    = 0;
    s_target_pulse = 0;
}

uint8_t PosLoop_IsDone(void)
{
    return s_done_flag;
}
