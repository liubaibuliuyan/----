#include "ctrl_mode.h"
#include "speed_loop.h"
#include "pos_loop.h"

Ctrl_t g_ctrl;  // 全局控制结构体

void Ctrl_SetMode(CtrlMode_t mode)
{
    g_ctrl.mode = mode;        // 必须同步更新结构体！
    // s_current_mode 删掉，完全用 g_ctrl
}

CtrlMode_t Ctrl_GetMode(void)
{
    return g_ctrl.mode;        // 读结构体
}

void Ctrl_Start(void)
{
    Encoder_Reset(ENCODER_ID_1);
    Ctrl_Enable();
}

void Ctrl_Stop(void)
{
    Ctrl_Disable();
    PosLoop_Reset();
}

void Ctrl_SetSpeedTarget(float rps)
{
    Ctrl_Set_Target(rps);
}

void Ctrl_SetPosTarget(int32_t pulse, float max_speed_rps)
{
    PosLoop_SetTarget(pulse, max_speed_rps);
}
