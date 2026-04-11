#include "ctrl_mode.h"
#include "speed_loop.h"
#include "pos_loop.h"


/*==========================================================
 * 私有变量
 *==========================================================*/
static CtrlMode_t s_current_mode = CTRL_MODE_SPEED;

/*==========================================================
 * 设置模式
 *==========================================================*/
void Ctrl_SetMode(CtrlMode_t mode)
{
    s_current_mode = mode;
}

CtrlMode_t Ctrl_GetMode(void)
{
    return s_current_mode;
}

/*==========================================================
 * 统一启动
 *==========================================================*/
void Ctrl_Start(void)
{
    Encoder_Reset(ENCODER_ID_1);
    Ctrl_Enable();   // speed_loop 使能
}

/*==========================================================
 * 统一停止
 *==========================================================*/
void Ctrl_Stop(void)
{
    Ctrl_Disable();  // speed_loop 失能+停机
    PosLoop_Reset();
}

/*==========================================================
 * 统一设置目标
 * SPEED模式     : target_a=rps,          target_b 忽略
 * POS模式       : target_a=目标脉冲数,   target_b=最大速度rps
 * SPEED_POS模式 : target_a=目标脉冲数,   target_b=最大速度rps
 *==========================================================*/
void Ctrl_SetTarget(float target_a, float target_b)
{
    switch (s_current_mode)
    {
        case CTRL_MODE_SPEED:
            Ctrl_Set_Target(target_a);
            break;

        case CTRL_MODE_POS:
        case CTRL_MODE_SPEED_POS:
            PosLoop_SetTarget((int32_t)target_a, target_b);
            break;

        default:
            break;
    }
}
