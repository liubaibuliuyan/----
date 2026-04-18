/*
 * start.h
 * Modified: added BMM350 includes.
 */
#ifndef __START_H
#define __START_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "myusart.h"
#include "motor.h"       /* 保留你原有的头文件 */
#include "encoder.h"
#include "key.h"
#include "lcd.h"
#include "ctrl_mode.h"
#include "protocol.h"
#include "timer6_interrupt.h"
#include "timer7_interrupt.h"

/* ---- BMM350 新增 ---- */
#include "bmm350_port.h"
#include "dwt_delay.h"
/* --------------------- */

extern volatile uint32_t g_tim6_1ms_cnt;

void Start_Init(void);
void Start_MainLoop(void);

#ifdef __cplusplus
}
#endif

#endif /* __START_H */
