#include "speed_loop.h"
#include "pos_loop.h"    
#include "ctrl_mode.h"

volatile float    g_ctrl_speed_meas   = 0.0f;
volatile float    g_ctrl_speed_target = 0.0f;
volatile int32_t  g_ctrl_total_pulse  = 0;
volatile uint8_t  g_ctrl_enable       = 0;

static uint32_t   s_tick_100us     = 0;
static int32_t    s_m_pulse        = 0;
static float      s_speed_filtered = 0.0f;
static float      s_alpha;
static PID_t      s_pid;

void SpeedLoop_Init(void)
{
    s_alpha = (float)CTRL_PERIOD_MS / (SPEED_TAU_MS + (float)CTRL_PERIOD_MS);

    PID_Init(&s_pid,
             160.0f, 0.03f, 0.007f,
             CTRL_PERIOD_S,
             -14999.0f, 14999.0f);

    s_tick_100us     = 0;
    s_speed_filtered = 0.0f;
    s_m_pulse        = 0;
}

void Ctrl_Enable(void)
{
    PID_Reset(&s_pid);
    s_speed_filtered = 0.0f;
    s_m_pulse        = 0;
    g_ctrl_enable    = 1;
}

void Ctrl_Disable(void)
{
    g_ctrl_enable       = 0;
    g_ctrl_speed_target = 0.0f;   /* ★ 停止时清零目标速度，串口显示正确 */
    PID_Reset(&s_pid);
    Motor_Control(MOTOR_ID_1, MOTOR_STOP, 0);
}

void Ctrl_Set_Target(float target_rps)
{
    g_ctrl_speed_target = target_rps;
    PID_Set_Target(&s_pid, target_rps);
}

void SpeedLoop_Process(int16_t diff_pulse, int32_t total_pulse)
{
    g_ctrl_total_pulse = total_pulse;
    s_m_pulse         += diff_pulse;

    s_tick_100us++;
    if (s_tick_100us >= 10)
    {
        s_tick_100us = 0;

        int32_t pulse   = s_m_pulse;
        s_m_pulse       = 0;
        float raw_speed = (float)pulse / ENCODER_COUNTS_PER_REV / CTRL_PERIOD_S;

        s_speed_filtered  = s_speed_filtered * (1.0f - s_alpha) + raw_speed * s_alpha;
        g_ctrl_speed_meas = s_speed_filtered;

        /* 位置环调度 */
        CtrlMode_t mode = Ctrl_GetMode();
        if (g_ctrl_enable &&
            (mode == CTRL_MODE_POS || mode == CTRL_MODE_SPEED_POS))
        {
            PosLoop_Process(g_ctrl_total_pulse);
        }

        /* 速度环输出 */
        if (g_ctrl_enable)
        {
            float pid_out   = PID_Calc(&s_pid, g_ctrl_speed_meas);
            float ff        = g_ctrl_speed_target * FEEDFORWARD_Kf;
            float final_out = pid_out + ff;

            if (final_out >  14999.0f) final_out =  14999.0f;
            if (final_out < -14999.0f) final_out = -14999.0f;

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
