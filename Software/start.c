#include "start.h"


/*==========================================================
 * 外部变量声明
 *==========================================================*/
extern volatile uint32_t g_tim6_1ms_cnt;
extern volatile float    g_ctrl_speed_meas;
extern volatile float    g_ctrl_speed_target;
extern volatile int32_t  g_ctrl_total_pulse;
extern volatile uint8_t  g_ctrl_enable;

/*==========================================================
 * 私有变量
 *==========================================================*/
static u32 key1_press_cnt = 0;
static float target_speed_rps = 1.0f;
static ProtocolContext_t g_proto_ctx;

/*==========================================================
 * 私有函数声明
 *==========================================================*/
static void Task_Key(void);
static void Task_LCD_Show(void);
static void Task_USART_Report(void);
static void _Protocol_SendByte_Callback(uint8_t byte);

/*==========================================================
 * 主程序入口
 *==========================================================*/
void Start_MainLoop(void)
{
    uint8_t rx_byte = 0;

    while (1)
    {
			
        // 1. 串口字节喂入协议层
        while (USART_Try_Get_Byte(USART_ID_1, &rx_byte))
        {
            Protocol_RecvByte(&g_proto_ctx, rx_byte);
        }

        // 2. 协议层帧解析
        ProtocolErr_t proto_err = Protocol_Process(&g_proto_ctx);

        // 【优化】只在收到有效帧（功能码非0）时打印，避免刷屏
        if (proto_err == PROTOCOL_OK)
        {
            // 示例：只在功能码非0时打印，你可以根据自己的功能码定义修改
            if (g_proto_ctx.frame.func != 0)
            {
                USART_Send_String(USART_ID_1, "[Proto] Valid Frame! Func=%d, Seq=%d, DataLen=%d\r\n", 
                                   g_proto_ctx.frame.func, 
                                   g_proto_ctx.frame.seq,
                                   g_proto_ctx.frame.len);
            }

            // ==============================================
            // 在这里写你的指令处理逻辑
            // 示例：if (g_proto_ctx.frame.func == 0x01) 设置电机速度
            // ==============================================
        }

        // 3. 原有业务任务
        Task_Key();
        Task_LCD_Show();
        Task_USART_Report();
				//HAL_Delay(1); 
    }
}

/*==========================================================
 * 按键任务
 *==========================================================*/
static void Task_Key(void)
{
    if (!Key_Scan(KEY_ID_1)) return;

    key1_press_cnt++;

    if (!g_ctrl_enable)
    {
        Encoder_Reset(ENCODER_ID_1);
        Ctrl_Set_Target(target_speed_rps);
        Ctrl_Enable();
        USART_Send_String(USART_ID_1, "Motor RUNNING\r\n");
    }
    else
    {
        Ctrl_Disable();
        USART_Send_String(USART_ID_1, "Motor STOPPED\r\n");
    }

    USART_Send_String(USART_ID_1, "KEY=%d\r\n", key1_press_cnt);
}

/*==========================================================
 * LCD显示任务 100ms刷新一次
 *==========================================================*/
static void Task_LCD_Show(void)
{
    static uint32_t lcd_last_ms = 0;
    if ((g_tim6_1ms_cnt - lcd_last_ms) < 100) return;
    lcd_last_ms = g_tim6_1ms_cnt;

    int32_t tgt_x100 = (int32_t)(g_ctrl_speed_target * 100.0f);
    int32_t spd_x100 = (int32_t)(g_ctrl_speed_meas   * 100.0f);

    lcd_show_u32(64, 0,   RED,   BLACK, key1_press_cnt);
    lcd_show_s32(64, 40,  GREEN, BLACK, tgt_x100);
    lcd_show_s32(64, 80,  RED,   BLACK, spd_x100);
    lcd_show_s32(64, 120, WHITE, BLACK, g_ctrl_total_pulse);
}

/*==========================================================
 * 串口定时上报任务 (100ms)
 *==========================================================*/
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

/*==========================================================
 * 协议层发送回调
 *==========================================================*/
static void _Protocol_SendByte_Callback(uint8_t byte)
{
    USART_Send_Byte(USART_ID_1, byte);
}

/*==========================================================
 * 系统初始化
 *==========================================================*/
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
