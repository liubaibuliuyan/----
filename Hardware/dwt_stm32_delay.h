/*
 * dwt_stm32_delay.h
 * DWT microsecond delay, compatible with STM32F1 / H7.
 * On H7 with new CMSIS (>= 5.7), CoreDebug is aliased to DCB.
 * The HAL include chain handles this transparently.
 */
#ifndef DWT_STM32_DELAY_H
#define DWT_STM32_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief  初始化 DWT 周期计数器
 * @return 0=成功，1=计数器未启动
 */
uint32_t DWT_Delay_Init(void);

/**
 * @brief  微秒级忙等延时（需先调用 DWT_Delay_Init）
 * @param  microseconds  延时微秒数
 */
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
    uint32_t clk_cycle_start = DWT->CYCCNT;
    microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000U);
    while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

#ifdef __cplusplus
}
#endif

#endif /* DWT_STM32_DELAY_H */
