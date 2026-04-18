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
    ENCODER_ID_2,
    ENCODER_ID_3,
    ENCODER_ID_4,
    ENCODER_ID_MAX
} Encoder_IDTypeDef;

void    Encoder_Init(void);
void    Encoder_Reset(Encoder_IDTypeDef id);
void    Encoder_Read(Encoder_IDTypeDef id, int16_t *diff, int32_t *total);

#ifdef __cplusplus
}
#endif

#endif
