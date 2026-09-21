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

void parse_weather(char **city, double *temp, char **desc);
void weather_task(void *pvParameters);
#endif
