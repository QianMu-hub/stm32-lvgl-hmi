#if 0
#include "ui_framework.h"

/**
 * @file ui_framework.c
 * @brief LVGL 简易 UI 框架 (240x320) - 修正版，强制固定布局
 */

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

static lv_obj_t *ui_screen;
static lv_obj_t *ui_btn_home;
// static lv_obj_t *ui_label_home;
static lv_obj_t *ui_dropdown_menu;
static lv_obj_t *ui_btn_settings;
static lv_obj_t *ui_status_label;

static lv_obj_t *label_city;
static lv_obj_t *label_wheather;
static lv_obj_t *label_temp;
static lv_obj_t *label_date;
static lv_obj_t *label_time;

static void ui_create_screen(void);
static void ui_create_tabview(void);
static void ui_create_status_bar(void);
// static void btn_event_cb(lv_event_t *e);
static void btn_event_wheather_update(lv_event_t *e);
static void ui_datetime_timer_cb(lv_timer_t *timer);

TaskHandle_t LVGL_task_handle;

void LVGL_task(void *pvParameters);
void Test_Task(void *pvParameters);
void Ir_Test_Task(void *pvParameters);

// static void add_to_default_group (lv_obj_t *n);
// void add_to_default_group (lv_obj_t *n)
//{
//	lv_group_t *g = lv_group_get_default();
//   if(g == NULL) {
//      g = lv_group_create();
//      lv_group_set_default(g);
//   }
//	lv_group_add_obj(g, n);
// }

void ui_create(void)
{
    ui_create_screen();
    ui_create_tabview();
    ui_create_status_bar();
}

static void ui_create_screen(void)
{
    ui_screen = lv_obj_create(NULL);
    lv_obj_set_size(ui_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0xF5F5F5), 0);
    lv_scr_load(ui_screen);
}

static lv_obj_t *ui_tab_bar;
static lv_obj_t *ui_content;
static lv_obj_t *ui_panel_home;
static lv_obj_t *ui_panel_settings;

static void tab_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    uint32_t tab_id = (uint32_t)lv_obj_get_user_data(btn);

    // 隐藏所有面板
    lv_obj_add_flag(ui_panel_home, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN);

    // 显示选中的面板
    if (tab_id == 0)
        lv_obj_clear_flag(ui_panel_home, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN);

    // 可选：改变按钮样式表示选中
}

static void ui_create_tabview(void)
{
    // ---- 标签栏（两个按钮）----
    ui_tab_bar = lv_obj_create(ui_screen);
    lv_obj_set_size(ui_tab_bar, SCREEN_WIDTH, 40);
    lv_obj_set_pos(ui_tab_bar, 0, 0);
    lv_obj_set_style_bg_color(ui_tab_bar, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_border_width(ui_tab_bar, 0, 0);
    lv_obj_set_flex_flow(ui_tab_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_tab_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 主页按钮
    lv_obj_t *btn_home = lv_btn_create(ui_tab_bar);
    lv_obj_set_size(btn_home, 80, 30);
    lv_obj_set_user_data(btn_home, (void *)0);
    lv_obj_add_event_cb(btn_home, tab_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Home");
    lv_obj_center(label_home);
    //		add_to_default_group(label_home);
    // 设置按钮
    lv_obj_t *btn_settings = lv_btn_create(ui_tab_bar);
    lv_obj_set_size(btn_settings, 80, 30);
    lv_obj_set_user_data(btn_settings, (void *)1);
    lv_obj_add_event_cb(btn_settings, tab_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, "Settings");
    lv_obj_center(label_settings);
    //		add_to_default_group(label_settings);
    // ---- 内容容器（占据剩余区域）----
    ui_content = lv_obj_create(ui_screen);

    lv_obj_set_size(ui_content, SCREEN_WIDTH, SCREEN_HEIGHT - 40 - 30); // 减去标签栏和状态栏
    lv_obj_set_pos(ui_content, 0, 40);
    lv_obj_set_style_bg_color(ui_content, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_pad_all(ui_content, 0, 0);
    lv_obj_set_style_border_width(ui_content, 0, 0);

    // ---- 主页面板 ----
    ui_panel_home = lv_obj_create(ui_content);
    lv_obj_set_size(ui_panel_home, SCREEN_WIDTH, SCREEN_HEIGHT - 40 - 30);
    lv_obj_set_pos(ui_panel_home, 0, 0);
    lv_obj_set_style_bg_color(ui_panel_home, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_border_width(ui_panel_home, 0, 0);
    // 在这里添加主页的控件（按钮等）
    ui_btn_home = lv_btn_create(ui_panel_home);
    lv_obj_set_size(ui_btn_home, 160, 40);
    lv_obj_align(ui_btn_home, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_event_cb(ui_btn_home, btn_event_wheather_update, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(ui_btn_home);
    lv_label_set_text(btn_label, "Click Me To Update");
    lv_obj_center(btn_label);
    //		lv_obj_add_event_cb(btn_label, btn_event_wheather_update, LV_EVENT_CLICKED, NULL);

    label_city = lv_label_create(ui_panel_home);
    lv_label_set_text(label_city, "label_city");
    lv_obj_align(label_city, LV_ALIGN_CENTER, 0, -30);

    label_wheather = lv_label_create(ui_panel_home);
    lv_label_set_text(label_wheather, "label_wheather ");
    lv_obj_align(label_wheather, LV_ALIGN_CENTER, 0, 0);

    label_temp = lv_label_create(ui_panel_home);
    lv_label_set_text(label_temp, "label_temp");
    lv_obj_align(label_temp, LV_ALIGN_CENTER, 0, 30);

    // ---- 日期 / 时间显示（主页顶部）----
    label_date = lv_label_create(ui_panel_home);
    lv_label_set_text(label_date, "----/--/--");
    lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_text_color(label_date, lv_color_hex(0x222222), 0);

    label_time = lv_label_create(ui_panel_home);
    lv_label_set_text(label_time, "--:--:--");
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0x000000), 0);

    // 每秒刷新一次日期时间
    lv_timer_create(ui_datetime_timer_cb, 1000, NULL);

    // ---- 设置面板 ----
    ui_panel_settings = lv_obj_create(ui_content);
    lv_obj_set_size(ui_panel_settings, SCREEN_WIDTH, SCREEN_HEIGHT - 40 - 30);
    lv_obj_set_pos(ui_panel_settings, 0, 0);
    lv_obj_set_style_bg_color(ui_panel_settings, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_border_width(ui_panel_settings, 0, 0);
    lv_obj_add_flag(ui_panel_settings, LV_OBJ_FLAG_HIDDEN); // 默认隐藏
    // 在这里添加设置页的控件
    ui_dropdown_menu = lv_dropdown_create(ui_panel_settings);
    lv_dropdown_set_options(ui_dropdown_menu, "Option 1\nOption 2\nOption 3\nOption 4");
    lv_obj_set_width(ui_dropdown_menu, 150);
    lv_obj_align(ui_dropdown_menu, LV_ALIGN_CENTER, 0, -30);
    ui_btn_settings = lv_btn_create(ui_panel_settings);
    lv_obj_set_size(ui_btn_settings, 100, 35);
    lv_obj_align(ui_btn_settings, LV_ALIGN_CENTER, 0, 30);
    //    lv_obj_add_event_cb(ui_btn_settings, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label2 = lv_label_create(ui_btn_settings);
    lv_label_set_text(btn_label2, "Apply");
    lv_obj_center(btn_label2);
}

static void ui_create_status_bar(void)
{
    ui_status_label = lv_label_create(ui_screen);
    lv_label_set_text(ui_status_label, "Status: Ready");
    lv_obj_align(ui_status_label, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_text_color(ui_status_label, lv_color_hex(0x333333), 0);
}

static void btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    if (btn == ui_btn_settings)
    {
        char buf[32];
        lv_dropdown_get_selected_str(ui_dropdown_menu, buf, sizeof(buf));
        lv_label_set_text_fmt(ui_status_label, "Selected: %s", buf);
    }
}

static void btn_event_wheather_update(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {

        lv_label_set_text(label_city, "Updating...");
        lv_label_set_text(label_temp, "Updating...");
        lv_label_set_text(label_wheather, "Updating...");
        xTaskNotifyGive(Get_Weather_Data_Task_handle);
    }
}

QueueHandle_t weather_queue;

#define WEATHER_QUEUE_LENGTH 2 // 队列长度，可根据需要调整

void create_weather_queue(void)
{
    weather_queue = xQueueCreate(WEATHER_QUEUE_LENGTH, sizeof(weather_msg_t));
    if (weather_queue == NULL)
    {
        // 处理创建失败
    }
}

/* 每秒刷新日期/时间显示 */
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

void LVGL_task(void *pvParameters)
{
    while (1)
    {
        static uint32_t last_tick = 0;
        uint32_t now = lv_tick_get();
        if (now != last_tick)
        {

            if (now % 1000 == 0)
                LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
            last_tick = now;
        }

        weather_msg_t msg;
        if (xQueueReceive(weather_queue, &msg, 0) == pdPASS)
        {
            // 更新 UI
            lv_label_set_text(label_city, msg.city);
            lv_label_set_text(label_temp, msg.temp_str);
            lv_label_set_text(label_wheather, msg.desc);

            // 释放消息中动态分配的内存
            free(msg.city);
            free(msg.desc);
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
#endif