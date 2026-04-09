#include "key.h"
#include "main.h"

static Key_HandleTypeDef key_list[] =
{
    {GPIOA, GPIO_PIN_4},  // KEY1
};

#define KEY_ID_MAX (sizeof(key_list) / sizeof(Key_HandleTypeDef))

void Key_Init(void)
{
    // GPIO已在MX_GPIO_Init初始化，无需额外操作
}

uint8_t Key_Scan(Key_IDTypeDef key_id)
{
    if (key_id >= KEY_ID_MAX)
        return 0;

    GPIO_TypeDef *port = key_list[key_id].gpio_port;
    uint16_t pin = key_list[key_id].gpio_pin;

    // 检测按下
    if (HAL_GPIO_ReadPin(port, pin) == 0)
    {
        HAL_Delay(20);  // 延时消抖 20ms

        // 再次确认按下
        if (HAL_GPIO_ReadPin(port, pin) == 0)
        {
            // 等待松手，防止重复触发
            while (HAL_GPIO_ReadPin(port, pin) == 0);
            return 1;
        }
    }
    return 0;
}
