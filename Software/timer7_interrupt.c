#include "timer7_interrupt.h"

/*==========================================================
 * 配置区
 *==========================================================*/
#define CTRL_PERIOD_MS     1
#define SPEED_TAU_MS       15.0f
#define MT_THRESHOLD       0
#define CTRL_PERIOD_S      ((float)CTRL_PERIOD_MS / 1000.0f)

// ===================== 前馈系数 Kf =====================

#define FEEDFORWARD_Kf     2350.0f

/*==========================================================
 * 电机控制全局变量
 *==========================================================*/
volatile float    g_ctrl_speed_meas   = 0.0f;
volatile float    g_ctrl_speed_target = 0.0f;
volatile int32_t  g_ctrl_total_pulse  = 0;
volatile uint8_t  g_ctrl_enable       = 0;

static uint32_t   s_tick_100us = 0;
static int32_t    s_m_pulse    = 0;
static uint32_t   s_ms_tick    = 0;

static uint32_t   s_t_last_ms  = 0;
static uint32_t   s_t_this_ms  = 0;
static int8_t     s_t_valid    = 0;
static int16_t    s_last_diff  = 0;
static float      s_speed_filtered = 0.0f;
static float      s_alpha;
static PID_t      s_pid;

/*==========================================================
 * 对外接口
 *==========================================================*/
void Ctrl_Set_Target(float target_rps)
{
    g_ctrl_speed_target = target_rps;
    PID_Set_Target(&s_pid, target_rps);
}

void Ctrl_Enable(void)
{
    PID_Reset(&s_pid);
    s_speed_filtered = 0.0f;
    s_m_pulse = 0;
    s_ms_tick = 0;
    s_t_valid = 0;
    g_ctrl_enable = 1;
}

void Ctrl_Disable(void)
{
    g_ctrl_enable = 0;
    PID_Reset(&s_pid);
    Motor_Control(MOTOR_ID_1, MOTOR_STOP, 0);
}

/*==========================================================
 * TIM7 100μs 主运行函数
 *==========================================================*/
void TIM7_Run(void)
{
    int16_t diff = 0;
    int32_t total = 0;
    Encoder_Read(ENCODER_ID_1, &diff, &total);
    g_ctrl_total_pulse = total;

    s_m_pulse += diff;

    if (diff != 0)
    {
        s_t_last_ms = s_t_this_ms;
        s_t_this_ms = s_ms_tick;
        s_last_diff = diff;
        s_t_valid   = 1;
    }

    s_tick_100us++;
    if (s_tick_100us >= 10)
    {
        s_tick_100us = 0;
        s_ms_tick++;

        int32_t pulse = s_m_pulse;
        s_m_pulse = 0;

        float raw_speed = 0.0f;

        if (pulse >= MT_THRESHOLD || pulse <= -MT_THRESHOLD)
        {
            raw_speed = (float)pulse / ENCODER_COUNTS_PER_REV / CTRL_PERIOD_S;
        }
        else if (s_t_valid)
        {
            float sign = (s_last_diff > 0) ? 1.0f : -1.0f;
            uint32_t elapsed = s_ms_tick - s_t_this_ms;

            if (elapsed > 1000)
            {
                raw_speed = 0.0f;
                s_t_valid = 0;
            }
            else
            {
                float t_s = (float)elapsed / 1000.0f;
                raw_speed = sign / ENCODER_COUNTS_PER_REV / t_s;
            }
        }
        else
        {
            raw_speed = 0.0f;
        }

        s_speed_filtered = s_speed_filtered * (1.0f - s_alpha) + raw_speed * s_alpha;
        g_ctrl_speed_meas = s_speed_filtered;

        if (g_ctrl_enable)
        {
            // 1. PID 计算（误差修正）
            float pid_out = PID_Calc(&s_pid, g_ctrl_speed_meas);

            // 2. 速度前馈计算（预判出力）
            float feedforward_out = g_ctrl_speed_target * FEEDFORWARD_Kf;

            // 3. 最终输出 = PID + 前馈
            float final_out = pid_out + feedforward_out;

            // 4. 输出限幅（保持原有范围）
            if(final_out > 14999.0f)  final_out = 14999.0f;
            if(final_out < -14999.0f) final_out = -14999.0f;

            // 5. 电机控制（不变）
            Motor_DirTypeDef dir;
            uint16_t pwm;

            if (final_out > 0)
            {
                dir = MOTOR_FORWARD;
                pwm = (uint16_t)final_out;
            }
            else if (final_out < 0)
            {
                dir = MOTOR_BACKWARD;
                pwm = (uint16_t)(-final_out);
            }
            else
            {
                dir = MOTOR_STOP;
                pwm = 0;
            }

            Motor_SetDir(MOTOR_ID_1, dir);
            Motor_SetSpeed(MOTOR_ID_1, pwm);
        }
    }
}

/*==========================================================
 * 初始化
 *==========================================================*/
void Timer7_Interrupt_Init(void)
{
    s_alpha = (float)CTRL_PERIOD_MS / (SPEED_TAU_MS + (float)CTRL_PERIOD_MS);

    PID_Init(&s_pid,
             180.0f,
             0.05f,
             0.0f,
             CTRL_PERIOD_S,
             -14999.0f,
             14999.0f);

    s_tick_100us = 0;
    s_speed_filtered = 0.0f;
    s_ms_tick = 0;

    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
        Error_Handler();
}

void Timer7_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim7);
}
