/**
 * @file    ui_ir.h
 * @brief   红外遥控驱动的 UI 框架（240x320 竖屏）。
 *
 * 导航规则：
 *   - 遥控器 左/右 键（Left 0xe0 / Right 0x90）→ 在 Home / Settings 界面间切换
 *   - 遥控器 上/下 键（+/− 0x02/0x98，映射为 NEXT/PREV）→ 在界面内的可交互控件间切换焦点
 *   - 遥控器 Enter（Play 0xa8）→ 激活当前聚焦的控件
 */

#ifndef UI_IR_H
#define UI_IR_H

#include "json_analysis.h"

extern TaskHandle_t LVGL_task_handle;

/**
 * @brief  创建红外遥控 UI（Home 天气页、Settings 设置页、标签栏与状态栏），建立焦点分组并注册遥控方向键回调
 * @param  无
 * @retval 无
 */
void ui_create(void);
/**
 * @brief  创建天气消息队列 weather_queue，供天气任务与 LVGL 任务之间传递天气数据
 * @param  无
 * @retval 无
 */
void create_weather_queue(void);
/**
 * @brief  LVGL 主任务：循环刷新界面并消费天气队列更新显示，同时每 500ms 翻转 PB2 作为运行指示
 * @param  pvParameters  FreeRTOS 任务参数（未使用）
 * @retval 无
 */
void LVGL_task(void *pvParameters);




#endif /* UI_IR_H */
