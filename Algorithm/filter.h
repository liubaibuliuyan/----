#ifndef __FILTER_H
#define __FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// 一阶低通滤波器结构体
typedef struct {
    float alpha;      // 滤波系数
    float out;        // 滤波输出
} LPF1_t;

// 初始化一阶低通滤波器
// tau_ms: 滤波时间常数(ms)
// period_ms: 数据更新周期(ms)
void LPF1_Init(LPF1_t *lpf, float tau_ms, float period_ms);

// 滤波计算：输入原始值，返回滤波后值
float LPF1_Apply(LPF1_t *lpf, float input);

// 重置滤波器
void LPF1_Reset(LPF1_t *lpf);

#ifdef __cplusplus
}
#endif

#endif
