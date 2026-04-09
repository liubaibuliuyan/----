#include "timer6_interrupt.h"

volatile uint32_t g_tim6_1ms_cnt = 0;

/*==========================================================
 * TIM6 1ms 中断：只做系统时钟
 *==========================================================*/
void TIM6_Run(void)
{
    g_tim6_1ms_cnt++;
}

/*==========================================================
 * 初始化
 *==========================================================*/
void Timer6_Interrupt_Init(void)
{
    g_tim6_1ms_cnt = 0;

    if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
        Error_Handler();
}

void Timer6_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim6);
}
