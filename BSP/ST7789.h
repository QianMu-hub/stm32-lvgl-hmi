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

void ips200_init (void);
void img_send (const uint8_t* img);

void ips200_clear (void);
void ips200_full (const uint16_t color);
void ips200_color_full (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color);
void ips200_set_color (uint16_t pen, const uint16_t bgcolor);
void ips200_draw_point (uint16_t x, uint16_t y, const uint16_t color);
void ips200_draw_line (uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t color);
void ips200_show_char (uint16_t x, uint16_t y, const char dat);
void ips200_show_string (uint16_t x, uint16_t y, const char dat[]);
void ips200_show_int (uint16_t x, uint16_t y, const int32_t dat, uint8_t num);
void ips200_show_uint (uint16_t x, uint16_t y, const uint32_t dat, uint8_t num);
void ips200_show_float (uint16_t x, uint16_t y, const double dat, uint8_t num, uint8_t pointnum);
void ips200_fill (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *img);
void ips200_set_region (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


#endif
