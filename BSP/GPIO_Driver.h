#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "ST7789.h"
#ifndef GPIO_DRIVER_H
#define GIPO_DRIVER_H

/**
 * @brief  初始化 PB2 为推挽输出（在 LVGL 任务中用于定时翻转，作为运行指示）
 * @param  无
 * @retval 无
 */
void ur_GPIO_Init(void);
/**
 * @brief  初始化 SPI1（主机/8 位/软件 NSS/16 分频），并配置 LCD 的 SCK(PA5)、MOSI(PA7) 与 RES/DC/CS 控制引脚
 * @param  无
 * @retval 无
 */
void ur_SPI1_Init(void);
/**
 * @brief  红外接收头所用引脚的初始化函数（目前仅保留声明，工程中未提供实现；红外接收实际由 IRReceiver_Init 配置 PD15 外部中断）
 * @param  无
 * @retval 无
 */
void ur_GPIO_IR_Init(void);
/**
 * @brief  初始化 DMA2 数据流3，用于 SPI1 显示数据的存储器到外设传输，并使能传输完成中断
 * @param  无
 * @retval 无
 */
void ur_DMA_Init(void);


#endif
