#include "pos_loop.h"
#include "speed_loop.h"
#include <math.h>

extern void Ctrl_Set_Target(float target_rps);

static int32_t  s_target_pulse = 0;
static float    s_max_speed    = POS_OUT_MAX;
static uint8_t  s_done_flag    = 0;

static PosPID_t s_pos_pid;

void PosLoop_Init(void)
{
    PosPID_Init(&s_pos_pid,
                0.0028f,
                0.00040f,
                0.0f,
                220.0f,
                POS_OUT_MIN, POS_OUT_MAX);

    // 积分分离：远离目标才积分，靠近清零，彻底防过冲
    PosPID_ConfigIntegralSep(&s_pos_pid, true, 50.0f);

    s_target_pulse = 0;
    s_max_speed    = POS_OUT_MAX;
    s_done_flag    = 0;
}

void PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps)
{
    s_target_pulse = target_pulse;
    s_done_flag    = 0;

    PosPID_Reset(&s_pos_pid);
    PosPID_SetTarget(&s_pos_pid, (float)target_pulse);

    float limit = max_speed_rps > 0.1f ? max_speed_rps : POS_OUT_MAX;
    s_max_speed = limit > POS_OUT_MAX ? POS_OUT_MAX : limit;
}

void PosLoop_Process(int32_t current_pulse)
{
    if (s_done_flag)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    float err     = (float)(s_target_pulse - current_pulse);
    float abs_err = fabsf(err);

    // ===================== 核心：柔顺速度规划 =====================
    // 速度 随 误差 线性衰减 → 自然梯形曲线
    float pid_out  = PosPID_Calc(&s_pos_pid, (float)current_pulse);
    float ff_out   = err * POS_FF_K;
    float speed_cmd = pid_out + ff_out;

    // 限幅（无方向钳位、不拉回、不反转）
    if (speed_cmd >  s_max_speed) speed_cmd =  s_max_speed;
    if (speed_cmd < -s_max_speed) speed_cmd = -s_max_speed;

    // ===================== 自然静止，无抖动 =====================
    if (abs_err <= 2)
    {
        speed_cmd = 0.0f;
        s_done_flag = 1;
        PosPID_Reset(&s_pos_pid);
    }

    Ctrl_Set_Target(speed_cmd);
}

void PosLoop_Reset(void)
{
    s_done_flag    = 0;
    s_target_pulse = 0;
    PosPID_Reset(&s_pos_pid);
}

uint8_t PosLoop_IsDone(void)
{
    return s_done_flag;
}
