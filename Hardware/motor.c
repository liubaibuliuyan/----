/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor.c
  * @brief   电机（TB6612驱动）控制相关函数实现
  ******************************************************************************
  */
/* USER CODE END Header */

#include "motor.h"

/********************* 电机通用句柄结构体（无需修改） *********************/
typedef struct {
    // 仅需配置的硬件参数（新增电机只改这里）
    GPIO_TypeDef       *ain1_port;      // Ain1引脚端口
    uint16_t           ain1_pin;       // Ain1引脚
    GPIO_TypeDef       *ain2_port;      // Ain2引脚端口
    uint16_t           ain2_pin;       // Ain2引脚
    TIM_HandleTypeDef  *pwm_tim;       // PWM定时器句柄
    uint32_t           pwm_channel;    // PWM通道
    uint16_t           pwm_max;        // PWM最大值（对应Period）
    
    // 运行状态（自动初始化，无需配置）
    Motor_DirTypeDef   current_dir;    // 当前方向
    uint16_t           current_speed;  // 当前转速
} Motor_HandleTypeDef;

/********************* 电机配置表：唯一需要修改的地方（懒人核心） *********************/
// 新增电机：仅在下面加一行硬件配置即可，其余全自动
static Motor_HandleTypeDef motor_list[] = {
    // 电机1：仅配置硬件参数，状态自动初始化
    {
        .ain1_port = GPIOC,
        .ain1_pin = GPIO_PIN_5,
        .ain2_port = GPIOC,
        .ain2_pin = GPIO_PIN_4,
        .pwm_tim = &htim2,
        .pwm_channel = TIM_CHANNEL_1,
        .pwm_max = 14999
    },
    // 电机2：新增仅加这一行！
    /*
    {
        .ain1_port = GPIOB,
        .ain1_pin = GPIO_PIN_0,
        .ain2_port = GPIOB,
        .ain2_pin = GPIO_PIN_1,
        .pwm_tim = &htim3,
        .pwm_channel = TIM_CHANNEL_2,
        .pwm_max = 9999
    },
    */
};

// 自动计算电机总数（适配配置表，无需手动改MOTOR_ID_MAX）
#define MOTOR_ID_MAX (sizeof(motor_list) / sizeof(Motor_HandleTypeDef))

/********************* 私有函数声明（永不修改） *********************/
static uint8_t _Motor_CheckIDValid(Motor_IDTypeDef motor_id);
static Motor_HandleTypeDef* _Motor_GetHandle(Motor_IDTypeDef motor_id);

/********************* 私有函数实现（永不修改） *********************/
static uint8_t _Motor_CheckIDValid(Motor_IDTypeDef motor_id)
{
    return (motor_id < MOTOR_ID_MAX && motor_list[motor_id].pwm_tim != NULL) ? 1 : 0;
}

static Motor_HandleTypeDef* _Motor_GetHandle(Motor_IDTypeDef motor_id)
{
    return _Motor_CheckIDValid(motor_id) ? &motor_list[motor_id] : NULL;
}

/********************* 通用函数实现（永不修改） *********************/
/**
  * @brief  电机全局初始化函数
  * @note   自动遍历所有配置的电机，启动PWM输出，默认停止
  * @param  无
  * @retval 无
  */
void Motor_Init(void)
{
    // 自动遍历初始化所有配置的电机
    for (Motor_IDTypeDef i = MOTOR_ID_1; i < MOTOR_ID_MAX; i++)
    {
        Motor_HandleTypeDef *hmotor = _Motor_GetHandle(i);
        if (!hmotor) continue;

        // 初始化运行状态
        hmotor->current_dir = MOTOR_STOP;
        hmotor->current_speed = 0;

        // 启动PWM输出
        HAL_TIM_PWM_Start(hmotor->pwm_tim, hmotor->pwm_channel);
        
        // 默认停止电机
        Motor_SetDir(i, MOTOR_STOP);
        Motor_SetSpeed(i, 0);
    }
}

/**
  * @brief  设置指定电机的转向
  * @param  motor_id: 电机ID（MOTOR_ID_1/MOTOR_ID_2...）
  * @param  dir: 电机方向（MOTOR_STOP/MOTOR_FORWARD/MOTOR_BACKWARD）
  * @retval 无
  */
void Motor_SetDir(Motor_IDTypeDef motor_id, Motor_DirTypeDef dir)
{
    Motor_HandleTypeDef *hmotor = _Motor_GetHandle(motor_id);
    if (!hmotor) return;

    switch(dir)
    {
        case MOTOR_STOP:
            // 停止：Ain1和Ain2都置低
            HAL_GPIO_WritePin(hmotor->ain1_port, hmotor->ain1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(hmotor->ain2_port, hmotor->ain2_pin, GPIO_PIN_RESET);
            hmotor->current_dir = MOTOR_STOP;
            break;
            
        case MOTOR_FORWARD:
            // 正转：Ain1置高，Ain2置低
            HAL_GPIO_WritePin(hmotor->ain1_port, hmotor->ain1_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(hmotor->ain2_port, hmotor->ain2_pin, GPIO_PIN_RESET);
            hmotor->current_dir = MOTOR_FORWARD;
            break;
            
        case MOTOR_BACKWARD:
            // 反转：Ain1置低，Ain2置高
            HAL_GPIO_WritePin(hmotor->ain1_port, hmotor->ain1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(hmotor->ain2_port, hmotor->ain2_pin, GPIO_PIN_SET);
            hmotor->current_dir = MOTOR_BACKWARD;
            break;
            
        default:
            // 非法参数默认停止
            Motor_SetDir(motor_id, MOTOR_STOP);
            break;
    }
}

/**
  * @brief  设置指定电机的转速（通过PWM占空比控制）
  * @param  motor_id: 电机ID（MOTOR_ID_1/MOTOR_ID_2...）
  * @param  speed: PWM脉冲值（0~对应电机的pwm_max）
  * @retval 无
  */
void Motor_SetSpeed(Motor_IDTypeDef motor_id, uint16_t speed)
{
    Motor_HandleTypeDef *hmotor = _Motor_GetHandle(motor_id);
    if (!hmotor) return;

    // 限制转速范围，防止超出PWM最大值
    if(speed > hmotor->pwm_max)
    {
        speed = hmotor->pwm_max;
    }
    
    // 设置PWM脉冲值
    __HAL_TIM_SET_COMPARE(hmotor->pwm_tim, hmotor->pwm_channel, speed);
    hmotor->current_speed = speed;
}

/**
  * @brief  综合控制指定电机：同时设置方向和转速
  * @param  motor_id: 电机ID（MOTOR_ID_1/MOTOR_ID_2...）
  * @param  dir: 电机方向
  * @param  speed: 电机转速（0~对应电机的pwm_max）
  * @retval 无
  */
void Motor_Control(Motor_IDTypeDef motor_id, Motor_DirTypeDef dir, uint16_t speed)
{
    Motor_SetDir(motor_id, dir);
    Motor_SetSpeed(motor_id, speed);
}
