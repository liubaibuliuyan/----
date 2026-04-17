/*
 * dwt_stm32_delay.c
 * Compatible with STM32F1 and STM32H7.
 *
 * On STM32H7 with CMSIS >= 5.7, CoreDebug_DEMCR_TRCENA_Msk may be
 * replaced by DCB_DEMCR_TRCENA_Msk. The stm32h7xx.h include chain
 * provides a CoreDebug alias so both names compile. If your toolchain
 * reports "CoreDebug undeclared", replace CoreDebug with DCB below.
 */
#include "dwt_stm32_delay.h"

uint32_t DWT_Delay_Init(void)
{
    /* 使能 DWT 追踪（TRCENA 位） */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;

    /* 使能周期计数器 */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;

    /* 清零计数器 */
    DWT->CYCCNT = 0U;

    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    /* 验证计数器已运行 */
    return (DWT->CYCCNT) ? 0U : 1U;
}
