/*********************************************************
 * rtrobot_bmm350.h
 * Copyright (c) 2012 - 2024 RTrobot Inc.
 * Modified for STM32H7 project integration.
 *********************************************************/
#ifndef RTROBOT_BMM350_H
#define RTROBOT_BMM350_H

#include "main.h"          /* 去掉 ../ 相对路径，适配H7工程目录结构 */
#include "bmm350_defs.h"

/* 磁力计数据结构，供外部模块使用 */
typedef struct {
    float x;           /* uT */
    float y;           /* uT */
    float z;           /* uT */
    float temperature; /* degC */
    uint8_t valid;     /* 1=本次有新数据，0=无新数据 */
} BMM350_Data_t;

void    rtrobot_bmm350_init(struct bmm350_dev *dev);
void    rtrobot_bmm350_test_raw(struct bmm350_dev *dev);
void    rtrobot_bmm350_test_compensated_magnetometer(struct bmm350_dev *dev);

/* 带返回值的读取接口，供start.c的Task使用 */
uint8_t rtrobot_bmm350_read(struct bmm350_dev *dev, BMM350_Data_t *out);

#endif /* RTROBOT_BMM350_H */
