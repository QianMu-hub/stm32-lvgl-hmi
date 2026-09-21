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
extern volatile uint8_t is_esp8266_ready;

/**
 * @brief  把接收环形缓冲区中的数据全部读出到 data（最多 1023 字节，自动补 '\0'）
 * @param  data  接收数据的输出缓冲区，容量需不小于 1024 字节
 * @retval 1 表示读到数据；0 表示缓冲区为空（此时 data[0] 被置为 '\0'）
 */
uint8_t Data_Read_all(char *data);
/**
 * @brief  从接收环形缓冲区中按行读取一条以 "\r\n" 结尾的数据行（去掉首尾控制字符）+ '\0'
 * @param  data  存放读出数据行的输出缓冲区
 * @retval 无
 */
void Data_Read_line(char *data);
/**
 * @brief  发送 AT+GMR 查询模块固件版本，并启动单字节中断接收（用于模块连通性测试）
 * @param  无
 * @retval 无
 */
void ESP8266_Test(void);
/**
 * @brief  初始化 ESP8266：配置 USART6、复位模块、设置为 STA 模式并连接 WiFi，成功后置位就绪标志
 * @param  无
 * @retval 无
 */
void ESP8266_Init(void);
/**
 * @brief  ESP8266 天气获取任务：启动后先获取一次天气，之后每 10 分钟被唤醒一次刷新天气数据
 * @param  pvParameters  FreeRTOS 任务参数（未使用）
 * @retval 无
 */
void Get_Weather_Data_Task(void *pvParameters);

#endif
