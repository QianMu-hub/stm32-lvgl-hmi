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

void ui_create(void);
void create_weather_queue(void);
void LVGL_task(void *pvParameters);




#endif /* UI_IR_H */
