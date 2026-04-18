#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"
#include "stdint.h"

typedef enum {
    KEY_ID_1 = 0,
    KEY_ID_MAX
} Key_IDTypeDef;

typedef struct
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;
} Key_HandleTypeDef;

void Key_Init(void);
uint8_t Key_Scan(Key_IDTypeDef key_id);

#endif
