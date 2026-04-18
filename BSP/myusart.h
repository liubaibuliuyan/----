#ifndef __MYUSART_H
#define __MYUSART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include "main.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// 配置区
#define USART_DEFAULT_RX_BUF_SIZE 256
#define USART_DEFAULT_TX_BUF_SIZE 256
#define USART_DEFAULT_FORMAT_BUF_SIZE 256

// 串口ID枚举
typedef enum {
    USART_ID_1 = 0,
    USART_ID_MAX
} USART_IDTypeDef;

// 核心接口
void USART_Init(void);

// 发送接口
void USART_Send_Byte(USART_IDTypeDef usart_id, uint8_t ch);
void USART_Send_String(USART_IDTypeDef usart_id, const char *fmt, ...);

// 接收接口：尝试从缓冲区取出一个字节
// 返回值：1=成功取出，0=缓冲区空
uint8_t USART_Try_Get_Byte(USART_IDTypeDef usart_id, uint8_t *out_byte);

#ifdef __cplusplus
}
#endif

#endif /* __MYUSART_H */
