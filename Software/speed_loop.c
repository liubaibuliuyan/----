#include "speed_loop.h"

volatile float    g_ctrl_speed_meas   = 0.0f;
volatile float    g_ctrl_speed_target = 0.0f;
volatile int32_t  g_ctrl_total_pulse  = 0;
volatile uint8_t  g_ctrl_enable       = 0;

static float      s_speed_filtered = 0.0f;
static float      s_alpha;
// 替换为增量式PID结构体
static IncPID_t   s_speed_pid;

void SpeedLoop_Init(void)
{
    s_alpha = (float)CTRL_PERIOD_MS / (SPEED_TAU_MS + (float)CTRL_PERIOD_MS);

    // 增量PID初始化：周期1ms
    IncPID_Init(&s_speed_pid,
                160.0f, 0.03f, 0,
                CTRL_PERIOD_S,
                -14999.0f, 14999.0f);

    s_speed_filtered = 0.0f;
}

void Ctrl_Enable(void)
{
    IncPID_Reset(&s_speed_pid);
    s_speed_filtered = 0.0f;
    g_ctrl_enable    = 1;
}

void Ctrl_Disable(void)
{
    g_ctrl_enable       = 0;
    g_ctrl_speed_target = 0.0f;
    IncPID_Reset(&s_speed_pid);
    Motor_Control(MOTOR_ID_1, MOTOR_STOP, 0);
}

void Ctrl_Set_Target(float target_rps)
{
    g_ctrl_speed_target = target_rps;
    IncPID_SetTarget(&s_speed_pid, target_rps);
}

void SpeedLoop_Process(int32_t diff_pulse_1ms, int32_t total_pulse)
{
    g_ctrl_total_pulse = total_pulse;

    float raw_speed = (float)diff_pulse_1ms / ENCODER_COUNTS_PER_REV / CTRL_PERIOD_S;
    s_speed_filtered  = s_speed_filtered * (1.0f - s_alpha) + raw_speed * s_alpha;
    g_ctrl_speed_meas = s_speed_filtered;

    if(g_ctrl_enable)
    {
        // 调用增量PID计算
        float pid_out   = IncPID_Calc(&s_speed_pid, g_ctrl_speed_meas);
        float ff        = g_ctrl_speed_target * FEEDFORWARD_Kf;
        float final_out = pid_out + ff;

        if(final_out >  14999.0f) final_out =  14999.0f;
        if(final_out < -14999.0f) final_out = -14999.0f;

        Motor_DirTypeDef dir;
        uint16_t pwm;
        if(final_out > 0)
        {
            dir = MOTOR_FORWARD;
            pwm = (uint16_t)final_out;
        }
        else if(final_out < 0)
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
