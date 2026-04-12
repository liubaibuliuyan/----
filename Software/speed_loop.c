#include "speed_loop.h"

volatile float    g_ctrl_speed_meas   = 0.0f;
volatile float    g_ctrl_speed_target = 0.0f;
volatile int32_t  g_ctrl_total_pulse  = 0;
volatile uint8_t  g_ctrl_enable       = 0;

static float      s_speed_filtered = 0.0f;
static float      s_alpha;
static PID_t      s_pid;

void SpeedLoop_Init(void)
{
    // 一阶低通滤波系数计算，和1ms周期匹配
    s_alpha = (float)CTRL_PERIOD_MS / (SPEED_TAU_MS + (float)CTRL_PERIOD_MS);

    // 速度环PID初始化
    PID_Init(&s_pid,
             160.0f, 0.03f, 0.007f,
             CTRL_PERIOD_S,
             -14999.0f, 14999.0f);

    s_speed_filtered = 0.0f;
}

void Ctrl_Enable(void)
{
    PID_Reset(&s_pid);
    s_speed_filtered = 0.0f;
    g_ctrl_enable    = 1;
}

void Ctrl_Disable(void)
{
    g_ctrl_enable       = 0;
    g_ctrl_speed_target = 0.0f;
    PID_Reset(&s_pid);
    Motor_Control(MOTOR_ID_1, MOTOR_STOP, 0);
}

void Ctrl_Set_Target(float target_rps)
{
    g_ctrl_speed_target = target_rps;
    PID_Set_Target(&s_pid, target_rps);
}

// 入参diff_pulse_1ms是1ms内累计的总脉冲，和计算周期严格匹配
void SpeedLoop_Process(int32_t diff_pulse_1ms, int32_t total_pulse)
{
    g_ctrl_total_pulse = total_pulse;

    // 速度计算：1ms总脉冲 / 编码器单圈脉冲数 / 1ms周期 = 转/秒(rps)
    float raw_speed = (float)diff_pulse_1ms / ENCODER_COUNTS_PER_REV / CTRL_PERIOD_S;

    // 一阶低通滤波
    s_speed_filtered  = s_speed_filtered * (1.0f - s_alpha) + raw_speed * s_alpha;
    g_ctrl_speed_meas = s_speed_filtered;

    // 速度环PID计算+电机输出
    if (g_ctrl_enable)
    {
        float pid_out   = PID_Calc(&s_pid, g_ctrl_speed_meas);
        float ff        = g_ctrl_speed_target * FEEDFORWARD_Kf;
        float final_out = pid_out + ff;

        // 输出限幅
        if (final_out >  14999.0f) final_out =  14999.0f;
        if (final_out < -14999.0f) final_out = -14999.0f;

        // 电机方向与PWM输出
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
