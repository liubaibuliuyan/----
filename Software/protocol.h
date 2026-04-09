#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stddef.h>

// ================= 配置区域（用户根据需求改） =================
#define USE_MULTI_DEVICE    0    // 0: 1对1通信  1: 多机通信
#define MAX_DATA_LEN        32   // 最大数据段长度(电赛推荐32)
#define FRAME_HEAD          0xAA55 // 双字节帧头(固定)
#define USE_SEQ_CHECK       0    // 0: 不检查序列号  1: 检查(防重放/乱序)
// =========================================================

// 动态计算缓冲区大小(避免溢出)
#if (USE_MULTI_DEVICE == 1)
    #define FRAME_FIXED_LEN (1+1+1+1+2) // dest+src+func+seq+len+crc16
#else
    #define FRAME_FIXED_LEN (1+1+1+2)     // func+seq+len+crc16
#endif
#define RAW_BUF_SIZE        (FRAME_FIXED_LEN + MAX_DATA_LEN)
#define ENC_BUF_SIZE        (RAW_BUF_SIZE + (RAW_BUF_SIZE / 254) + 2) // COBS最坏情况
#define RECV_BUF_SIZE       (ENC_BUF_SIZE + 2) // 接收缓冲区

// 错误码定义
typedef enum {
    PROTOCOL_OK            = 0,
    PROTOCOL_ERR_HEAD      = 1, // 帧头错误
    PROTOCOL_ERR_LEN       = 2, // 长度错误
    PROTOCOL_ERR_CRC       = 3, // CRC错误
    PROTOCOL_ERR_TIMEOUT   = 4, // 超时(保留未用)
    PROTOCOL_ERR_OVERFLOW  = 5, // 缓冲区溢出
    PROTOCOL_ERR_SEQ       = 6  // 序列号错误
} ProtocolErr_t;

// 帧结构定义
typedef struct {
#if (USE_MULTI_DEVICE == 1)
    uint8_t  dest;    // 目标地址
    uint8_t  src;     // 源地址
#endif
    uint8_t  func;    // 功能码
    uint8_t  seq;     // 序列号
    uint8_t  len;     // 数据长度
    uint8_t  data[MAX_DATA_LEN]; // 数据段
    uint16_t crc16;   // CRC16校验
} Frame_t;

// 接收状态枚举
typedef enum {
    RX_STATE_WAIT_HEAD_LOW,
    RX_STATE_WAIT_HEAD_HIGH,
    RX_STATE_RECV_DATA,
    RX_STATE_RECV_DONE,   // 收到结束符
    RX_STATE_ERROR
} RxState_t;

// 协议上下文
typedef struct {
    RxState_t    state;
    uint8_t      recv_buf[RECV_BUF_SIZE];
    uint16_t     recv_len;
    Frame_t      frame;
    ProtocolErr_t last_err;
    uint8_t      tx_seq;
    uint8_t      rx_seq;
} ProtocolContext_t;

// ================= 核心函数声明 =================
void Protocol_Init(ProtocolContext_t *ctx);
void Protocol_Reset(ProtocolContext_t *ctx);

// 发送一帧
void Protocol_SendFrame(
#if (USE_MULTI_DEVICE == 1)
    uint8_t dest, uint8_t src,
#endif
    uint8_t func, const uint8_t *data, uint8_t data_len,
    void (*send_byte)(uint8_t byte),
    ProtocolContext_t *ctx
);

// 中断/回调中喂入单字节
void Protocol_RecvByte(ProtocolContext_t *ctx, uint8_t byte);

// 主循环处理，返回PROTOCOL_OK表示收到完整帧
ProtocolErr_t Protocol_Process(ProtocolContext_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */
