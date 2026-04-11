#include "timer7_interrupt.h"


/*==========================================================
 * TIM7 100μs 中断：仅调度，无业务逻辑
 *==========================================================*/
void TIM7_Run(void)
{
    int16_t diff = 0;
    int32_t total = 0;

    // 1. 读取编码器
    Encoder_Read(ENCODER_ID_1, &diff, &total);

    // 2. 调用独立速度环模块
    SpeedLoop_Process(diff, total);
}

/*==========================================================
 * 初始化
 *==========================================================*/
void Timer7_Interrupt_Init(void)
{
    // 初始化速度环
    SpeedLoop_Init();
		PosLoop_Init(); 

    // 启动定时器中断
    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
        Error_Handler();
}

void Timer7_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim7);
}
