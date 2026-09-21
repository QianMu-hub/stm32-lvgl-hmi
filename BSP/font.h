#ifndef __FONT_H_
#define __FONT_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//-------常用颜色----------
typedef enum
{
    RGB565_WHITE    = (0xFFFF),                                                     // 白色
    RGB565_BLACK    = (0x0000),                                                     // 黑色
    RGB565_BLUE     = (0x001F),                                                     // 蓝色
    RGB565_PURPLE   = (0xF81F),                                                     // 紫色
    RGB565_PINK     = (0xFE19),                                                     // 粉色
    RGB565_RED      = (0xF800),                                                     // 红色
    RGB565_MAGENTA  = (0xF81F),                                                     // 品红
    RGB565_GREEN    = (0x07E0),                                                     // 绿色
    RGB565_CYAN     = (0x07FF),                                                     // 青色
    RGB565_YELLOW   = (0xFFE0),                                                     // 黄色
    RGB565_BROWN    = (0xBC40),                                                     // 棕色
    RGB565_GRAY     = (0x8430),                                                     // 灰色

    RGB565_39C5BB   = (0x3616),
    RGB565_66CCFF   = (0x665F),
}rgb565_color_enum;

extern const uint8_t      ascii_font_8x16[][16];
extern const uint8_t      ascii_font_6x8[][6];

/**
 * @brief  将有符号整数按十进制转换为字符串（自动带 '-' 号，不追加结束符）
 * @param  str     输出字符串缓冲区，由调用方保证足够长度并预先初始化
 * @param  number  需要转换的有符号整数
 * @retval 无
 */
void func_int_to_str (char *str, int32_t number);
/**
 * @brief  将无符号整数按十进制转换为字符串（不追加结束符）
 * @param  str     输出字符串缓冲区，由调用方保证足够长度并预先初始化
 * @param  number  需要转换的无符号整数
 * @retval 无
 */
void func_uint_to_str (char *str, uint32_t number);
/**
 * @brief  将浮点数转换为字符串：写入完整的整数部分，并在 point_bit 大于 0 时写出小数点与小数部分（不追加结束符）
 * @param  str        输出字符串缓冲区，由调用方保证足够长度并预先初始化
 * @param  number     需要转换的浮点数
 * @param  point_bit  保留的小数位数，为 0 时不输出小数点
 * @retval 无
 */
void func_double_to_str (char *str, double number, uint8_t point_bit);


#endif
