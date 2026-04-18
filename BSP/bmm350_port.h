#ifndef BMM350_PORT_H
#define BMM350_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "bmm350_defs.h"
#include "bmm350.h"

/* 磁力计数据结构，供外部模块使用 */
typedef struct {
    float x;           /* uT */
    float y;           /* uT */
    float z;           /* uT */
    float temperature; /* degC */
    uint8_t valid;     /* 1=本次有新数据，0=无新数据 */
} BMM350_Data_t;

void    BMM350_Init(struct bmm350_dev *dev);
void    BMM350_Test_Raw(struct bmm350_dev *dev);
void    BMM350_Test_Compensated(struct bmm350_dev *dev);
uint8_t BMM350_Read(struct bmm350_dev *dev, BMM350_Data_t *out);

#ifdef __cplusplus
}
#endif

#endif /* BMM350_PORT_H */
