#include "start.h"

extern volatile uint32_t g_tim6_1ms_cnt;

static u32 key1_press_cnt = 0;
static ProtocolContext_t g_proto_ctx;

static void Task_Key(void);
static void Task_LCD_Show(void);
static void Task_USART_Report(void);

void Start_MainLoop(void)
{
    uint8_t rx_byte = 0;

    while (1)
    {
        while (USART_Try_Get_Byte(USART_ID_1, &rx_byte))
        {
            Protocol_RecvByte(&g_proto_ctx, rx_byte);
        }

        ProtocolErr_t proto_err = Protocol_Process(&g_proto_ctx);

        if (proto_err == PROTOCOL_OK)
        {
            if (g_proto_ctx.frame.func != 0)
            {
                USART_Send_String(USART_ID_1, "[Proto] Valid Frame! Func=%d, Seq=%d, DataLen=%d\r\n",
                                   g_proto_ctx.frame.func,
                                   g_proto_ctx.frame.seq,
                                   g_proto_ctx.frame.len);
            }
        }

        Task_Key();
        Task_LCD_Show();
        Task_USART_Report();
    }
}

static void Task_Key(void)
{
    if (!Key_Scan(KEY_ID_1)) return;
    key1_press_cnt++;

    if (!g_ctrl_enable)
    {
        static uint8_t mode_cnt = 0;
        uint8_t mode_sel = mode_cnt % 2;   // 现在只有 2 个模式
        mode_cnt++;

        Ctrl_SetMode((CtrlMode_t)mode_sel);

        switch (mode_sel)
        {
            case CTRL_MODE_SPEED:
                Ctrl_SetSpeedTarget(3.0f);
                USART_Send_String(USART_ID_1, "Mode: SPEED  Tgt=3.0rps\r\n");
                break;

            case CTRL_MODE_POS:
                Ctrl_SetPosTarget(7000, 3.0f);
                USART_Send_String(USART_ID_1, "Mode: POS    Tgt=6000pulse\r\n");
                break;
        }

        Ctrl_Start();
    }
    else
    {
        Ctrl_Stop();
        USART_Send_String(USART_ID_1, "Motor STOPPED\r\n");
    }

    USART_Send_String(USART_ID_1, "KEY=%d\r\n", key1_press_cnt);
}

static void Task_LCD_Show(void)
{
    static uint32_t lcd_last_ms = 0;
    if ((g_tim6_1ms_cnt - lcd_last_ms) < 100) return;
    lcd_last_ms = g_tim6_1ms_cnt;

    int32_t tgt_x100 = (int32_t)(g_ctrl_speed_target * 100.0f);
    int32_t spd_x100 = (int32_t)(g_ctrl_speed_meas   * 100.0f);

    lcd_show_u32(64, 0,   RED,   BLACK, key1_press_cnt);
    lcd_show_s32(64, 64,  GREEN, BLACK, tgt_x100);
    lcd_show_s32(64, 80,  RED,   BLACK, spd_x100);
    lcd_show_s32(64, 120, WHITE, BLACK, g_ctrl_total_pulse);
}

static void Task_USART_Report(void)
{
    static uint32_t last_ms = 0;
    if ((g_tim6_1ms_cnt - last_ms) < 100) return;
    last_ms = g_tim6_1ms_cnt;

    int32_t tgt_i = (int32_t)g_ctrl_speed_target;
    int32_t tgt_f = (int32_t)((g_ctrl_speed_target - (float)tgt_i) * 100.0f);
    if (tgt_f < 0) tgt_f = -tgt_f;

    int32_t spd_i = (int32_t)g_ctrl_speed_meas;
    int32_t spd_f = (int32_t)((g_ctrl_speed_meas - (float)spd_i) * 100.0f);
    if (spd_f < 0) spd_f = -spd_f;

    USART_Send_String(USART_ID_1,
        "Tgt=%ld.%02ld Act=%ld.%02ld r/s  Pulse=%ld\r\n",
        (long)tgt_i, (long)tgt_f,
        (long)spd_i, (long)spd_f,
        (long)g_ctrl_total_pulse);
}

void Start_Init(void)
{
    USART_Init();
    Lcd_Init();
    Lcd_Clear(BLACK);
    Encoder_Init();
    Motor_Init();
    Timer6_Interrupt_Init();
    Timer7_Interrupt_Init();
    Key_Init();

    Protocol_Init(&g_proto_ctx);

    lcd_show_str(5, 0,   WHITE, BLACK, "KEY:");
    lcd_show_str(5, 40,  WHITE, BLACK, "Tgt*100:");
    lcd_show_str(5, 80,  WHITE, BLACK, "Spd*100:");
    lcd_show_str(5, 120, WHITE, BLACK, "Pulse:");

    USART_Send_String(USART_ID_1, "System Ready\r\n");
}
