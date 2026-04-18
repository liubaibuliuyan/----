#include "dwt_delay.h"

uint32_t DWT_Init(void)
{
    /* 使能调试追踪模块 */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;

    /* 使能 DWT 周期计数器 */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;

    /* 清零计数器 */
    DWT->CYCCNT = 0U;

    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    /* 检查是否启动成功 */
    return (DWT->CYCCNT != 0U) ? 0U : 1U;
}
