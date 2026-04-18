#ifndef BMM350_I2C_H
#define BMM350_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

int8_t bmm350_i2c_read(uint8_t reg_addr, uint8_t *rev_data, uint32_t len, void *intf_ptr);
int8_t bmm350_i2c_write(uint8_t reg_addr, const uint8_t *send_data, uint32_t len, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif
