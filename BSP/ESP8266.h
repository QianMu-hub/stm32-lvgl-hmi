#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "UART.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"

#ifndef ESP8266_H
#define ESP8266_H

#define NEXT_POS(x) ((x + 1) % BUF_LEN)

extern uint32_t time_start_time;
extern uint32_t time_end_time;
extern UART_HandleTypeDef uart6;
extern TaskHandle_t Get_Weather_Data_Task_handle;
extern TaskHandle_t weather_task_handle;
uint8_t Data_Read_all(char *data);
void Data_Read_line(char *data);
void ESP8266_Test(void);
void ESP8266_Init(void);
void Get_Weather_Data_Task(void *pvParameters);

#endif
