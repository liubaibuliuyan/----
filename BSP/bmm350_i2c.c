#include "bmm350_i2c.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;

int8_t bmm350_i2c_read(uint8_t reg_addr, uint8_t *rev_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    return (int8_t)HAL_I2C_Mem_Read(
        &hi2c1,
        (uint16_t)(dev_addr << 1),
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        rev_data,
        (uint16_t)len,
        1000
    );
}

int8_t bmm350_i2c_write(uint8_t reg_addr, const uint8_t *send_data, uint32_t len, void *intf_ptr)
{
    uint8_t dev_addr = *(uint8_t *)intf_ptr;

    return (int8_t)HAL_I2C_Mem_Write(
        &hi2c1,
        (uint16_t)(dev_addr << 1),
        reg_addr,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)send_data,
        (uint16_t)len,
        1000
    );
}
