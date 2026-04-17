/*********************************************************
 * rtrobot_bmm350.c
 * Copyright (c) 2012 - 2024 RTrobot Inc.
 * Modified for STM32H7 project integration.
 *   - printf -> USART_Send_String
 *   - DWT delay compatible with H7
 *   - Added rtrobot_bmm350_read() for task polling
 *********************************************************/
#include "rtrobot_bmm350.h"

#include <math.h>

#include "dwt_stm32_delay.h"
#include "main.h"
#include "rtrobot_common.h"
#include "bmm350.h"
#include "myusart.h"    /* 使用项目统一的串口发送接口 */

static uint8_t dev_addr;

/* ------------------------------------------------------------------ */
/* 延时回调（DWT微秒延时，H7上DWT初始化后可用）                         */
/* ------------------------------------------------------------------ */
void bmm350_delay(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    DWT_Delay_us(period);
}

/* ------------------------------------------------------------------ */
/* 错误码打印（printf → USART_Send_String）                            */
/* ------------------------------------------------------------------ */
void bmm350_error_codes_print_result(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BMM350_OK:
            break;
        case BMM350_E_NULL_PTR:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BMM350_E_COM_FAIL:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Communication fail\r\n", api_name, rslt);
            break;
        case BMM350_E_DEV_NOT_FOUND:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BMM350_E_INVALID_CONFIG:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Invalid configuration\r\n", api_name, rslt);
            break;
        case BMM350_E_BAD_PAD_DRIVE:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Bad pad drive\r\n", api_name, rslt);
            break;
        case BMM350_E_RESET_UNFINISHED:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Reset unfinished\r\n", api_name, rslt);
            break;
        case BMM350_E_INVALID_INPUT:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Invalid input\r\n", api_name, rslt);
            break;
        case BMM350_E_SELF_TEST_INVALID_AXIS:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Self-test invalid axis selection\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_BOOT:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP boot\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_PAGE_RD:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP page read\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_PAGE_PRG:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP page prog\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_SIGN:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP sign\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_INV_CMD:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP invalid command\r\n", api_name, rslt);
            break;
        case BMM350_E_OTP_UNDEFINED:
            USART_Send_String(USART_ID_1, "%s Error [%d] : OTP undefined\r\n", api_name, rslt);
            break;
        case BMM350_E_ALL_AXIS_DISABLED:
            USART_Send_String(USART_ID_1, "%s Error [%d] : All axis are disabled\r\n", api_name, rslt);
            break;
        case BMM350_E_PMU_CMD_VALUE:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Unexpected PMU CMD value\r\n", api_name, rslt);
            break;
        default:
            USART_Send_String(USART_ID_1, "%s Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

/* ------------------------------------------------------------------ */
/* 初始化                                                               */
/* ------------------------------------------------------------------ */
void rtrobot_bmm350_init(struct bmm350_dev *dev)
{
    int8_t  rslt = BMM350_OK;
    uint8_t int_ctrl, err_reg_data = 0;
    uint8_t set_int_ctrl;
    struct  bmm350_pmu_cmd_status_0 pmu_cmd_stat_0;

    /* 填充设备结构体 */
    dev_addr        = BMM350_I2C_ADSEL_SET_LOW;
    dev->intf_ptr   = &dev_addr;
    dev->read       = rtrobot_I2C_ReadCommand;
    dev->write      = rtrobot_I2C_WriteCommand;
    dev->delay_us   = bmm350_delay;

    /* 初始化芯片 */
    rslt = bmm350_init(dev);
    bmm350_error_codes_print_result("bmm350_init", rslt);

    USART_Send_String(USART_ID_1, "BMM350 Chip ID : 0x%X\r\n", dev->chip_id);
    USART_Send_String(USART_ID_1, "--- Coefficients ---\r\n");
    USART_Send_String(USART_ID_1, "Offset X:%.2f Y:%.2f Z:%.2f\r\n",
        dev->mag_comp.dut_offset_coef.offset_x,
        dev->mag_comp.dut_offset_coef.offset_y,
        dev->mag_comp.dut_offset_coef.offset_z);
    USART_Send_String(USART_ID_1, "T_offset:%.2f\r\n",
        dev->mag_comp.dut_offset_coef.t_offs);
    USART_Send_String(USART_ID_1, "Sensitivity X:%.2f Y:%.2f Z:%.2f\r\n",
        dev->mag_comp.dut_sensit_coef.sens_x,
        dev->mag_comp.dut_sensit_coef.sens_y,
        dev->mag_comp.dut_sensit_coef.sens_z);
    USART_Send_String(USART_ID_1, "T_sens:%.2f\r\n",
        dev->mag_comp.dut_sensit_coef.t_sens);
    USART_Send_String(USART_ID_1, "TCO X:%.2f Y:%.2f Z:%.2f\r\n",
        dev->mag_comp.dut_tco.tco_x,
        dev->mag_comp.dut_tco.tco_y,
        dev->mag_comp.dut_tco.tco_z);
    USART_Send_String(USART_ID_1, "TCS X:%.4f Y:%.4f Z:%.4f\r\n",
        dev->mag_comp.dut_tcs.tcs_x,
        dev->mag_comp.dut_tcs.tcs_y,
        dev->mag_comp.dut_tcs.tcs_z);
    USART_Send_String(USART_ID_1, "T0:%.2f\r\n", dev->mag_comp.dut_t0);
    USART_Send_String(USART_ID_1, "Cross XY:%f YX:%f ZX:%f ZY:%f\r\n",
        dev->mag_comp.cross_axis.cross_x_y,
        dev->mag_comp.cross_axis.cross_y_x,
        dev->mag_comp.cross_axis.cross_z_x,
        dev->mag_comp.cross_axis.cross_z_y);

    /* 检查PMU busy */
    rslt = bmm350_get_pmu_cmd_status_0(&pmu_cmd_stat_0, dev);
    bmm350_error_codes_print_result("bmm350_get_pmu_cmd_status_0", rslt);
    USART_Send_String(USART_ID_1, "PMU cmd busy : 0x%X (expect 0x0)\r\n",
        pmu_cmd_stat_0.pmu_cmd_busy);

    /* 读错误寄存器 */
    rslt = bmm350_get_regs(BMM350_REG_ERR_REG, &err_reg_data, 1, dev);
    bmm350_error_codes_print_result("bmm350_get_error_reg_data", rslt);
    USART_Send_String(USART_ID_1, "Error Register : 0x%X (expect 0x0)\r\n", err_reg_data);

    /* 配置中断（不映射到引脚，软件轮询状态寄存器） */
    rslt = bmm350_configure_interrupt(BMM350_PULSED,
                                      BMM350_ACTIVE_HIGH,
                                      BMM350_INTR_PUSH_PULL,
                                      BMM350_UNMAP_FROM_PIN,
                                      dev);
    bmm350_error_codes_print_result("bmm350_configure_interrupt", rslt);

    /* 使能数据就绪中断标志 */
    rslt = bmm350_enable_interrupt(BMM350_ENABLE_INTERRUPT, dev);
    bmm350_error_codes_print_result("bmm350_enable_interrupt", rslt);

    /* 读回中断控制寄存器确认 */
    rslt = bmm350_get_regs(BMM350_REG_INT_CTRL, &int_ctrl, 1, dev);
    bmm350_error_codes_print_result("bmm350_get_regs(INT_CTRL)", rslt);
    set_int_ctrl = (uint8_t)((BMM350_INT_POL_ACTIVE_HIGH << 1) |
                              (BMM350_INT_OD_PUSHPULL    << 2) |
                              (BMM350_ENABLE             << 7));
    USART_Send_String(USART_ID_1, "INT_CTRL expect:0x%X  read:0x%X\r\n",
        set_int_ctrl, int_ctrl);
    if (int_ctrl & BMM350_DRDY_DATA_REG_EN_MSK)
        USART_Send_String(USART_ID_1, "Data ready enabled\r\n");

    /* 设置ODR和平均次数：50Hz，4次平均 */
    rslt = bmm350_set_odr_performance(BMM350_DATA_RATE_50HZ, BMM350_AVERAGING_4, dev);
    bmm350_error_codes_print_result("bmm350_set_odr_performance", rslt);

    /* 使能全部轴 */
    rslt = bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, dev);
    bmm350_error_codes_print_result("bmm350_enable_axes", rslt);

    /* 进入正常测量模式 */
    rslt = bmm350_set_powermode(BMM350_NORMAL_MODE, dev);
    bmm350_error_codes_print_result("bmm350_set_powermode", rslt);

    USART_Send_String(USART_ID_1, "BMM350 Init Done\r\n");
}

/* ------------------------------------------------------------------ */
/* 读取补偿后磁场数据（有新数据时打印，供调试用）                        */
/* ------------------------------------------------------------------ */
void rtrobot_bmm350_test_compensated_magnetometer(struct bmm350_dev *dev)
{
    uint8_t int_status = 0;
    int8_t  rslt;
    struct  bmm350_mag_temp_data mag_temp_data;

    rslt = bmm350_get_regs(BMM350_REG_INT_STATUS, &int_status, 1, dev);
    bmm350_error_codes_print_result("bmm350_get_regs", rslt);

    if (int_status & BMM350_DRDY_DATA_REG_MSK)
    {
        rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_temp_data, dev);
        bmm350_error_codes_print_result("bmm350_get_compensated_mag_xyz_temp_data", rslt);

        /* 串口绘图仪格式（Serial Studio / 串口助手波形显示） */
        USART_Send_String(USART_ID_1, "$0:%.2f;$1:%.2f;$2:%.2f;\r\n",
            mag_temp_data.x, mag_temp_data.y, mag_temp_data.z);
    }
}

/* ------------------------------------------------------------------ */
/* 读取原始数据（供调试用）                                             */
/* ------------------------------------------------------------------ */
void rtrobot_bmm350_test_raw(struct bmm350_dev *dev)
{
    uint8_t int_status = 0;
    int8_t  rslt;
    struct  bmm350_raw_mag_data raw_data;

    rslt = bmm350_get_regs(BMM350_REG_INT_STATUS, &int_status, 1, dev);
    bmm350_error_codes_print_result("bmm350_get_regs", rslt);

    if (int_status & BMM350_DRDY_DATA_REG_MSK)
    {
        rslt = bmm350_read_uncomp_mag_temp_data(&raw_data, dev);
        bmm350_error_codes_print_result("bmm350_read_uncomp_mag_data", rslt);

        uint32_t secs = 0, nano_secs = 0;
        rslt = bmm350_read_sensortime(&secs, &nano_secs, dev);
        bmm350_error_codes_print_result("bmm350_read_sensortime", rslt);

        USART_Send_String(USART_ID_1, "RAW X:%ld Y:%ld Z:%ld T:%ld  time:%lu.%09lu\r\n",
            (long)raw_data.raw_xdata,
            (long)raw_data.raw_ydata,
            (long)raw_data.raw_zdata,
            (long)raw_data.raw_data_t,
            (unsigned long)secs,
            (unsigned long)nano_secs);
    }
}

/* ------------------------------------------------------------------ */
/* 带返回值的读取接口，供start.c的Task_BMM350()调用                     */
/* 返回值：1=有新数据并已填入out，0=无新数据                            */
/* ------------------------------------------------------------------ */
uint8_t rtrobot_bmm350_read(struct bmm350_dev *dev, BMM350_Data_t *out)
{
    uint8_t int_status = 0;
    int8_t  rslt;
    struct  bmm350_mag_temp_data mag_temp_data;

    out->valid = 0;

    rslt = bmm350_get_regs(BMM350_REG_INT_STATUS, &int_status, 1, dev);
    if (rslt != BMM350_OK) return 0;

    if (int_status & BMM350_DRDY_DATA_REG_MSK)
    {
        rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_temp_data, dev);
        if (rslt != BMM350_OK) return 0;

        out->x           = mag_temp_data.x;
        out->y           = mag_temp_data.y;
        out->z           = mag_temp_data.z;
        out->temperature = mag_temp_data.temperature;
        out->valid       = 1;
        return 1;
    }
    return 0;
}
