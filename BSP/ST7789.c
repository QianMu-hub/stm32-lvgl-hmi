#include "ST7789.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_utils.h" 
#include "FreeRTOS.h"
#include "task.h"


#include "stm32f4xx.h" 
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_spi.h"



#define Delay_ms(x)                 HAL_Delay(x)

#define ips200_write_8bit_data_spi(data)                ( ST7789_WriteByte(data) )
#define ips200_write_16bit_data_spi(data)				( ST7789_Write2Byte(data) )

static	uint16_t						ips_show_h		= LCD_H;									// 屏幕高
static	uint16_t						ips_show_w		= LCD_W;									// 屏幕宽
static 	uint16_t                   	ips200_pencolor     = IPS200_DEFAULT_PENCOLOR;					// 画笔颜色(字体色)
static 	uint16_t                   	ips200_bgcolor      = IPS200_DEFAULT_BGCOLOR; 					// 背景颜色
static 	ips200_font_size_enum    	ips200_display_font = IPS200_DEFAULT_DISPLAY_FONT;      		// 显示字体类型

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     发送一个字节（根据平台自行实现）
// 参数说明     TxData           	发送字节
// 返回参数     void
// 使用示例     ST7789_WriteByte(0xff);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void ST7789_WriteByte(uint8_t TxData)
{
	LL_SPI_TransmitData8(SPI1, TxData);
	while(!LL_SPI_IsActiveFlag_TXE(SPI1));
    while(LL_SPI_IsActiveFlag_BSY(SPI1));
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     发送两个字节（根据平台自行实现）
// 参数说明     TxData           	发送字节
// 返回参数     void
// 使用示例     ST7789_Write2Byte(RGB565_BLACK);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void ST7789_Write2Byte(uint16_t TxData)
{		
	LL_SPI_TransmitData8(SPI1, (uint8_t)((TxData & 0xFF00) >> 8));
	while (!LL_SPI_IsActiveFlag_TXE(SPI1));
	LL_SPI_TransmitData8(SPI1, (uint8_t)(TxData & 0x00FF));
	while (!LL_SPI_IsActiveFlag_TXE(SPI1));
    while(LL_SPI_IsActiveFlag_BSY(SPI1));
}

//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 写命令
// 参数说明     command         命令
// 返回参数     void
// 使用示例     ips200_write_command(0x2a);
// 备注信息     内部调用 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static void ips200_write_command (const uint8_t command)
{
	IPS200_DC(0);
	ips200_write_8bit_data_spi(command);
	IPS200_DC(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     设置显示区域
// 参数说明     x1              起始x轴坐标
// 参数说明     y1              起始y轴坐标
// 参数说明     x2              结束x轴坐标
// 参数说明     y2              结束y轴坐标
// 返回参数     void
// 使用示例     ips200_set_region(0, 0, ips200_width_max - 1, ips200_height_max - 1);
// 备注信息     内部调用 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
 void ips200_set_region (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{		

    ips200_write_command(0x2a);
    ips200_write_16bit_data_spi(x1);
    ips200_write_16bit_data_spi(x2);
    
    ips200_write_command(0x2b);
    ips200_write_16bit_data_spi(y1);
    ips200_write_16bit_data_spi(y2);
    
    ips200_write_command(0x2c);
		
}

/**
 * @brief  从图片数据中解析出宽高并把整张图片写到屏幕左上角 (0,0) 起始的区域
 * @param  img   图片数据首地址 前 8 字节为图片头 第 2~3 字节为图片宽度 第 4~5 字节为图片高度 (均为高字节在前) 第 8 字节起为按行排列的 RGB565 像素数据 (每像素 2 字节 高字节在前)
 * @retval 无
 */
void img_send (const uint8_t* img)
{		uint32_t img_w=img[2]<<8|img[3];
		uint32_t img_h=img[4]<<8|img[5];
    uint32_t tag = 2*img_w * img_h;
    IPS200_CS(0);
    ips200_set_region(0, 0, img_w - 1, img_h - 1);
    for (uint32_t i=0; i <tag; i ++)
    {
     ST7789_WriteByte(img[i+8]);
    }
    IPS200_CS(1);
}

/**
 * @brief  用颜色缓冲区填充屏幕上指定的矩形区域 (与 ips200_color_full 功能相同)
 * @param  x1     矩形起始 x 像素坐标
 * @param  y1     矩形起始 y 像素坐标
 * @param  x2     矩形终止 x 像素坐标 需不小于 x1 否则区域宽高计算错误
 * @param  y2     矩形终止 y 像素坐标 需不小于 y1 否则区域宽高计算错误
 * @param  img    颜色缓冲区首地址 按行顺序存放 (x2-x1+1)*(y2-y1+1) 个 RGB565 颜色值
 * @retval 无
 */
void ips200_fill (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *img)
{
    uint32_t area = (x2 - x1 + 1) * (y2 - y1 + 1);
    IPS200_CS(0);
    ips200_set_region(x1, y1, x2, y2);
	for(int j = 0; (j < area); j++)
		ips200_write_16bit_data_spi(img[j]);
    IPS200_CS(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 清屏函数
// 参数说明     void
// 返回参数     void
// 使用示例     ips200_clear();
// 备注信息     将屏幕清空成背景颜色
//-------------------------------------------------------------------------------------------------------------------
void ips200_clear (void)
{
    uint32_t i = ips_show_w * ips_show_h;
    IPS200_CS(0);
    ips200_set_region(0, 0, ips_show_w - 1, ips_show_h - 1);
    for (; i != 0; i --)
    {
        ips200_write_16bit_data_spi(ips200_bgcolor);
    }
    IPS200_CS(1);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 屏幕填充函数
// 参数说明     color           颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     ips200_full(RGB565_BLACK);
// 备注信息     将屏幕填充成指定颜色
//-------------------------------------------------------------------------------------------------------------------
void ips200_full (const uint16_t color)
{
    uint32_t i = ips_show_w * ips_show_h;
    IPS200_CS(0);
    ips200_set_region(0, 0, ips_show_w - 1, ips_show_h - 1);
    for (; i != 0; i --)
    {
        ips200_write_16bit_data_spi(color);
    }
    IPS200_CS(1);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     区域填充函数
// 参数说明		x1				起始x坐标
// 参数说明		y1				起始y坐标
// 参数说明		x2				终止x坐标
// 参数说明		y2				终止y坐标
// 参数说明     color           颜色缓冲区
// 返回参数     void
// 使用示例     ips200_color_full(RGB565_YELLOW);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void ips200_color_full (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color)
{
    uint16_t i = (x2 - x1 + 1) * (y2 - y1 + 1);
    IPS200_CS(0);
    ips200_set_region(x1, y1, x2, y2);
	for(int j = 0; j < i; j++)
		ips200_write_16bit_data_spi(color[j]);
    IPS200_CS(1);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     设置显示颜色
// 参数说明     pen             颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 参数说明     bgcolor         颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     ips200_set_color(RGB565_RED, RGB565_GRAY);
// 备注信息     字体颜色和背景颜色也可以随时自由设置 设置后生效
//-------------------------------------------------------------------------------------------------------------------
void ips200_set_color (uint16_t pen, const uint16_t bgcolor)
{
    ips200_pencolor = pen;
    ips200_bgcolor = bgcolor;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 画点
// 参数说明     x               坐标x方向的起点 [0, ips200_width_max-1]
// 参数说明     y               坐标y方向的起点 [0, ips200_height_max-1]
// 参数说明     color           颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     ips200_draw_point(0, 0, RGB565_RED);            //坐标0,0画一个红色的点
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_draw_point (uint16_t x, uint16_t y, const uint16_t color)
{
    IPS200_CS(0);
    ips200_set_region(x, y, x, y);
    ips200_write_16bit_data_spi(color);
    IPS200_CS(1);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 画线
// 参数说明     x_start         坐标x方向的起点 [0, ips200_width_max-1]
// 参数说明     y_start         坐标y方向的起点 [0, ips200_height_max-1]
// 参数说明     x_end           坐标x方向的终点 [0, ips200_width_max-1]
// 参数说明     y_end           坐标y方向的终点 [0, ips200_height_max-1]
// 参数说明     color           颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     ips200_draw_line(0, 0, 10, 10, RGB565_RED);     // 坐标 0,0 到 10,10 画一条红色的线
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_draw_line (uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t color)
{
    uint16_t x_dir = (x_start < x_end ? 1 : -1);
    uint16_t y_dir = (y_start < y_end ? 1 : -1);
    float temp_rate = 0;
    float temp_b = 0;
    do
    {
        if(x_start != x_end)
        {
            temp_rate = (float)(y_start - y_end) / (float)(x_start - x_end);
            temp_b = (float)y_start - (float)x_start * temp_rate;
        }
        else
        {
            while(y_start != y_end)
            {
                ips200_draw_point(x_start, y_start, color);
                y_start += y_dir;
            }
            ips200_draw_point(x_start, y_start, color);
            break;
        }
        
        if(abs(y_start - y_end) > abs(x_start - x_end))
        {
            while(y_start != y_end)
            {
                ips200_draw_point(x_start, y_start, color);
                y_start += y_dir;
                x_start = (int16_t)(((float)y_start - temp_b) / temp_rate);
            }
            ips200_draw_point(x_start, y_start, color);
        }
        else
        {
            while(x_start != x_end)
            {
                ips200_draw_point(x_start, y_start, color);
                x_start += x_dir;
                y_start = (int16_t)((float)x_start * temp_rate + temp_b);
            }
            ips200_draw_point(x_start, y_start, color);
        }
    }while(0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 显示字符
// 参数说明     x               坐标x方向的起点 参数范围 [0, ips200_width_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, ips200_height_max-1]
// 参数说明     dat             需要显示的字符
// 返回参数     void
// 使用示例     ips200_show_char(0, 0, 'x');                     // 坐标0,0写一个字符x
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_show_char (uint16_t x, uint16_t y, const char dat)
{
    uint8_t i = 0, j = 0;
    IPS200_CS(0);
	switch(ips200_display_font)
	{
		case IPS200_6X8_FONT:
		ips200_set_region(x, y, x + 5, y + 7);
		for(i = 0; 8 > i; i ++)
		{
			for(j = 0; 6 > j; j ++)
			{		
				// 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
				uint8_t temp_top = ascii_font_6x8[dat - 32][j];
				if(temp_top & (0x01 << i))
				{
						ips200_write_16bit_data_spi(ips200_pencolor);
				}
				else
				{
						ips200_write_16bit_data_spi(ips200_bgcolor);
				}
				temp_top >>= 1;
			}
		}
		break;
		case IPS200_8X16_FONT:
		ips200_set_region(x, y, x + 7, y + 15);
		
		for(i = 0; 8 > i; i ++)
		{
			for(j = 0; 8 > j; j ++)
			{
				uint8_t temp_top = ascii_font_8x16[dat - 32][j];
				if(temp_top & (0x01 << i))
				{
						ips200_write_16bit_data_spi(ips200_pencolor);
				}
				else
				{
						ips200_write_16bit_data_spi(ips200_bgcolor);
				}
				temp_top >>= 1;
			}
		}
		for(i = 0; 8 > i; i ++)
		{
			for(j = 8; 16 > j; j ++)
			{
				uint8_t temp_bottom = ascii_font_8x16[dat - 32][j];
				if(temp_bottom & (0x01 << i))
				{
						ips200_write_16bit_data_spi(ips200_pencolor);
				}
				else
				{
						ips200_write_16bit_data_spi(ips200_bgcolor);
				}
				temp_bottom >>= 1;
			}
		}
		break;
		default:
		break;
	}
    IPS200_CS(1);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 显示字符串
// 参数说明     x               坐标x方向的起点 参数范围 [0, ips200_width_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, ips200_height_max-1]
// 参数说明     dat             需要显示的字符串
// 返回参数     void
// 使用示例     ips200_show_string(0, 0, "seekfree");
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_show_string (uint16_t x, uint16_t y, const char dat[])
{   
    uint16_t j = 0;
    while('\0' != dat[j])
    {
        switch(ips200_display_font)
        {
            case IPS200_6X8_FONT:   ips200_show_char(x + 6 * j, y, dat[j]); break;
            case IPS200_8X16_FONT:  ips200_show_char(x + 8 * j, y, dat[j]); break;
            case IPS200_16X16_FONT: break;                                      // 暂不支持
        }
        j ++;
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 显示32位有符号 (去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, ips200_width_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, ips200_height_max-1]
// 参数说明     dat             需要显示的变量 数据类型 int32
// 参数说明     num             需要显示的位数 最高10位  不包含正负号
// 返回参数     void
// 使用示例     ips200_show_int(0, 0, x, 3);                    // x 可以为 int32 int16 int8 类型
// 备注信息     负数会显示一个 ‘-’号
//-------------------------------------------------------------------------------------------------------------------
void ips200_show_int (uint16_t x, uint16_t y, const int32_t dat, uint8_t num)
{
    int32_t dat_temp = dat;
    int32_t offset = 1;
    char data_buffer[12];

    memset(data_buffer, 0, 12);
    memset(data_buffer, ' ', num + 1);

    // 用来计算余数显示 123 显示 2 位则应该显示 23
    if(10 > num)
    {
        for(; 0 < num; num --)
        {
            offset *= 10;
        }
        dat_temp %= offset;
    }
    func_int_to_str(data_buffer, dat_temp);
    ips200_show_string(x, y, (const char *)&data_buffer);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 显示32位无符号 (去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, ips114_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, ips114_y_max-1]
// 参数说明     dat             需要显示的变量 数据类型 uint32
// 参数说明     num             需要显示的位数 最高10位  不包含正负号
// 返回参数     void
// 使用示例     ips200_show_uint(0, 0, x, 3);                   // x 可以为 uint32 uint16 uint8 类型
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_show_uint (uint16_t x, uint16_t y, const uint32_t dat, uint8_t num)
{
    uint32_t dat_temp = dat;
    int32_t offset = 1;
    char data_buffer[12];
    memset(data_buffer, 0, 12);
    memset(data_buffer, ' ', num);

    // 用来计算余数显示 123 显示 2 位则应该显示 23
    if(10 > num)
    {
        for(; 0 < num; num --)
        {
            offset *= 10;
        }
        dat_temp %= offset;
    }
    func_uint_to_str(data_buffer, dat_temp);
    ips200_show_string(x, y, (const char *)&data_buffer);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     IPS200 显示浮点数(去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, ips200_width_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, ips200_height_max-1]
// 参数说明     dat             需要显示的变量 数据类型 double
// 参数说明     num             整数位显示长度   最高8位  
// 参数说明     pointnum        小数位显示长度   最高6位
// 返回参数     void
// 使用示例     ips200_show_float(0, 0, x, 2, 3);               // 显示浮点数   整数显示2位   小数显示三位
// 备注信息     特别注意当发现小数部分显示的值与你写入的值不一样的时候，
//              可能是由于浮点数精度丢失问题导致的，这并不是显示函数的问题，
//              有关问题的详情，请自行百度学习   浮点数精度丢失问题。
//              负数会显示一个 ‘-’号
//-------------------------------------------------------------------------------------------------------------------
void ips200_show_float (uint16_t x, uint16_t y, const double dat, uint8_t num, uint8_t pointnum)
{
    double dat_temp = dat;
    double offset = 1.0;
    char data_buffer[17];
    memset(data_buffer, 0, 17);
    memset(data_buffer, ' ', num + pointnum + 2);

    // 用来计算余数显示 123 显示 2 位则应该显示 23
    for(; 0 < num; num --)
    {
        offset *= 10;
    }
    dat_temp = dat_temp - ((int)dat_temp / (int)offset) * offset;
    func_double_to_str(data_buffer, dat_temp, pointnum);
    ips200_show_string(x, y, data_buffer);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     2寸 IPS液晶初始化
// 返回参数     void
// 使用示例     ips200_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void ips200_init (void)
{
    ips200_set_color(ips200_pencolor, ips200_bgcolor);

    IPS200_RES(0);
    Delay_ms(5);

    IPS200_RES(1);
    Delay_ms(120);
    IPS200_CS(0);

    ips200_write_command(0x11);
    Delay_ms(120);

    ips200_write_command(0x36);
    switch(IPS200_DEFAULT_DISPLAY_DIR)
    {
        case IPS200_PORTAIT:        
			ips200_write_8bit_data_spi(0x00);
			break;
        case IPS200_PORTAIT_180:    
			ips200_write_8bit_data_spi(0xC0);
			break;
        case IPS200_CROSSWISE:      
			ips200_write_8bit_data_spi(0x70);
			ips_show_h = LCD_W;
			ips_show_w = LCD_H;
			break;
        case IPS200_CROSSWISE_180:  
			ips200_write_8bit_data_spi(0xA0);
			ips_show_h = LCD_W;
			ips_show_w = LCD_H;
			break;
    }

    ips200_write_command(0x3A);
    ips200_write_8bit_data_spi(0x05);
    
    ips200_write_command(0xB2);
    ips200_write_8bit_data_spi(0x0C);
    ips200_write_8bit_data_spi(0x0C);
    ips200_write_8bit_data_spi(0x00);
    ips200_write_8bit_data_spi(0x33);
    ips200_write_8bit_data_spi(0x33);

    ips200_write_command(0xB7);
    ips200_write_8bit_data_spi(0x35);

    ips200_write_command(0xBB);
    ips200_write_8bit_data_spi(0x29);                                               // 32 Vcom=1.35V

    ips200_write_command(0xC2);
    ips200_write_8bit_data_spi(0x01);

    ips200_write_command(0xC3);
    ips200_write_8bit_data_spi(0x19);                                               // GVDD=4.8V 

    ips200_write_command(0xC4);
    ips200_write_8bit_data_spi(0x20);                                               // VDV, 0x20:0v

    ips200_write_command(0xC5);
    ips200_write_8bit_data_spi(0x1A);                                               // VCOM Offset Set

    ips200_write_command(0xC6);
    ips200_write_8bit_data_spi(0x01F);                                              // 0x0F:60Hz

    ips200_write_command(0xD0);
    ips200_write_8bit_data_spi(0xA4);
    ips200_write_8bit_data_spi(0xA1);
                
    ips200_write_command(0xE0);
    ips200_write_8bit_data_spi(0xD0);
    ips200_write_8bit_data_spi(0x08);
    ips200_write_8bit_data_spi(0x0E);
    ips200_write_8bit_data_spi(0x09);
    ips200_write_8bit_data_spi(0x09);
    ips200_write_8bit_data_spi(0x05);
    ips200_write_8bit_data_spi(0x31);
    ips200_write_8bit_data_spi(0x33);
    ips200_write_8bit_data_spi(0x48);
    ips200_write_8bit_data_spi(0x17);
    ips200_write_8bit_data_spi(0x14);
    ips200_write_8bit_data_spi(0x15);
    ips200_write_8bit_data_spi(0x31);
    ips200_write_8bit_data_spi(0x34);

    ips200_write_command(0xE1);  
    ips200_write_8bit_data_spi(0xD0);
    ips200_write_8bit_data_spi(0x08);
    ips200_write_8bit_data_spi(0x0E);
    ips200_write_8bit_data_spi(0x09);
    ips200_write_8bit_data_spi(0x09);
    ips200_write_8bit_data_spi(0x15);
    ips200_write_8bit_data_spi(0x31);
    ips200_write_8bit_data_spi(0x33);
    ips200_write_8bit_data_spi(0x48);
    ips200_write_8bit_data_spi(0x17);
    ips200_write_8bit_data_spi(0x14);
    ips200_write_8bit_data_spi(0x15);
    ips200_write_8bit_data_spi(0x31);
    ips200_write_8bit_data_spi(0x34);

    ips200_write_command(0x20);
    
    ips200_write_command(0x29);
		
		
		
		
		

	IPS200_CS(1);

    ips200_clear();                                                             // 初始化为黑屏
}
