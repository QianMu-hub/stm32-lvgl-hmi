#if 0

#include "lvgl.h"
#include <stdlib.h>
#include "ESP8266.h"
#include "json_analysis.h"
#include "RTC.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "lvgl.h"
#include "GPIO_Driver.h"

#ifndef UI_FRAMEWORK_H
#define UI_FRAMEWORK_H

extern TaskHandle_t LVGL_task_handle;
void create_weather_queue(void);
void ui_create(void);
void LVGL_task(void *pvParameters);
#endif

#endif