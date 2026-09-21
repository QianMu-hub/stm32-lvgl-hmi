
/**
 * @file lv_port_indev_templ.h
 *
 */

/*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_TEMPL_H
#define LV_PORT_INDEV_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/


void lv_port_indev_init(void);
void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map);

/* 注册遥控器方向键回调（在输入层拦截，不进入 LVGL 控件）：
 * up/down 用于界面内导航（焦点切换/下拉框选项），left/right 用于界面切换 */
void lv_indev_set_ir_nav_cb(void (*up_cb)(void), void (*down_cb)(void),
                            void (*left_cb)(void), void (*right_cb)(void));


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_TEMPL_H*/

#endif /*Disable/Enable content*/
