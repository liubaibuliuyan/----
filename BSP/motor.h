/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor.h
  * @brief   电机（TB6612驱动）控制相关函数声明
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"
#include "tim.h"

/********************* 无需手动改枚举！自动适配配置表 *********************/
// 电机ID枚举：固定结构，新增电机无需修改
typedef enum {
    MOTOR_ID_1 = 0,    // 固定起始，新增自动往后排
    MOTOR_ID_MAX       // 自动等于配置表行数，无需手动改
} Motor_IDTypeDef;

// 电机方向枚举（通用化，所有电机共用）
typedef enum {
    MOTOR_STOP = 0,    // 停止
    MOTOR_FORWARD,     // 正转
    MOTOR_BACKWARD     // 反转
} Motor_DirTypeDef;

/********************* 极简接口声明（通用化，永不修改） *********************/
void Motor_Init(void);                                      // 电机全局初始化（自动遍历所有电机）
void Motor_SetDir(Motor_IDTypeDef motor_id, Motor_DirTypeDef dir); // 设置电机方向
void Motor_SetSpeed(Motor_IDTypeDef motor_id, uint16_t speed);      // 设置电机转速
void Motor_Control(Motor_IDTypeDef motor_id, Motor_DirTypeDef dir, uint16_t speed); // 综合控制

#endif /* __MOTOR_H */
