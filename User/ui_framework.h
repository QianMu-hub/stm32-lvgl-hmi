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
/**
 * @brief  创建天气消息队列 weather_queue，供天气任务与 LVGL 任务之间传递天气数据
 * @param  无
 * @retval 无
 */
void create_weather_queue(void);
/**
 * @brief  创建界面框架：建立主屏、Home/Settings 标签页与状态栏（本文件整体处于 #if 0 屏蔽状态）
 * @param  无
 * @retval 无
 */
void ui_create(void);
/**
 * @brief  LVGL 主任务：循环调用 lv_timer_handler 刷新界面、消费天气队列更新标签，并周期性翻转 PB2
 * @param  pvParameters  FreeRTOS 任务参数（未使用）
 * @retval 无
 */
void LVGL_task(void *pvParameters);
#endif

#endif