#ifndef __ST7789_H_
#define __ST7789_H_
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_utils.h" 

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_spi.h"



#include <stdlib.h>
#include <string.h>
#include "font.h"

#include "main.h"
//---------------------------------------------------------------------------------------------------------------

#define IPS200_DEFAULT_DISPLAY_DIR      (IPS200_PORTAIT)                      	// 默认的显示方向
#define IPS200_DEFAULT_PENCOLOR         (RGB565_RED)                            // 默认的画笔颜色
#define IPS200_DEFAULT_BGCOLOR          (RGB565_BLACK)                          // 默认的背景颜色
#define IPS200_DEFAULT_DISPLAY_FONT     (IPS200_8X16_FONT)                      // 默认的字体模式
#define LCD_W 240																// 屏幕尺寸W
#define LCD_H 320																// 屏幕尺寸H


#define LCD_RES_GPIO_Port   GPIOA
#define LCD_RES_Pin         LL_GPIO_PIN_3

// 数据/命令引脚
#define LCD_DC_GPIO_Port    GPIOA
#define LCD_DC_Pin          LL_GPIO_PIN_4

// 片选引脚
#define LCD_CS_GPIO_Port    GPIOA
#define LCD_CS_Pin          LL_GPIO_PIN_6

// 为了方便，可以直接使用宏操作（注意前缀可能与你的代码不一致）
#define IPS200_RES(x)   ((x) ? LL_GPIO_SetOutputPin(LCD_RES_GPIO_Port, LCD_RES_Pin) : LL_GPIO_ResetOutputPin(LCD_RES_GPIO_Port, LCD_RES_Pin))
#define IPS200_DC(x)    ((x) ? LL_GPIO_SetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin)   : LL_GPIO_ResetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin))
#define IPS200_CS(x)    ((x) ? LL_GPIO_SetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin)   : LL_GPIO_ResetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin))







//#define IPS200_RES(x)					( (x) ? LL_GPIO_SetOutputPin(LCD_RES_GPIO_Port, LCD_RES_Pin) : LL_GPIO_ResetOutputPin(LCD_RES_GPIO_Port, LCD_RES_Pin) )
//#define IPS200_DC(x)					( (x) ? LL_GPIO_SetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin) :  LL_GPIO_ResetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin) )
//#define IPS200_CS(x)					( (x) ? LL_GPIO_SetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin) : LL_GPIO_ResetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin) )

//---------------------------------------------------------------------------------------------------------------
typedef enum
{
    IPS200_PORTAIT                      = 0,                                    // 竖屏模式
    IPS200_PORTAIT_180                  = 1,                                    // 竖屏模式  旋转180
    IPS200_CROSSWISE                    = 2,                                    // 横屏模式
    IPS200_CROSSWISE_180                = 3,                                    // 横屏模式  旋转180
}ips200_dir_enum;

typedef enum
{
    IPS200_6X8_FONT                     = 0,                                    // 6x8      字体
    IPS200_8X16_FONT                    = 1,                                    // 8x16     字体
    IPS200_16X16_FONT                   = 2,                                    // 16x16    字体 目前不支持
}ips200_font_size_enum;
//---------------------------------------------------------------------------------------------------------------

/**
 * @brief  初始化 ST7789 液晶屏：硬复位、下发初始化寄存器序列，并按默认方向宏设置显示方向后清屏
 * @param  无
 * @retval 无
 */
void ips200_init (void);
/**
 * @brief  发送一整幅图片到屏幕（数据前 2 字节为标志，第 3~4 字节为宽、第 5~6 字节为高，像素数据从第 9 字节开始）
 * @param  img  图片数据缓冲区指针（含文件头）
 * @retval 无
 */
void img_send (const uint8_t* img);

/**
 * @brief  用当前背景色清屏
 * @param  无
 * @retval 无
 */
void ips200_clear (void);
/**
 * @brief  用指定颜色填满整个屏幕
 * @param  color  RGB565 颜色值（可用 rgb565_color_enum 枚举）
 * @retval 无
 */
void ips200_full (const uint16_t color);
/**
 * @brief  用颜色缓冲区中的数据填充指定矩形区域
 * @param  x1     起始 x 坐标
 * @param  y1     起始 y 坐标
 * @param  x2     结束 x 坐标
 * @param  y2     结束 y 坐标
 * @param  color  颜色数据缓冲区（元素个数不少于区域像素数）
 * @retval 无
 */
void ips200_color_full (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color);
/**
 * @brief  设置字符显示使用的前景色（画笔色）和背景色，设置后立即生效
 * @param  pen      前景色（字体颜色），RGB565
 * @param  bgcolor  背景色，RGB565
 * @retval 无
 */
void ips200_set_color (uint16_t pen, const uint16_t bgcolor);
/**
 * @brief  在指定坐标画一个像素点
 * @param  x      像素 x 坐标
 * @param  y      像素 y 坐标
 * @param  color  RGB565 颜色值
 * @retval 无
 */
void ips200_draw_point (uint16_t x, uint16_t y, const uint16_t color);
/**
 * @brief  在两点之间画一条直线
 * @param  x_start  起点 x 坐标
 * @param  y_start  起点 y 坐标
 * @param  x_end    终点 x 坐标
 * @param  y_end    终点 y 坐标
 * @param  color    RGB565 颜色值
 * @retval 无
 */
void ips200_draw_line (uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t color);
/**
 * @brief  按当前字体设置在指定位置显示一个 ASCII 字符
 * @param  x    字符左上角 x 坐标
 * @param  y    字符左上角 y 坐标
 * @param  dat  要显示的 ASCII 字符
 * @retval 无
 */
void ips200_show_char (uint16_t x, uint16_t y, const char dat);
/**
 * @brief  按当前字体设置在指定位置显示以 '\0' 结尾的 ASCII 字符串
 * @param  x    字符串起始 x 坐标
 * @param  y    字符串起始 y 坐标
 * @param  dat  要显示的字符串
 * @retval 无
 */
void ips200_show_string (uint16_t x, uint16_t y, const char dat[]);
/**
 * @brief  在指定位置显示有符号整数（只取数值低 num 位，不显示无效前导零，负数带 '-' 号）
 * @param  x    显示起始 x 坐标
 * @param  y    显示起始 y 坐标
 * @param  dat  要显示的有符号整数
 * @param  num  显示位数，最大 10 位，不含符号位
 * @retval 无
 */
void ips200_show_int (uint16_t x, uint16_t y, const int32_t dat, uint8_t num);
/**
 * @brief  在指定位置显示无符号整数（只取数值低 num 位，不显示无效前导零）
 * @param  x    显示起始 x 坐标
 * @param  y    显示起始 y 坐标
 * @param  dat  要显示的无符号整数
 * @param  num  显示位数，最大 10 位
 * @retval 无
 */
void ips200_show_uint (uint16_t x, uint16_t y, const uint32_t dat, uint8_t num);
/**
 * @brief  在指定位置显示浮点数（整数部分只取低 num 位，小数部分显示 pointnum 位，负数带 '-' 号）
 * @param  x         显示起始 x 坐标
 * @param  y         显示起始 y 坐标
 * @param  dat       要显示的浮点数
 * @param  num       整数部分显示位数，最大 8 位
 * @param  pointnum  小数部分显示位数，最大 6 位
 * @retval 无
 */
void ips200_show_float (uint16_t x, uint16_t y, const double dat, uint8_t num, uint8_t pointnum);
/**
 * @brief  用颜色缓冲区中的数据填充指定矩形区域（与 ips200_color_full 功能相同）
 * @param  x1   起始 x 坐标
 * @param  y1   起始 y 坐标
 * @param  x2   结束 x 坐标
 * @param  y2   结束 y 坐标
 * @param  img  颜色数据缓冲区（元素个数不少于区域像素数）
 * @retval 无
 */
void ips200_fill (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *img);
/**
 * @brief  设置 LCD 写数据的显示窗口范围（内部调用，供各绘图/填充函数使用）
 * @param  x1  窗口起始 x 坐标
 * @param  y1  窗口起始 y 坐标
 * @param  x2  窗口结束 x 坐标
 * @param  y2  窗口结束 y 坐标
 * @retval 无
 */
void ips200_set_region (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


#endif
