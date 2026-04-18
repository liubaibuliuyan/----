#include "timer7_interrupt.h"
#include "encoder.h"
#include "speed_loop.h"
#include "pos_loop.h"
#include "ctrl_mode.h"

static volatile uint32_t s_tick_cnt = 0;
// 编码器结构体，中断内每100μs更新
volatile Enc_t g_enc;

void TIM7_Run(void)
{
    s_tick_cnt++;
    int16_t enc_diff = 0;
    int32_t enc_total = 0;

    // 每100μs必须读取编码器，累计增量
    Encoder_Read(ENCODER_ID_1, &enc_diff, &enc_total);
    g_enc.diff_acc += enc_diff;
    g_enc.total_pulse = enc_total;

    // 速度环调度：1ms一次，传入1ms累计的总脉冲
    if (s_tick_cnt % SPEED_LOOP_PERIOD_TICKS == 0)
    {
        SpeedLoop_Process(g_enc.diff_acc, g_enc.total_pulse);
        g_enc.diff_acc = 0; // 计算完成后清零，重新累计下一个1ms的脉冲
    }

    // 位置环调度：5ms一次，独立运行，和速度环解耦
    if (s_tick_cnt % POS_LOOP_PERIOD_TICKS == 0)
    {
        CtrlMode_t mode = Ctrl_GetMode();
        // =====================  =====================
        if (g_ctrl.enable && mode == CTRL_MODE_POS)
        {
            PosLoop_Process(g_enc.total_pulse);
        }
    }

    // 计数溢出保护
    if (s_tick_cnt >= 10000) s_tick_cnt = 0;
}

void Timer7_Interrupt_Init(void)
{
    s_tick_cnt = 0;
    g_enc.diff_acc = 0;
    g_enc.total_pulse = 0;
    
    SpeedLoop_Init();
    PosLoop_Init();

    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
        Error_Handler();
}

void Timer7_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim7);
}
