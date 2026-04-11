#ifndef __START_H
#define __START_H

#include "main.h"
#include "myusart.h"
#include "key.h"
#include "lcd.h"
#include <stdio.h>
#include "encoder.h"
#include "timer6_interrupt.h"
#include "timer7_interrupt.h"
#include "motor.h"
#include "protocol.h"
#include "speed_loop.h"  
#include "pos_loop.h"
#include "ctrl_mode.h"

void Start_Init(void);
void Start_MainLoop(void);

#endif
