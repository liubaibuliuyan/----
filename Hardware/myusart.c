#include "myusart.h"

// ===================== 修复：DMA缓冲区放到结构体外面 =====================
uint8_t dma_rx_buf[USART_DEFAULT_RX_BUF_SIZE] __attribute__((section(".RAM_D2")));

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t rx_buf[USART_DEFAULT_RX_BUF_SIZE];
    uint8_t tx_buf[USART_DEFAULT_TX_BUF_SIZE];
    char format_buf[USART_DEFAULT_FORMAT_BUF_SIZE];
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
} USART_HandleTypeDef;

static USART_HandleTypeDef usart_list[] = {
    {&huart1},
};

#define USART_ID_MAX (sizeof(usart_list) / sizeof(USART_HandleTypeDef))

static uint8_t _USART_CheckIDValid(USART_IDTypeDef usart_id) {
    return (usart_id < USART_ID_MAX && usart_list[usart_id].huart != NULL) ? 1 : 0;
}

static USART_HandleTypeDef* _USART_GetHandle(USART_IDTypeDef usart_id) {
    return _USART_CheckIDValid(usart_id) ? &usart_list[usart_id] : NULL;
}

// ==========================================================================
// DMA IDLE 中断处理
// ==========================================================================
void USART1_IDLE_IRQ_Process(void)
{
    USART_HandleTypeDef *husart = &usart_list[USART_ID_1];
    UART_HandleTypeDef *huart = husart->huart;

    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        HAL_UART_DMAStop(huart);

        uint16_t len = USART_DEFAULT_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);

        // 压入环形缓冲区
        for(uint16_t i=0; i<len; i++)
        {
            uint16_t next_head = (husart->rx_head + 1) % USART_DEFAULT_RX_BUF_SIZE;
            if(next_head != husart->rx_tail)
            {
                husart->rx_buf[husart->rx_head] = dma_rx_buf[i];
                husart->rx_head = next_head;
            }
        }

        HAL_UART_Receive_DMA(huart, dma_rx_buf, USART_DEFAULT_RX_BUF_SIZE);
    }
}

// ==========================================================================
// 旧回调空实现
// ==========================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

}

// ==========================================================================
// 初始化
// ==========================================================================
void USART_Init(void)
{
    // ===================== 修复：用uint8_t代替枚举 =====================
    for(uint8_t i = 0; i < USART_ID_MAX; i++)
    {
        USART_HandleTypeDef *husart = &usart_list[i];
        if (!husart->huart) continue;

        husart->rx_head = 0;
        husart->rx_tail = 0;
        memset(husart->rx_buf, 0, USART_DEFAULT_RX_BUF_SIZE);

        // 启动DMA
        HAL_UART_Receive_DMA(husart->huart, dma_rx_buf, USART_DEFAULT_RX_BUF_SIZE);
        __HAL_UART_ENABLE_IT(husart->huart, UART_IT_IDLE);
    }
}

// ==========================================================================
// 发送一字节
// ==========================================================================
void USART_Send_Byte(USART_IDTypeDef usart_id, uint8_t ch)
{
    USART_HandleTypeDef *husart = _USART_GetHandle(usart_id);
    if (husart)
        HAL_UART_Transmit(husart->huart, &ch, 1, 100);
}

// ==========================================================================
// 格式化发送
// ==========================================================================
void USART_Send_String(USART_IDTypeDef usart_id, const char *fmt, ...)
{
    USART_HandleTypeDef *husart = _USART_GetHandle(usart_id);
    if (!husart || !fmt) return;

    va_list ap;
    va_start(ap, fmt);
    int str_len = vsnprintf(husart->format_buf, USART_DEFAULT_FORMAT_BUF_SIZE-1, fmt, ap);
    va_end(ap);

    if (str_len > 0)
    {
        HAL_UART_Transmit(husart->huart, (uint8_t*)husart->format_buf, str_len, 100);
    }
}

// ==========================================================================
// 取字节
// ==========================================================================
uint8_t USART_Try_Get_Byte(USART_IDTypeDef usart_id, uint8_t *out_byte)
{
    USART_HandleTypeDef *husart = _USART_GetHandle(usart_id);
    if (!husart || !out_byte) return 0;

    __disable_irq();
    if (husart->rx_head == husart->rx_tail)
    {
        __enable_irq();
        return 0;
    }

    *out_byte = husart->rx_buf[husart->rx_tail];
    husart->rx_tail = (husart->rx_tail + 1) % USART_DEFAULT_RX_BUF_SIZE;
    __enable_irq();
    return 1;
}
