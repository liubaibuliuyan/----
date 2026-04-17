/*********************************************************
 * rtrobot_common.c
 * Copyright (c) 2012 - 2024 RTrobot Inc.
 * Modified for STM32H7 project integration.
 *   - hi2c1 与H7 i2c.c中定义的句柄名称一致，无需修改
 *   - HAL_I2C_Mem_Read/Write 接口H7与F1相同，无需修改
 *********************************************************/
#include "rtrobot_common.h"
#include "i2c.h"   /* H7工程中 MX_I2C1_Init 生成的头文件 */

/* hi2c1 在 i2c.c 中定义，此处直接引用 */
extern I2C_HandleTypeDef hi2c1;

/* ------------------------------------------------------------------ */
/* I2C 读                                                               */
/* ------------------------------------------------------------------ */
int8_t rtrobot_I2C_ReadCommand(uint8_t reg_addr, uint8_t *rev_data,
                                uint32_t length, void *intf_ptr)
{
    uint8_t device_addr = *(uint8_t *)intf_ptr;
    (void)intf_ptr;
    return (int8_t)HAL_I2C_Mem_Read(&hi2c1,
                                     (uint16_t)(device_addr << 1),
                                     reg_addr, I2C_MEMADD_SIZE_8BIT,
                                     rev_data, (uint16_t)length, 1000);
}

/* ------------------------------------------------------------------ */
/* I2C 写                                                               */
/* ------------------------------------------------------------------ */
int8_t rtrobot_I2C_WriteCommand(uint8_t reg_addr, uint8_t const *send_data,
                                 uint32_t length, void *intf_ptr)
{
    uint8_t device_addr = *(uint8_t *)intf_ptr;
    (void)intf_ptr;
    return (int8_t)HAL_I2C_Mem_Write(&hi2c1,
                                      (uint16_t)(device_addr << 1),
                                      reg_addr, I2C_MEMADD_SIZE_8BIT,
                                      (uint8_t *)send_data, (uint16_t)length, 1000);
}
