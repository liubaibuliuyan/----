#include "protocol.h"

// CRC16-Modbus 查找表
static const uint16_t crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static uint16_t _CRC16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    while (len--) crc = (crc >> 8) ^ crc16_table[(crc ^ *data++) & 0xFF];
    return crc;
}

static uint16_t _COBS_Encode(const uint8_t *in, uint16_t in_len, uint8_t *out) {
    uint16_t r = 0, w = 1, c_idx = 0;
    uint8_t code = 1;
    while (r < in_len) {
        if (in[r] == 0) {
            out[c_idx] = code; code = 1; c_idx = w++; r++;
        } else {
            out[w++] = in[r++]; code++;
            if (code == 0xFF) { out[c_idx] = code; code = 1; c_idx = w++; }
        }
    }
    out[c_idx] = code;
    return w;
}

static uint16_t _COBS_Decode(const uint8_t *in, uint16_t in_len, uint8_t *out) {
    uint16_t r = 0, w = 0;
    uint8_t code, i;
    while (r < in_len) {
        code = in[r];
        if (r + code > in_len && code != 1) return 0;
        r++;
        for (i = 1; i < code; i++) out[w++] = in[r++];
        if (code < 0xFF && r < in_len) out[w++] = 0;
    }
    return w;
}

void Protocol_Init(ProtocolContext_t *ctx) {
    memset(ctx, 0, sizeof(ProtocolContext_t));
    ctx->state = RX_STATE_WAIT_HEAD_LOW;
    ctx->last_err = PROTOCOL_OK;
}

void Protocol_Reset(ProtocolContext_t *ctx) {
    ctx->state = RX_STATE_WAIT_HEAD_LOW;
    ctx->recv_len = 0;
    ctx->last_err = PROTOCOL_OK;
    memset(ctx->recv_buf, 0, RECV_BUF_SIZE);
}

// ==========================================================================
// 发送时追加 0x00 结束符
// ==========================================================================
void Protocol_SendFrame(
#if (USE_MULTI_DEVICE == 1)
    uint8_t dest, uint8_t src,
#endif
    uint8_t func, const uint8_t *data, uint8_t data_len,
    void (*send_byte)(uint8_t byte),
    ProtocolContext_t *ctx) {
    
    if (!send_byte || !ctx || data_len > MAX_DATA_LEN) return;

    uint8_t raw[RAW_BUF_SIZE];
    uint16_t raw_len = 0;

#if (USE_MULTI_DEVICE == 1)
    raw[raw_len++] = dest;
    raw[raw_len++] = src;
#endif
    raw[raw_len++] = func;
    raw[raw_len++] = ctx->tx_seq++;
    raw[raw_len++] = data_len;
    if (data && data_len) { memcpy(&raw[raw_len], data, data_len); raw_len += data_len; }

    uint16_t crc = _CRC16(raw, raw_len);
    raw[raw_len++] = crc & 0xFF;
    raw[raw_len++] = (crc >> 8) & 0xFF;

    uint8_t enc[ENC_BUF_SIZE];
    uint16_t enc_len = _COBS_Encode(raw, raw_len, enc);

    send_byte(FRAME_HEAD & 0xFF);
    send_byte((FRAME_HEAD >> 8) & 0xFF);
    for (uint16_t i = 0; i < enc_len; i++) send_byte(enc[i]);
    
    // 发送结束符
    send_byte(0x00);
}

// ==========================================================================
// 接收：已清理无用时钟/超时参数
// ==========================================================================
void Protocol_RecvByte(ProtocolContext_t *ctx, uint8_t byte) {
    if (!ctx) return;

    if (ctx->state == RX_STATE_ERROR) return;

    switch (ctx->state) {
        case RX_STATE_WAIT_HEAD_LOW:
            if (byte == (FRAME_HEAD & 0xFF))
                ctx->state = RX_STATE_WAIT_HEAD_HIGH;
            break;

        case RX_STATE_WAIT_HEAD_HIGH:
            if (byte == ((FRAME_HEAD >> 8) & 0xFF)) {
                ctx->state = RX_STATE_RECV_DATA;
                ctx->recv_len = 0;
                memset(ctx->recv_buf, 0, RECV_BUF_SIZE);
            } else {
                ctx->state = RX_STATE_WAIT_HEAD_LOW;
            }
            break;

        case RX_STATE_RECV_DATA:
            // 遇到0x00表示帧结束
            if (byte == 0x00) {
                ctx->state = RX_STATE_RECV_DONE;
            } else {
                if (ctx->recv_len < sizeof(ctx->recv_buf)) {
                    ctx->recv_buf[ctx->recv_len++] = byte;
                } else {
                    ctx->last_err = PROTOCOL_ERR_OVERFLOW;
                    ctx->state = RX_STATE_ERROR;
                }
            }
            break;

        default:
            break;
    }
}

// ==========================================================================
// 处理帧：已清理无用时钟/超时
// ==========================================================================
ProtocolErr_t Protocol_Process(ProtocolContext_t *ctx) {
    if (!ctx) return PROTOCOL_ERR_LEN;

    if (ctx->state == RX_STATE_ERROR)
        return ctx->last_err;

    // 只在收完一帧才处理
    if (ctx->state != RX_STATE_RECV_DONE)
        return PROTOCOL_OK;

    ProtocolErr_t err = PROTOCOL_OK;
    uint8_t dec[RAW_BUF_SIZE] = {0};
    uint16_t dec_len = _COBS_Decode(ctx->recv_buf, ctx->recv_len, dec);

    do {
        if (dec_len == 0) { err = PROTOCOL_ERR_HEAD; break; }
        if (dec_len < FRAME_FIXED_LEN) { err = PROTOCOL_ERR_LEN; break; }

        uint16_t idx = 0;

#if (USE_MULTI_DEVICE == 1)
        ctx->frame.dest = dec[idx++];
        ctx->frame.src = dec[idx++];
#endif
        ctx->frame.func = dec[idx++];
        ctx->frame.seq = dec[idx++];
        ctx->frame.len = dec[idx++];

        if (ctx->frame.len > MAX_DATA_LEN) { err = PROTOCOL_ERR_LEN; break; }
        if (dec_len != FRAME_FIXED_LEN + ctx->frame.len) { err = PROTOCOL_ERR_LEN; break; }

        memset(ctx->frame.data, 0, MAX_DATA_LEN);
        if (ctx->frame.len > 0)
            memcpy(ctx->frame.data, &dec[idx], ctx->frame.len);
        idx += ctx->frame.len;

        uint16_t recv_crc = dec[idx] | (dec[idx+1] << 8);
        uint16_t calc_crc = _CRC16(dec, dec_len - 2);
        if (recv_crc != calc_crc) { err = PROTOCOL_ERR_CRC; break; }
        ctx->frame.crc16 = recv_crc;

#if (USE_SEQ_CHECK == 1)
        if (ctx->frame.seq != (ctx->rx_seq + 1) && ctx->rx_seq != 0) {
            err = PROTOCOL_ERR_SEQ; break;
        }
        ctx->rx_seq = ctx->frame.seq;
#endif

    } while (0);

    ctx->last_err = err;
    Protocol_Reset(ctx);
    return err;
}
