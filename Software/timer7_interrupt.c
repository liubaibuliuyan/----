#include "timer7_interrupt.h"

/*==========================================================
 * TIM7 初始化（只开中断，无业务）
 *==========================================================*/
void Timer7_Interrupt_Init(void)
{
    // 开启 TIM7 中断
    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
        Error_Handler();
}

/*==========================================================
 * 停止 TIM7
 *==========================================================*/
void Timer7_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim7);
}
