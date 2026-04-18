#ifndef DWT_DELAY_H
#define DWT_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

uint32_t DWT_Init(void);

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (HAL_RCC_GetHCLKFreq() / 1000000U);
    while ((DWT->CYCCNT - start) < cycles);
}

#ifdef __cplusplus
}
#endif

#endif
