#include "ctrl_mode.h"
#include "speed_loop.h"
#include "pos_loop.h"

static CtrlMode_t s_current_mode = CTRL_MODE_SPEED;

void Ctrl_SetMode(CtrlMode_t mode)
{
    s_current_mode = mode;
}

CtrlMode_t Ctrl_GetMode(void)
{
    return s_current_mode;
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
