#ifndef __UART_H_
#define __UART_H_
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef uart6;
/**
 * @brief  初始化 ESP8266 使用的 USART6：配置 PC6/PC7 为复用串口引脚，波特率 115200，并使能接收中断
 * @param  无
 * @retval 无
 */
void esp8266_uart_init(void);
#endif
