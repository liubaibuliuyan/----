#include "timer6_interrupt.h"
#include "encoder.h"
#include "motor.h"
#include "pid.h"

/*==========================================================
 * 【配置区】 All in One
 *==========================================================*/
#define CTRL_PERIOD_MS   1              // 控制周期 (ms)，改这里即可
#define SPEED_TAU_MS     30.0f          // 速度滤波时间常数 (ms)
#define MT_THRESHOLD     0               // M/T法切换阈值 (脉冲数)

/*==========================================================
 * 内部变量
 *==========================================================*/
#define CTRL_PERIOD_S    ((float)CTRL_PERIOD_MS / 1000.0f)

volatile uint32_t g_tim6_1ms_cnt = 0;

// TIM7 100us 计数（临时放在这里）
volatile uint32_t g_tim7_100us_cnt = 0;

/* 对外状态 */
volatile float   g_ctrl_speed_meas   = 0.0f;
volatile float   g_ctrl_speed_target = 0.0f;
volatile int32_t g_ctrl_total_pulse  = 0;
volatile uint8_t g_ctrl_enable       = 0;

/* M/T法测速变量 */
static uint32_t s_ms_tick          = 0;
static int32_t  s_m_pulse          = 0;
static uint32_t s_t_last_ms        = 0;
static uint32_t s_t_this_ms        = 0;
static int8_t   s_t_valid          = 0;
static int16_t  s_last_diff        = 0;
static float    s_speed_filtered   = 0.0f;

/* 【优化】预计算的滤波系数 */
static float    s_alpha            = 0.0f;

/* PID实例 */
static PID_t s_pid;

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
    s_ms_tick        = 0;
    s_m_pulse        = 0;
    s_t_valid        = 0;
    g_ctrl_enable    = 1;
}

void Ctrl_Disable(void)
{
    g_ctrl_enable = 0;
    PID_Reset(&s_pid);
    Motor_Control(MOTOR_ID_1, MOTOR_STOP, 0);
}

/*==========================================================
 * M/T法测速 (完整版)
 *==========================================================*/
static uint8_t MT_Update(int16_t diff)
{
    /* T法：记录脉冲时刻 */
    if (diff != 0)
    {
        s_t_last_ms = s_t_this_ms;
        s_t_this_ms = g_tim6_1ms_cnt;
        s_last_diff = diff;
        s_t_valid   = 1;
    }

    /* M法：累计脉冲 */
    s_m_pulse += diff;
    s_ms_tick++;

    if (s_ms_tick < CTRL_PERIOD_MS) return 0;

    /* ── 窗口到期：计算速度 ── */
    float raw_speed = 0.0f;

    if (s_m_pulse >= MT_THRESHOLD || s_m_pulse <= -MT_THRESHOLD)
    {
        /* M法：高速 */
        raw_speed = (float)s_m_pulse
                  / (float)ENCODER_COUNTS_PER_REV
                  / CTRL_PERIOD_S;
    }
    else if (s_t_valid)
    {
        float sign = (s_last_diff > 0) ? 1.0f : -1.0f;

        if (s_m_pulse == 0)
        {
            /* T法外推：窗口内无脉冲 */
            uint32_t elapsed = g_tim6_1ms_cnt - s_t_this_ms;
            if (elapsed > 1000)
            {
                raw_speed  = 0.0f;
                s_t_valid  = 0;
            }
            else
            {
                float t_s = (float)elapsed / 1000.0f;
                if (t_s > 0.0f)
                    raw_speed = sign / (float)ENCODER_COUNTS_PER_REV / t_s;
            }
        }
        else
        {
            /* T法：用相邻脉冲间隔 */
            uint32_t t_diff = (s_t_this_ms > s_t_last_ms)
                            ? (s_t_this_ms - s_t_last_ms)
                            : 1;
            float t_s = (float)t_diff / 1000.0f;
            if (t_s > 0.0f)
                raw_speed = sign / (float)ENCODER_COUNTS_PER_REV / t_s;
        }
    }
    else
    {
        raw_speed = 0.0f;
    }

    /* --- 【优化】使用预计算的 s_alpha 进行滤波 --- */
    s_speed_filtered = s_speed_filtered * (1.0f - s_alpha) + raw_speed * s_alpha;
    g_ctrl_speed_meas = s_speed_filtered;

    /* 重置窗口 */
    s_ms_tick = 0;
    s_m_pulse = 0;

    return 1;
}

/*==========================================================
 * 【全局唯一】定时器中断回调
 * 包含 TIM6 + TIM7
 *==========================================================*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // ==========================
    // TIM6：1ms 系统时钟 + 电机控制
    // ==========================
    if (htim->Instance == TIM6)
    {
        g_tim6_1ms_cnt++;

        /* 1. 读编码器 */
        int16_t diff  = 0;
        int32_t total = 0;
        Encoder_Read(ENCODER_ID_1, &diff, &total);
        g_ctrl_total_pulse = total;

        /* 2. 测速更新 */
        uint8_t speed_ready = MT_Update(diff);

        /* 3. 闭环控制 */
        if (speed_ready)
        {
            if (g_ctrl_enable)
            {
                float pwm_output = PID_Calc(&s_pid, g_ctrl_speed_meas);
                uint16_t pwm_abs;
                Motor_DirTypeDef dir;

                if(pwm_output > 0)
                {
                    dir = MOTOR_FORWARD;
                    pwm_abs = (uint16_t)pwm_output;
                }
                else if(pwm_output < 0)
                {
                    dir = MOTOR_BACKWARD;
                    pwm_abs = (uint16_t)(-pwm_output);
                }
                else
                {
                    dir = MOTOR_STOP;
                    pwm_abs = 0;
                }

                Motor_SetDir(MOTOR_ID_1, dir);
                Motor_SetSpeed(MOTOR_ID_1, pwm_abs);
            }
        }
    }

    // ==========================
    // TIM7：100us 基础计数
    // ==========================
    else if (htim->Instance == TIM7)
    {
        g_tim7_100us_cnt++;
    }
}

/*==========================================================
 * 系统初始化
 *==========================================================*/
void Timer6_Interrupt_Init(void)
{
    PID_Init(&s_pid,
             280.0f,
             0.035f,
             0.0f,
             CTRL_PERIOD_S,
             -14999.0f,
             14999.0f);

    s_alpha = (float)CTRL_PERIOD_MS / (SPEED_TAU_MS + (float)CTRL_PERIOD_MS);
    if (s_alpha > 1.0f) s_alpha = 1.0f;
    if (s_alpha < 0.0f) s_alpha = 0.0f;

    g_tim6_1ms_cnt = 0;

    if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
        Error_Handler();
}

void Timer6_Interrupt_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim6);
}

void Timer6_Interrupt_Restart(void)
{
    Timer6_Interrupt_Stop();
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    g_tim6_1ms_cnt = 0;
    Timer6_Interrupt_Init();
}
