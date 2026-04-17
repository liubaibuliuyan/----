/*
 * start.c
 * Modified: added BMM350 init and Task_BMM350().
 */
#include "start.h"

extern volatile uint32_t g_tim6_1ms_cnt;

/* ---- 原有变量 ---- */
static uint32_t         key1_press_cnt = 0;
static ProtocolContext_t g_proto_ctx;

/* ---- BMM350 新增全局变量 ---- */
static struct bmm350_dev g_bmm350_dev = {0};  /* 设备句柄 */
static BMM350_Data_t     g_bmm350_data;        /* 最新一帧磁场数据 */

/* ---- 任务函数声明 ---- */
static void Task_Key(void);
static void Task_LCD_Show(void);
static void Task_USART_Report(void);
static void Task_BMM350(void);   /* 新增 */

/* ==================================================================
   主循环
   ================================================================== */
void Start_MainLoop(void)
{
    uint8_t rx_byte = 0;

    while (1)
    {
        /* 串口接收处理 */
        while (USART_Try_Get_Byte(USART_ID_1, &rx_byte))
        {
            Protocol_RecvByte(&g_proto_ctx, rx_byte);
        }

        ProtocolErr_t proto_err = Protocol_Process(&g_proto_ctx);
        if (proto_err == PROTOCOL_OK)
        {
            if (g_proto_ctx.frame.func != 0)
            {
                USART_Send_String(USART_ID_1,
                    "[Proto] Valid Frame! Func=%d, Seq=%d, DataLen=%d\r\n",
                    g_proto_ctx.frame.func,
                    g_proto_ctx.frame.seq,
                    g_proto_ctx.frame.len);
            }
        }

        Task_Key();
        Task_LCD_Show();
        Task_USART_Report();
        Task_BMM350();    /* 新增：磁力计轮询任务 */
    }
}

/* ==================================================================
   初始化
   ================================================================== */
void Start_Init(void)
{
    /* ---- 原有初始化（顺序保持不变） ---- */
    USART_Init();
    Lcd_Init();
    Lcd_Clear(BLACK);
    Encoder_Init();
    Motor_Init();
    Timer6_Interrupt_Init();
    Timer7_Interrupt_Init();
    Key_Init();
    Protocol_Init(&g_proto_ctx);

    /* ---- BMM350 新增：DWT延时初始化必须在BMM350之前 ---- */
    DWT_Delay_Init();
    rtrobot_bmm350_init(&g_bmm350_dev);  /* 初始化磁力计 */

    /* ---- 原有LCD标签 ---- */
    lcd_show_str(5, 0,   WHITE, BLACK, "KEY:");
    lcd_show_str(5, 40,  WHITE, BLACK, "Tgt*100:");
    lcd_show_str(5, 80,  WHITE, BLACK, "Spd*100:");
    lcd_show_str(5, 120, WHITE, BLACK, "Pulse:");

    USART_Send_String(USART_ID_1, "System Ready\r\n");
}

/* ==================================================================
   Task: 按键
   ================================================================== */
static void Task_Key(void)
{
    if (!Key_Scan(KEY_ID_1)) return;

    key1_press_cnt++;

    if (!g_ctrl.enable)
    {
        static uint8_t mode_cnt = 0;
        uint8_t mode_sel = mode_cnt % 2;
        mode_cnt++;

        Ctrl_SetMode((CtrlMode_t)mode_sel);
        switch (mode_sel)
        {
            case CTRL_MODE_SPEED:
                Ctrl_SetSpeedTarget(3.0f);
                USART_Send_String(USART_ID_1, "Mode: SPEED  Tgt=3.0rps\r\n");
                break;
            case CTRL_MODE_POS:
                Ctrl_SetPosTarget(6000, 3.0f);
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

/* ==================================================================
   Task: LCD显示（100ms刷新）
   ================================================================== */
static void Task_LCD_Show(void)
{
    static uint32_t lcd_last_ms = 0;
    if ((g_tim6_1ms_cnt - lcd_last_ms) < 100) return;
    lcd_last_ms = g_tim6_1ms_cnt;

    int32_t tgt_x100 = (int32_t)(g_ctrl.speed_target * 100.0f);
    int32_t spd_x100 = (int32_t)(g_ctrl.speed_meas   * 100.0f);

    lcd_show_u32(64,   0, RED,   BLACK, key1_press_cnt);
    lcd_show_s32(64,  64, GREEN, BLACK, tgt_x100);
    lcd_show_s32(64,  80, RED,   BLACK, spd_x100);
    lcd_show_s32(64, 120, WHITE, BLACK, g_ctrl.total_pulse);
}

/* ==================================================================
   Task: 串口上报电机状态（100ms）
   ================================================================== */
static void Task_USART_Report(void)
{
    static uint32_t last_ms = 0;
    if ((g_tim6_1ms_cnt - last_ms) < 100) return;
    last_ms = g_tim6_1ms_cnt;

    int32_t tgt_i = (int32_t)g_ctrl.speed_target;
    int32_t tgt_f = (int32_t)((g_ctrl.speed_target - (float)tgt_i) * 100.0f);
    if (tgt_f < 0) tgt_f = -tgt_f;

    int32_t spd_i = (int32_t)g_ctrl.speed_meas;
    int32_t spd_f = (int32_t)((g_ctrl.speed_meas - (float)spd_i) * 100.0f);
    if (spd_f < 0) spd_f = -spd_f;

    USART_Send_String(USART_ID_1,
        "Tgt=%ld.%02ld Act=%ld.%02ld r/s  Pulse=%ld\r\n",
        (long)tgt_i, (long)tgt_f,
        (long)spd_i, (long)spd_f,
        (long)g_ctrl.total_pulse);
}

/* ==================================================================
   Task: BMM350磁力计读取（20ms轮询，50Hz与传感器ODR一致）
   有新数据时通过串口上报
   ================================================================== */
static void Task_BMM350(void)
{
    static uint32_t last_ms = 0;
    if ((g_tim6_1ms_cnt - last_ms) < 20) return;
    last_ms = g_tim6_1ms_cnt;

    if (rtrobot_bmm350_read(&g_bmm350_dev, &g_bmm350_data))
    {
        /* 串口绘图仪格式，可用Serial Studio等工具实时绘波形 */
        USART_Send_String(USART_ID_1,
            "$0:%.2f;$1:%.2f;$2:%.2f;\r\n",
            g_bmm350_data.x,
            g_bmm350_data.y,
            g_bmm350_data.z);
    }
}
