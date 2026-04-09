#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tim.h"
#include "stdint.h"

#define ENCODER_BASE_PPR       11
#define ENCODER_MULTIPLIER     4
#define ENCODER_REDUCTION      30
#define ENCODER_COUNTS_PER_REV (ENCODER_BASE_PPR * ENCODER_MULTIPLIER * ENCODER_REDUCTION)  // 1320

typedef enum {
    ENCODER_ID_1 = 0,
    ENCODER_ID_MAX
} Encoder_IDTypeDef;

void    Encoder_Init(void);
void    Encoder_Reset(Encoder_IDTypeDef id);

/**
 * @brief 读取脉冲增量并累计总数（纯硬件读取，不计算速度）
 * @param id    编码器ID
 * @param diff  输出：本次脉冲增量（带方向）
 * @param total 输出：累计脉冲总数
 */
void    Encoder_Read(Encoder_IDTypeDef id, int16_t *diff, int32_t *total);

#ifdef __cplusplus
}
#endif

#endif
