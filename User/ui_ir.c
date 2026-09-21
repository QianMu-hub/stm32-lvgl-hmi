#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
#include "ui_ir.h"
#include <stdlib.h>
#include <string.h>
#include "ESP8266.h"

#include "RTC.h"
#include "GPIO_Driver.h"   /* 提供 LL_GPIO_TogglePin（LVGL_task 内翻转 PB2） */
#include "lv_port_indev.h" /* 提供 lv_indev_set_ir_nav_cb() */
#include "city_config.h"   /* 城市列表，供 Settings 下拉框选择 */

TaskHandle_t LVGL_task_handle;

/* Get_Weather_Data() 定义在 ESP8266.c，但头文件未声明，这里补充声明 */
void Get_Weather_Data(void);

/**
 * @file ui_ir.c
 * @brief 红外遥控驱动的 UI 框架（240x320 竖屏）
 *
 * 保留原有功能（Home 天气页 / Settings 设置页 / 状态栏 / 日期时间显示），
 * 改用遥控器导航：
 *   - 左/右键  → 切换界面
 *   - 上/下键  → 界面内控件焦点切换
 *   - Enter    → 激活控件
 */

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

/* ---- 界面元素 ---- */
static lv_obj_t *ui_screen;
static lv_obj_t *ui_panel_home;
static lv_obj_t *ui_panel_settings;
static lv_obj_t *ui_status_label;
static lv_obj_t *label_uptime;

/* 标签栏（视觉指示当前页） */
static lv_obj_t *ui_tab_home;
static lv_obj_t *ui_tab_settings;

/* Home 页控件 */
static lv_obj_t *ui_btn_update;
static lv_obj_t *label_city;
static lv_obj_t *label_wheather;
static lv_obj_t *label_temp;
static lv_obj_t *label_date;
static lv_obj_t *label_time;

/* Settings 页控件 */
static lv_obj_t *ui_dropdown;

/* 分组：每个界面一个组，供上下键聚焦切换 */
static lv_group_t *group_home;
static lv_group_t *group_settings;

/* 当前界面：0=Home，1=Settings */
static uint8_t current_page = 0;

static void ui_create_screen(void);
static void ui_create_tab_bar(void);
static void ui_create_home(void);
static void ui_create_settings(void);
static void ui_create_status_bar(void);

static void switch_page(uint8_t page);
static lv_group_t *current_group(void);
static void ui_ir_page_left(void);
static void ui_ir_page_right(void);
static void ui_ir_up(void);
static void ui_ir_down(void);
static void apply_selected_city(void);
static void btn_event_dropdown(lv_event_t *e);
static void btn_event_wheather_update(lv_event_t *e);
static void put_wheather(void);
static void ui_datetime_timer_cb(lv_timer_t *timer);
static void ui_uptime_timer_cb(lv_timer_t *timer);

/* 关闭对象的滚动条并禁止滚动（LVGL 容器/标签/按钮默认可滚动，会显示滚动条） */
static void ui_disable_scroll(lv_obj_t *obj)
{
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

/* ------------------------------------------------------------------ */
/* 天气消息队列（由 create_weather_queue 创建，LVGL_task 消费）           */
/* ------------------------------------------------------------------ */
QueueHandle_t weather_queue;

#define WEATHER_QUEUE_LENGTH 2 // 队列长度，可根据需要调整

void create_weather_queue(void)
{
    weather_queue = xQueueCreate(WEATHER_QUEUE_LENGTH, sizeof(weather_msg_t));
    if (weather_queue == NULL)
    {
        /* 创建失败处理 */
    }
}

/* ------------------------------------------------------------------ */
/* 创建主屏                                                             */
/* ------------------------------------------------------------------ */
void ui_create(void)
{
    ui_create_screen();
    ui_create_tab_bar();
    ui_create_home();
    ui_create_settings();
    ui_create_status_bar();

    /* 每个界面一个分组，供 上/下(NEXT/PREV) 键在界面内切换控件焦点 */
    group_home = lv_group_create();
    group_settings = lv_group_create();
    lv_group_add_obj(group_home, ui_btn_update);
    lv_group_add_obj(group_settings, ui_dropdown);

    /* 注册遥控器方向键回调（在输入层拦截，不进入 LVGL 控件）：
     * 上/下：下拉框打开时改选项，否则在控件间切焦点；
     * 左/右：切换界面。 */
    lv_indev_set_ir_nav_cb(ui_ir_up, ui_ir_down, ui_ir_page_left, ui_ir_page_right);

    /* 初始显示 Home */
    switch_page(0);
}

static void ui_create_screen(void)
{
    ui_screen = lv_obj_create(NULL);
    lv_obj_set_size(ui_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_scrollbar_mode(ui_screen, LV_SCROLLBAR_MODE_OFF);
    lv_scr_load(ui_screen);
}

/* ------------------------------------------------------------------ */
/* 顶部标签栏（仅作当前页指示）                                          */
/* ------------------------------------------------------------------ */
static void ui_create_tab_bar(void)
{
    lv_obj_t *bar = lv_obj_create(ui_screen);
    lv_obj_set_size(bar, SCREEN_WIDTH, 40);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    /* 去掉标签栏的滚动条 */
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    ui_tab_home = lv_obj_create(bar);
    lv_obj_set_size(ui_tab_home, 90, 34);
    lv_obj_t *lb_home = lv_label_create(ui_tab_home);
    lv_label_set_text(lb_home, "Home");
    lv_obj_center(lb_home);
    ui_disable_scroll(ui_tab_home);
    ui_disable_scroll(lb_home);

    ui_tab_settings = lv_obj_create(bar);
    lv_obj_set_size(ui_tab_settings, 90, 34);
    lv_obj_t *lb_set = lv_label_create(ui_tab_settings);
    lv_label_set_text(lb_set, "Settings");
    lv_obj_center(lb_set);
    ui_disable_scroll(ui_tab_settings);
    ui_disable_scroll(lb_set);
}

/* ------------------------------------------------------------------ */
/* Home 界面（天气 + 日期时间）                                          */
/* ------------------------------------------------------------------ */
static void ui_create_home(void)
{
    ui_panel_home = lv_obj_create(ui_screen);
    lv_obj_set_size(ui_panel_home, SCREEN_WIDTH, SCREEN_HEIGHT - 40 - 44);
    lv_obj_set_pos(ui_panel_home, 0, 40);
    lv_obj_set_style_bg_color(ui_panel_home, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_pad_all(ui_panel_home, 0, 0);
    lv_obj_set_style_border_width(ui_panel_home, 0, 0);
    ui_disable_scroll(ui_panel_home);

    /* 日期 / 时间（顶部） */
    label_date = lv_label_create(ui_panel_home);
    lv_label_set_text(label_date, "----/--/--");
    lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_text_color(label_date, lv_color_hex(0x222222), 0);
    ui_disable_scroll(label_date);

    label_time = lv_label_create(ui_panel_home);
    lv_label_set_text(label_time, "--:--:--");
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0x000000), 0);
    ui_disable_scroll(label_time);

    /* 天气信息 */
    label_city = lv_label_create(ui_panel_home);
    lv_label_set_text(label_city, "City");
    lv_obj_align(label_city, LV_ALIGN_CENTER, 0, -50);
    ui_disable_scroll(label_city);

    label_wheather = lv_label_create(ui_panel_home);
    lv_label_set_text(label_wheather, "Weather");
    lv_obj_align(label_wheather, LV_ALIGN_CENTER, 0, -10);
    ui_disable_scroll(label_wheather);

    label_temp = lv_label_create(ui_panel_home);
    lv_label_set_text(label_temp, "--.- C");
    lv_obj_align(label_temp, LV_ALIGN_CENTER, 0, 30);
    ui_disable_scroll(label_temp);

    /* 更新天气按钮 */
    ui_btn_update = lv_btn_create(ui_panel_home);
    lv_obj_set_size(ui_btn_update, 160, 40);
    lv_obj_align(ui_btn_update, LV_ALIGN_CENTER, 0, 90);
    lv_obj_add_event_cb(ui_btn_update, btn_event_wheather_update, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(ui_btn_update);
    lv_label_set_text(btn_label, "Update Weather");
    lv_obj_center(btn_label);
    ui_disable_scroll(ui_btn_update);
    ui_disable_scroll(btn_label);

    /* 可见的焦点高亮 */
    lv_obj_set_style_border_width(ui_btn_update, 3, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(ui_btn_update, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_FOCUSED);

    /* 1 秒刷新日期时间 */
    lv_timer_create(ui_datetime_timer_cb, 1000, NULL);
}

/* ------------------------------------------------------------------ */
/* Settings 界面（下拉框 + 应用）                                       */
/* ------------------------------------------------------------------ */
static void ui_create_settings(void)
{
    ui_panel_settings = lv_obj_create(ui_screen);
    lv_obj_set_size(ui_panel_settings, SCREEN_WIDTH, SCREEN_HEIGHT - 40 - 44);
    lv_obj_set_pos(ui_panel_settings, 0, 40);
    lv_obj_set_style_bg_color(ui_panel_settings, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(ui_panel_settings, 0, 0);
    lv_obj_add_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN);
    ui_disable_scroll(ui_panel_settings);

    /* 下拉框标题（放在下拉框左侧） */
    lv_obj_t *dropdown_title = lv_label_create(ui_panel_settings);
    lv_label_set_text(dropdown_title, "Select City:");
    lv_obj_align(dropdown_title, LV_ALIGN_LEFT_MID, 2, -30);
    lv_obj_set_style_text_color(dropdown_title, lv_color_hex(0xFFFFFF), 0);
    ui_disable_scroll(dropdown_title);

    ui_dropdown = lv_dropdown_create(ui_panel_settings);
    /* 用城市英文名生成下拉选项（默认字体不支持中文，故用英文名） */
    static char s_city_options[512];
    {
        size_t pos = 0;
        uint16_t cnt = city_get_count();
        for (uint16_t i = 0; i < cnt && pos < sizeof(s_city_options) - 1; i++)
        {
            const city_item_t *item = city_get_item(i);
            const char *name = item ? item->query_name : "?";
            size_t len = strlen(name);
            if (pos + len + 1 >= sizeof(s_city_options))
                break;
            memcpy(s_city_options + pos, name, len);
            pos += len;
            if (i < cnt - 1)
                s_city_options[pos++] = '\n';
        }
        s_city_options[pos] = '\0';
    }
    lv_dropdown_set_options(ui_dropdown, s_city_options);
    lv_dropdown_set_selected(ui_dropdown, city_get_current_index());
    lv_obj_set_width(ui_dropdown, 120);
    lv_obj_align(ui_dropdown, LV_ALIGN_LEFT_MID, 90, -30);
    lv_obj_set_style_border_width(ui_dropdown, 3, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(ui_dropdown, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_FOCUSED);
    /* Enter 确认（关闭下拉框）后应用所选城市 */
    lv_obj_add_event_cb(ui_dropdown, btn_event_dropdown, LV_EVENT_RELEASED, NULL);
    ui_disable_scroll(ui_dropdown);
}

/* ------------------------------------------------------------------ */
/* 底部状态栏 + 运行时间统计                                             */
/* ------------------------------------------------------------------ */
static void ui_create_status_bar(void)
{
    ui_status_label = lv_label_create(ui_screen);
    lv_label_set_text(ui_status_label, "Status: Ready");
    lv_obj_align(ui_status_label, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_style_text_color(ui_status_label, lv_color_hex(0x333333), 0);
    ui_disable_scroll(ui_status_label);

    /* 运行时间统计标签（状态栏下方） */
    label_uptime = lv_label_create(ui_screen);
    lv_label_set_text(label_uptime, "Run: 00:00:00");
    lv_obj_align(label_uptime, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_style_text_color(label_uptime, lv_color_hex(0x555555), 0);
    ui_disable_scroll(label_uptime);

    /* 每秒刷新运行时间 */
    lv_timer_create(ui_uptime_timer_cb, 1000, NULL);
}

/* ------------------------------------------------------------------ */
/* 界面切换                                                            */
/* ------------------------------------------------------------------ */
static void switch_page(uint8_t page)
{
    lv_indev_t *indev = lv_indev_get_next(NULL); /* 当前唯一的 keypad 输入设备 */

    current_page = page;

    if (page == 0)
    {
        /* 离开 Settings 时关闭下拉框，避免它停在打开/编辑状态 */
        lv_dropdown_close(ui_dropdown);
        lv_obj_clear_flag(ui_panel_home, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN);

        /* 标签高亮 */
        lv_obj_set_style_bg_color(ui_tab_home, lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_obj_set_style_bg_color(ui_tab_settings, lv_color_hex(0xC8C8C8), 0);

        /* 切换到 Home 组并聚焦更新按钮 */
        lv_indev_set_group(indev, group_home);
        lv_group_focus_obj(ui_btn_update);
    }
    else
    {
        lv_obj_add_flag(ui_panel_home, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_style_bg_color(ui_tab_home, lv_color_hex(0xC8C8C8), 0);
        lv_obj_set_style_bg_color(ui_tab_settings, lv_palette_main(LV_PALETTE_BLUE), 0);

        /* 切换到 Settings 组并聚焦下拉框 */
        lv_indev_set_group(indev, group_settings);
        lv_group_focus_obj(ui_dropdown);
    }
}

/* ------------------------------------------------------------------ */
/* 遥控器方向键：界面切换 + 界面内导航（由输入层调用，不进入 LVGL 控件）  */
/* ------------------------------------------------------------------ */
static lv_group_t *current_group(void)
{
    return (current_page == 0) ? group_home : group_settings;
}

static void ui_ir_page_left(void)
{
    switch_page((current_page + 2 - 1) % 2); /* 上一页（两页时等价于切换） */
}

static void ui_ir_page_right(void)
{
    switch_page((current_page + 1) % 2);
}

/* 应用下拉框当前选中的城市：切换城市并触发天气更新 */
static void apply_selected_city(void)
{
    uint32_t sel = lv_dropdown_get_selected(ui_dropdown);
    if (city_set_by_index((uint16_t)sel) == 0)
    {
        const city_item_t *item = city_get_item((uint16_t)sel);
        lv_label_set_text_fmt(ui_status_label, "City: %s",
                              item ? item->query_name : "?");
        lv_label_set_text(label_city, "Updating...");
        lv_label_set_text(label_temp, "Updating...");
        lv_label_set_text(label_wheather, "Updating...");
        xTaskNotifyGive(Get_Weather_Data_Task_handle);
    }
}

/* 下拉框释放时：若刚被确认关闭（下拉框不再打开），则应用所选城市 */
static void btn_event_dropdown(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_RELEASED)
    {
        if (!lv_dropdown_is_open(ui_dropdown))
        {
            apply_selected_city();
        }
    }
}

/* 下：下拉框打开时选下一项，否则切到下一个控件 */
static void ui_ir_down(void)
{
    if (lv_dropdown_is_open(ui_dropdown))
    {
        uint32_t cur = lv_dropdown_get_selected(ui_dropdown);
        uint32_t cnt = lv_dropdown_get_option_count(ui_dropdown);
        lv_dropdown_set_selected(ui_dropdown, (cur + 1) % cnt);
    }
    else
    {
        lv_group_focus_next(current_group());
    }
}

/* 上：下拉框打开时选上一项，否则切到上一个控件 */
static void ui_ir_up(void)
{
    if (lv_dropdown_is_open(ui_dropdown))
    {
        uint32_t cur = lv_dropdown_get_selected(ui_dropdown);
        uint32_t cnt = lv_dropdown_get_option_count(ui_dropdown);
        lv_dropdown_set_selected(ui_dropdown, (cur + cnt - 1) % cnt);
    }
    else
    {
        lv_group_focus_prev(current_group());
    }
}

/* ------------------------------------------------------------------ */
/* 更新天气按钮                                                         */
/* ------------------------------------------------------------------ */
static void btn_event_wheather_update(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        //lv_label_set_text(label_city, "Updating...");
        lv_label_set_text(label_temp, "Updating...");
        lv_label_set_text(label_wheather, "Updating...");
        xTaskNotifyGive(Get_Weather_Data_Task_handle);
    }
}

/* ------------------------------------------------------------------ */
/* 显示天气                                                             */
/* ------------------------------------------------------------------ */
static void put_wheather(void)
{
    char *City = NULL;
    char *Desc = NULL;
    double Temp;
    parse_weather(&City, &Temp, &Desc);

    int temp_int = (int)(Temp * 10 + (Temp >= 0 ? 0.5 : -0.5)); // 四舍五入到一位小数
    char Temp_buffer[32];
    snprintf(Temp_buffer, sizeof(Temp_buffer), "%d.%d\xC2\xB0"
                                               "C",
             temp_int / 10, abs(temp_int % 10));

    if (City != NULL)
        lv_label_set_text(label_city, City);
    if (Desc != NULL)
        lv_label_set_text(label_wheather, Desc);
    lv_label_set_text(label_temp, Temp_buffer);

    vPortFree(City);
    vPortFree(Desc);
}

/* ------------------------------------------------------------------ */
/* 每秒刷新日期时间                                                     */
/* ------------------------------------------------------------------ */
static void ui_datetime_timer_cb(lv_timer_t *timer)
{
    RTC_DateTypeDef sDate = {0};
    RTC_TimeTypeDef sTime = {0};

    if (RTC_GetDateTime(&sDate, &sTime) != 0)
        return;

    const char *wd[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    uint8_t wd_idx = (sDate.WeekDay >= 1 && sDate.WeekDay <= 7) ? sDate.WeekDay : 0;

    lv_label_set_text_fmt(label_date, "%04u-%02u-%02u  %s",
                          2000U + sDate.Year, sDate.Month, sDate.Date, wd[wd_idx]);
    lv_label_set_text_fmt(label_time, "%02u:%02u:%02u",
                          sTime.Hours, sTime.Minutes, sTime.Seconds);
}

/* ------------------------------------------------------------------ */
/* 运行时间统计（开机/初始化后经过的时间）                                */
/* ------------------------------------------------------------------ */
static void ui_update_uptime(void)
{
    uint32_t sec = lv_tick_get() / 1000; /* 毫秒 → 秒 */
    uint32_t h = sec / 3600;
    uint32_t m = (sec % 3600) / 60;
    uint32_t s = sec % 60;
    lv_label_set_text_fmt(label_uptime, "Run: %02lu:%02lu:%02lu",
                          (unsigned long)h, (unsigned long)m, (unsigned long)s);
}

static void ui_uptime_timer_cb(lv_timer_t *timer)
{
    ui_update_uptime();
}

/* ------------------------------------------------------------------ */
/* LVGL 主任务：驱动渲染、消费天气队列                                  */
/* ------------------------------------------------------------------ */
void LVGL_task(void *pvParameters)
{
    static uint32_t last_tick = 0;
    uint32_t now = 0;
    while (1)
    {

        now = lv_tick_get();

        if ((uint32_t)(now - last_tick) >= 500)
        {
            LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
            last_tick = now;
        }

        weather_msg_t msg;
        if (xQueueReceive(weather_queue, &msg, 0) == pdPASS)
        {
            /* 更新 UI */
            /* 城市名用所选城市的英文名（city_config），
             * 不用 API 返回的名字（如 Ürümqi 的 Ü/ü 默认字体没有字形会显示成方框） */
            lv_label_set_text(label_city, city_get_query_name());
            lv_label_set_text(label_temp, msg.temp_str);
            lv_label_set_text(label_wheather, msg.desc);

            /* 释放消息中动态分配的内存 */
            vPortFree(msg.city);
            vPortFree(msg.desc);
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
