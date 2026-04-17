/*********************************************************
 * rtrobot_common.h
 * Copyright (c) 2012 - 2024 RTrobot Inc.
 * Modified for STM32H7 project integration.
 *********************************************************/
#ifndef RTROBOT_COMMON_H
#define RTROBOT_COMMON_H

#include "main.h"

#define READ_WRITE_LEN  UINT8_C(8)

int8_t rtrobot_I2C_ReadCommand (uint8_t reg_addr, uint8_t *rev_data,        uint32_t length, void *intf_ptr);
int8_t rtrobot_I2C_WriteCommand(uint8_t reg_addr, uint8_t const *send_data, uint32_t length, void *intf_ptr);

#endif /* RTROBOT_COMMON_H */
