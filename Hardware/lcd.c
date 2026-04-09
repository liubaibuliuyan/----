#include "lcd.h"


/********************* 全局变量 *********************/
extern SPI_HandleTypeDef hspi1;  // CubeMX生成的SPI句柄

/********************* 缓存表结构体：地址作为唯一标识 *********************/
typedef struct {
    const void *var_addr;  // 变量内存地址（唯一键值）
    u32 old_val;           // 无符号变量历史旧值
} U32_CacheItem;

typedef struct {
    const void *var_addr;  // 变量内存地址（唯一键值）
    s32 old_val;           // 有符号变量历史旧值
} S32_CacheItem;

/********************* 静态缓存表（函数内部自动管理） *********************/
static U32_CacheItem u32_cache_table[CACHE_TABLE_SIZE] = {0};
static S32_CacheItem s32_cache_table[CACHE_TABLE_SIZE] = {0};

/********************* 私有函数：缓存表查找/更新 *********************/
static u32 u32_cache_find_and_update(const void *addr, u32 new_val)
{
    for(u8 i=0; i<CACHE_TABLE_SIZE; i++)
    {
        if(u32_cache_table[i].var_addr == addr)
        {
            u32 old_val = u32_cache_table[i].old_val;
            u32_cache_table[i].old_val = new_val;
            return old_val;
        }
        if(u32_cache_table[i].var_addr == NULL)
        {
            u32_cache_table[i].var_addr = addr;
            u32_cache_table[i].old_val = new_val;
            return 0;
        }
    }
    return new_val;
}

static s32 s32_cache_find_and_update(const void *addr, s32 new_val)
{
    for(u8 i=0; i<CACHE_TABLE_SIZE; i++)
    {
        if(s32_cache_table[i].var_addr == addr)
        {
            s32 old_val = s32_cache_table[i].old_val;
            s32_cache_table[i].old_val = new_val;
            return old_val;
        }
        if(s32_cache_table[i].var_addr == NULL)
        {
            s32_cache_table[i].var_addr = addr;
            s32_cache_table[i].old_val = new_val;
            return 0;
        }
    }
    return new_val;
}

/********************* LCD底层驱动函数 *********************/
static void SPI_WriteData(u8 Data)
{
    HAL_SPI_Transmit(&hspi1, &Data, 1, 100);
}

static void Lcd_Reset(void)
{
    LCD_RST_CLR;
    HAL_Delay(100);
    LCD_RST_SET;
    HAL_Delay(50);
}

void LCD_GPIO_Init(void)
{
}

void Lcd_WriteIndex(u8 Index)
{
    LCD_CS_CLR;
    LCD_RS_CLR;
    SPI_WriteData(Index);
    LCD_CS_SET;
}

void Lcd_WriteData(u8 Data)
{
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI_WriteData(Data);
    LCD_CS_SET;
}

void LCD_WriteData_16Bit(u16 Data)
{
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI_WriteData(Data >> 8);
    SPI_WriteData(Data & 0xFF);
    LCD_CS_SET;
}

void Lcd_SetRegion(u16 x_start, u16 y_start, u16 x_end, u16 y_end)
{
    x_start = x_start > X_MAX_PIXEL-1 ? X_MAX_PIXEL-1 : x_start;
    y_start = y_start > Y_MAX_PIXEL-1 ? Y_MAX_PIXEL-1 : y_start;
    x_end   = x_end   > X_MAX_PIXEL-1 ? X_MAX_PIXEL-1 : x_end;
    y_end   = y_end   > Y_MAX_PIXEL-1 ? Y_MAX_PIXEL-1 : y_end;

    Lcd_WriteIndex(0x2A);
    Lcd_WriteData(x_start >> 8);Lcd_WriteData(x_start & 0xFF);
    Lcd_WriteData(x_end >> 8);Lcd_WriteData(x_end & 0xFF);

    Lcd_WriteIndex(0x2B);
    Lcd_WriteData(y_start >> 8);Lcd_WriteData(y_start & 0xFF);
    Lcd_WriteData(y_end >> 8);Lcd_WriteData(y_end & 0xFF);

    Lcd_WriteIndex(0x2C);
}

void Lcd_SetXY(u16 x, u16 y)
{
    Lcd_SetRegion(x, y, x, y);
}

void Gui_DrawPoint(u16 x, u16 y, u16 Data)
{
    if(x < X_MAX_PIXEL && y < Y_MAX_PIXEL)
    {
        Lcd_SetXY(x, y);
        LCD_WriteData_16Bit(Data);
    }
}

void Lcd_Clear(u16 Color)
{
    u32 i, total_pixel = (u32)X_MAX_PIXEL * Y_MAX_PIXEL;
    Lcd_SetRegion(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
    for(i=0; i<total_pixel; i++) LCD_WriteData_16Bit(Color);
}

void Lcd_ClearRegion(u16 x, u16 y, u16 w, u16 h, u16 Color)
{
    u32 i, pixel_num = (u32)w * h;
    Lcd_SetRegion(x, y, x+w-1, y+h-1);
    for(i=0; i<pixel_num; i++) LCD_WriteData_16Bit(Color);
}

void Lcd_Init(void)
{
    LCD_GPIO_Init();
    Lcd_Reset();

    Lcd_WriteIndex(0x11);HAL_Delay(120);
    Lcd_WriteIndex(0xB1);Lcd_WriteData(0x05);Lcd_WriteData(0x3C);Lcd_WriteData(0x3C);
    Lcd_WriteIndex(0xB2);Lcd_WriteData(0x05);Lcd_WriteData(0x3C);Lcd_WriteData(0x3C);
    Lcd_WriteIndex(0xB3);Lcd_WriteData(0x05);Lcd_WriteData(0x3C);Lcd_WriteData(0x3C);Lcd_WriteData(0x05);Lcd_WriteData(0x3C);Lcd_WriteData(0x3C);
    Lcd_WriteIndex(0xB4);Lcd_WriteData(0x03);
    Lcd_WriteIndex(0xC0);Lcd_WriteData(0x28);Lcd_WriteData(0x08);Lcd_WriteData(0x04);
    Lcd_WriteIndex(0xC1);Lcd_WriteData(0xC0);
    Lcd_WriteIndex(0xC2);Lcd_WriteData(0x0D);Lcd_WriteData(0x00);
    Lcd_WriteIndex(0xC3);Lcd_WriteData(0x8D);Lcd_WriteData(0x2A);
    Lcd_WriteIndex(0xC4);Lcd_WriteData(0x8D);Lcd_WriteData(0xEE);
    Lcd_WriteIndex(0xC5);Lcd_WriteData(0x1A);
    Lcd_WriteIndex(0x00);Lcd_WriteData(0xA0);
    Lcd_WriteIndex(0xE0);Lcd_WriteData(0x04);Lcd_WriteData(0x22);Lcd_WriteData(0x07);Lcd_WriteData(0x0A);Lcd_WriteData(0x2E);Lcd_WriteData(0x30);Lcd_WriteData(0x25);Lcd_WriteData(0x2A);Lcd_WriteData(0x28);Lcd_WriteData(0x26);Lcd_WriteData(0x2E);Lcd_WriteData(0x3A);Lcd_WriteData(0x00);Lcd_WriteData(0x01);Lcd_WriteData(0x03);Lcd_WriteData(0x13);
    Lcd_WriteIndex(0xE1);Lcd_WriteData(0x04);Lcd_WriteData(0x16);Lcd_WriteData(0x06);Lcd_WriteData(0x0D);Lcd_WriteData(0x2D);Lcd_WriteData(0x26);Lcd_WriteData(0x23);Lcd_WriteData(0x2B);Lcd_WriteData(0x28);Lcd_WriteData(0x26);Lcd_WriteData(0x37);Lcd_WriteData(0x3C);Lcd_WriteData(0x00);Lcd_WriteData(0x01);Lcd_WriteData(0x03);Lcd_WriteData(0x13);
    Lcd_WriteIndex(0x3A);Lcd_WriteData(0x05);
    Lcd_WriteIndex(0x29);HAL_Delay(100);

    Lcd_Clear(BLACK);
}

void Gui_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 Color)
{
    int dx = x1-x0, dy = y1-y0;
    int x_inc = dx>=0?1:-1, y_inc = dy>=0?1:-1;
    dx = dx<0?-dx:dx; dy = dy<0?-dy:dy;
    int dx2=dx<<1, dy2=dy<<1, error = dy2-dx;

    if(dx>dy)
    {
        for(int i=0; i<=dx; i++)
        {
            Gui_DrawPoint(x0, y0, Color);
            if(error>=0){error-=dx2; y0+=y_inc;}
            error+=dy2; x0+=x_inc;
        }
    }
    else
    {
        error = dx2 - dy;
        for(int i=0; i<=dy; i++)
        {
            Gui_DrawPoint(x0, y0, Color);
            if(error>=0){error-=dy2; x0+=x_inc;}
            error+=dx2; y0+=y_inc;
        }
    }
}

/********************* 显示核心底层函数 *********************/
static void lcd_draw_char(u16 x, u16 y, u16 fc, u16 bc, u8 ch)
{
    ch = (ch>=0x20 && ch<=0x7E) ? (ch-0x20) : 0;
    for(u8 i=0; i<ASCII16_HEIGHT; i++)
    {
        for(u8 j=0; j<ASCII16_WIDTH; j++)
        {
            if(asc16[ch*ASCII16_HEIGHT + i] & (0x80 >> j))
            {
                Gui_DrawPoint(x+j, y+i, fc);
            }
            else if(fc != bc)
            {
                Gui_DrawPoint(x+j, y+i, bc);
            }
        }
    }
}

static void var_format_fixed(u8 *buf, u32 buf_len, u8 is_signed, s32 val)
{
    if(buf == NULL || buf_len < VAR_DEFAULT_DIGITS+1) return;
    if(is_signed)
    {
        sprintf((char*)buf, "%-*d", VAR_DEFAULT_DIGITS, val);
    }
    else
    {
        sprintf((char*)buf, "%-*ld", VAR_DEFAULT_DIGITS, (u32)val);
    }
}

/********************* 函数定义 *********************/
// 显示常量字符串：lcd_show_str(x, y, 前景色, 背景色, 字符串)
void lcd_show_str(u16 x, u16 y, u16 fc, u16 bc, const char *s)
{
    if(s == NULL) return;
    u16 curr_x = x;
    while(*s != '\0')
    {
        if(curr_x + ASCII16_WIDTH > X_MAX_PIXEL || y + ASCII16_HEIGHT > Y_MAX_PIXEL) break;
        lcd_draw_char(curr_x, y, fc, bc, (u8)*s);
        curr_x += ASCII16_WIDTH;
        s++;
    }
}

// 显示无符号32位变量：lcd_show_u32(x, y, 前景色, 背景色, 无符号变量)
u8 lcd_show_u32(u16 x, u16 y, u16 fc, u16 bc, u32 var)
{
    u32 old_val = u32_cache_find_and_update(&var, var);
    if(old_val == var) return 0;

    u8 lcd_buf[VAR_DEFAULT_DIGITS+1] = {0};
    u16 clear_w = VAR_DEFAULT_DIGITS * ASCII16_WIDTH;
    Lcd_ClearRegion(x, y, clear_w, ASCII16_HEIGHT, bc);
    var_format_fixed(lcd_buf, sizeof(lcd_buf), 0, (s32)var);
    lcd_show_str(x, y, fc, bc, (const char*)lcd_buf);

    return 1;
}

// 显示有符号32位变量：lcd_show_s32(x, y, 前景色, 背景色, 有符号变量)
u8 lcd_show_s32(u16 x, u16 y, u16 fc, u16 bc, s32 var)
{
    s32 old_val = s32_cache_find_and_update(&var, var);
    if(old_val == var) return 0;

    u8 lcd_buf[VAR_DEFAULT_DIGITS+1] = {0};
    u16 clear_w = VAR_DEFAULT_DIGITS * ASCII16_WIDTH;
    Lcd_ClearRegion(x, y, clear_w, ASCII16_HEIGHT, bc);
    var_format_fixed(lcd_buf, sizeof(lcd_buf), 1, var);
    lcd_show_str(x, y, fc, bc, (const char*)lcd_buf);

    return 1;
}
