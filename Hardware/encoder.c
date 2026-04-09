#include "encoder.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    int16_t  last_cnt;
    int32_t  total_cnt;
} Encoder_t;

static Encoder_t encoder_list[] = {
    { .htim = &htim1 },
};

#define ENCODER_NUM (sizeof(encoder_list) / sizeof(Encoder_t))

void Encoder_Init(void)
{
    for (int i = 0; i < ENCODER_NUM; i++)
    {
        encoder_list[i].last_cnt  = 0;
        encoder_list[i].total_cnt = 0;
        HAL_TIM_Encoder_Start(encoder_list[i].htim, TIM_CHANNEL_ALL);
    }
}

void Encoder_Reset(Encoder_IDTypeDef id)
{
    if (id >= ENCODER_NUM) return;
    Encoder_t *enc = &encoder_list[id];

    __HAL_TIM_SET_COUNTER(enc->htim, 0);
    enc->last_cnt  = 0;
    enc->total_cnt = 0;
}

void Encoder_Read(Encoder_IDTypeDef id, int16_t *diff, int32_t *total)
{
    if (id >= ENCODER_NUM) return;
    Encoder_t *enc = &encoder_list[id];

    int16_t now       = (int16_t)(__HAL_TIM_GET_COUNTER(enc->htim));
    int16_t d         = (int16_t)(now - enc->last_cnt);  // 自动处理溢出
    enc->last_cnt     = now;
    enc->total_cnt   += d;

    if (diff)  *diff  = d;
    if (total) *total = enc->total_cnt;
}
