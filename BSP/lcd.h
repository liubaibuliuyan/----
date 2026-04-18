#ifndef __LCD_H
#define __LCD_H

#include "stdio.h"
#include "string.h"
#include "stm32h7xx_hal.h"
#include "gpio.h"
#include "spi.h"
#include "font.h"

/********************* 自定义类型别名 *********************/
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s32 int32_t

/********************* LCD分辨率（1.8寸ST7735S标准） *********************/
#define X_MAX_PIXEL        128
#define Y_MAX_PIXEL        160

/********************* LCD引脚宏定义 *********************/
#define LCD_RST_GPIO       GPIOB
#define LCD_RST_PIN        GPIO_PIN_0
#define LCD_RS_GPIO        GPIOB
#define LCD_RS_PIN         GPIO_PIN_1
#define LCD_CS_GPIO        GPIOB
#define LCD_CS_PIN         GPIO_PIN_2

/********************* RGB565颜色宏定义 *********************/
#define WHITE              0xFFFF
#define BLACK              0x0000
#define RED                0xF800
#define GREEN              0x07E0
#define BLUE               0x001F
#define YELLOW             0xFFE0
#define GRAY               0x8410
#define MAGENTA            0xF81F
#define CYAN               0x07FF

/********************* 引脚操作宏 *********************/
#define LCD_RST_CLR        HAL_GPIO_WritePin(LCD_RST_GPIO, LCD_RST_PIN, GPIO_PIN_RESET)
#define LCD_RST_SET        HAL_GPIO_WritePin(LCD_RST_GPIO, LCD_RST_PIN, GPIO_PIN_SET)
#define LCD_RS_CLR         HAL_GPIO_WritePin(LCD_RS_GPIO, LCD_RS_PIN, GPIO_PIN_RESET)
#define LCD_RS_SET         HAL_GPIO_WritePin(LCD_RS_GPIO, LCD_RS_PIN, GPIO_PIN_SET)
#define LCD_CS_CLR         HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_SET         HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_SET)

/********************* 字符/变量显示参数 *********************/
#define ASCII16_WIDTH      8   // 16x8点阵字符宽度
#define ASCII16_HEIGHT     16  // 16x8点阵字符高度
#define VAR_DEFAULT_DIGITS 5   // 变量显示默认最大字符数
#define CACHE_TABLE_SIZE   16  // 变量旧值缓存表最大支持数

/********************* 函数声明*********************/
/* LCD底层驱动函数 */
void Lcd_Init(void);
void Lcd_WriteIndex(u8 Index);
void Lcd_WriteData(u8 Data);
void LCD_WriteData_16Bit(u16 Data);
void Lcd_SetRegion(u16 x, u16 y, u16 x_end, u16 y_end);
void Lcd_SetXY(u16 x, u16 y);
void Gui_DrawPoint(u16 x, u16 y, u16 Data);
void Lcd_Clear(u16 Color);
void Lcd_ClearRegion(u16 x, u16 y, u16 w, u16 h, u16 Color);
void Gui_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color);

/* 显示相关函数*/
void lcd_show_str(u16 x, u16 y, u16 fc, u16 bc, const char *s);    // 显示常量字符串
u8  lcd_show_u32(u16 x, u16 y, u16 fc, u16 bc, u32 var);         // 显示无符号32位变量
u8  lcd_show_s32(u16 x, u16 y, u16 fc, u16 bc, s32 var);         // 显示有符号32位变量

#endif /* __LCD_H */
