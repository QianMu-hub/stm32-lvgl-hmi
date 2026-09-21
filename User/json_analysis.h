#include <stdio.h>
#include "cJSON.h"
#include <string.h>
#include "ESP8266.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdlib.h>
#ifndef JSON_ANALYSIS_H
#define JSON_ANALYSIS_H
extern QueueHandle_t weather_queue;

typedef struct
{
    char *city;
    char *desc;
    char temp_str[16]; // 存储格式化后的温度字符串
} weather_msg_t;

/**
 * @brief  读取 ESP8266 的 HTTP 响应，解析出城市名、温度与天气描述，并用响应头的日期同步 RTC
 * @param  city  输出：动态分配的城市名首地址（调用方负责释放；无数据时保持原值）
 * @param  temp  输出：温度值（摄氏度）
 * @param  desc  输出：动态分配的天气描述首地址（调用方负责释放；无数据时保持原值）
 * @retval 无
 */
void parse_weather(char **city, double *temp, char **desc);
/**
 * @brief  天气处理任务：等待通知后解析天气数据，格式化温度为字符串并发送到天气队列供 UI 显示
 * @param  pvParameters  FreeRTOS 任务参数（未使用）
 * @retval 无
 */
void weather_task(void *pvParameters);
#endif
