#ifndef __UART_H_
#define __UART_H_
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef uart6;
void esp8266_uart_init(void);
#endif
