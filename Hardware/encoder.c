#include "encoder.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    int32_t  last_cnt;    // 改为32位，兼容TIM5
    int32_t  total_cnt;
} Encoder_t;

// 4路编码器的TIM1/TIM3/TIM4/TIM5
static Encoder_t encoder_list[] = {
    { .htim = &htim1 },   // 编码1
    { .htim = &htim3 },   // 编码2
    { .htim = &htim4 },   // 编码3
    { .htim = &htim5 },   // 编码4
};

#define ENCODER_NUM  ENCODER_ID_MAX

void Encoder_Init(void)
{
    for (int i = 0; i < ENCODER_NUM; i++)
    {
        encoder_list[i].last_cnt = 0;
        encoder_list[i].total_cnt = 0;
        HAL_TIM_Encoder_Start(encoder_list[i].htim, TIM_CHANNEL_ALL);
    }
}

void Encoder_Reset(Encoder_IDTypeDef id)
{
    if ((uint32_t)id >= ENCODER_NUM) return;

    Encoder_t *enc = &encoder_list[id];
    __HAL_TIM_SET_COUNTER(enc->htim, 0);
    enc->last_cnt = 0;
    enc->total_cnt = 0;
}

void Encoder_Read(Encoder_IDTypeDef id, int16_t *diff, int32_t *total)
{
    if ((uint32_t)id >= ENCODER_NUM) return;

    Encoder_t *enc = &encoder_list[id];

    int32_t now = (int32_t)__HAL_TIM_GET_COUNTER(enc->htim);
    int32_t d = now - enc->last_cnt;
    enc->last_cnt = now;
    enc->total_cnt += d;

    if (diff)  *diff  = (int16_t)d;
    if (total) *total = enc->total_cnt;
}
