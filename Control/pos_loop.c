#include "pos_loop.h"
#include "speed_loop.h"
#include <math.h>

extern void Ctrl_Set_Target(float target_rps);

// 位置环内部状态 打包结构体
typedef struct {
    int32_t  target_pulse;
    float    max_speed;
    uint8_t  done_flag;
} PosLoopStatus_t;

// 初始化
static PosLoopStatus_t s_pos = {
    .target_pulse = 0,
    .max_speed    = POS_OUT_MAX,
    .done_flag    = 0
};

static PosPID_t s_pos_pid;

void PosLoop_Init(void)
{
    PosPID_Init(&s_pos_pid,
                0.0028f,
                0.00040f,
                0.0f,
                220.0f,
                POS_OUT_MIN, POS_OUT_MAX);

    // 积分分离：远离目标才积分，靠近清零
    PosPID_ConfigIntegralSep(&s_pos_pid, true, 50.0f);

    s_pos.target_pulse = 0;
    s_pos.max_speed    = POS_OUT_MAX;
    s_pos.done_flag    = 0;
}

// 这里修复为 float，和.h完全一致
void PosLoop_SetTarget(int32_t target_pulse, float max_speed_rps)
{
    s_pos.target_pulse = target_pulse;
    s_pos.done_flag    = 0;

    PosPID_Reset(&s_pos_pid);
    PosPID_SetTarget(&s_pos_pid, (float)target_pulse);

    float limit = max_speed_rps > 0.1f ? max_speed_rps : POS_OUT_MAX;
    s_pos.max_speed = limit > POS_OUT_MAX ? POS_OUT_MAX : limit;
}

void PosLoop_Process(int32_t current_pulse)
{
    if (s_pos.done_flag)
    {
        Ctrl_Set_Target(0.0f);
        return;
    }

    float err     = (float)(s_pos.target_pulse - current_pulse);
    float abs_err = fabsf(err);

    float pid_out  = PosPID_Calc(&s_pos_pid, (float)current_pulse);
    float ff_out   = err * POS_FF_K;
    float speed_cmd = pid_out + ff_out;

    if (speed_cmd >  s_pos.max_speed) speed_cmd =  s_pos.max_speed;
    if (speed_cmd < -s_pos.max_speed) speed_cmd = -s_pos.max_speed;

    if (abs_err <= 2)
    {
        speed_cmd = 0.0f;
        s_pos.done_flag = 1;
        PosPID_Reset(&s_pos_pid);
    }

    Ctrl_Set_Target(speed_cmd);
}

void PosLoop_Reset(void)
{
    s_pos.done_flag    = 0;
    s_pos.target_pulse = 0;
    PosPID_Reset(&s_pos_pid);
}

uint8_t PosLoop_IsDone(void)
{
    return s_pos.done_flag;
}
