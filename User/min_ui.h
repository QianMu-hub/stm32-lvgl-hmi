#if 0
#include "lvgl.h"
#include "ESP8266.h"
#include "FreeRTOS.h"
#include "queue.h"
#ifndef MIN_UI_H
#define MIN_UI_H


/**
 * @brief  创建简单演示界面：建立信息显示按钮与 "Do" 触发按钮，并加入默认焦点组以支持按键导航
 * @param  无
 * @retval 无
 */
void create_demo_ui(void);
/**
 * @brief  读取串口缓冲区数据并刷新界面信息标签（仅保留声明，当前实现已在 min_ui.c 中注释）
 * @param  无
 * @retval 无
 */
void Inf_Update(void);




#endif
#endif