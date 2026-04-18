#include "filter.h"

void LPF1_Init(LPF1_t *lpf, float tau_ms, float period_ms)
{
    lpf->alpha = period_ms / (tau_ms + period_ms);
    lpf->out   = 0.0f;
}

float LPF1_Apply(LPF1_t *lpf, float input)
{
    lpf->out = lpf->out * (1.0f - lpf->alpha) + input * lpf->alpha;
    return lpf->out;
}

void LPF1_Reset(LPF1_t *lpf)
{
    lpf->out = 0.0f;
}
