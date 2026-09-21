#include <stdlib.h>
#include "lvgl.h"
#include "min_ui.h"
#include "ESP8266.h"
#include "json_analysis.h"

//static lv_obj_t *info_label = NULL;
/* 按钮点击事件回调 */
static void btn_event_wifi_sc(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        //static uint8_t cnt = 0;
//				ESP8266_Test();
				  Get_Weather_Data();
    }
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
		char *City= NULL;
		char *Desc=	NULL;
		double Temp;
		parse_weather(&City, &Temp, &Desc);
    lv_label_set_text(lv_obj_get_child(btn, 0), City);
		free(City);
    }
}

/* 创建简单UI */
void create_demo_ui(void)
{
    /* 获取默认组（group1），确保输入设备已关联该组 */
    lv_group_t * g = lv_group_get_default();
    if(g == NULL) {
        g = lv_group_create();
        lv_group_set_default(g);
    }

    /* 创建一个容器作为背景，便于布局 */
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* 创建i个按钮，并添加到组中 */
    const char * btn_labels[] = {"AP List"};
    for(int i = 0; i < 1; i++) {
        lv_obj_t * btn = lv_btn_create(scr);
        lv_obj_set_width(btn, 240);
        lv_obj_set_height(btn, 280);
//				lv_obj_set_scroll_dir(btn, LV_DIR_VER);         // 只允许垂直滚动
//				lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_AUTO); // 根据需要自动显示滚动条

			
			
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * label = lv_label_create(btn);
				lv_obj_set_width(label, 220); 
				lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
//				lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);   // 滚动模式
        lv_label_set_text(label, btn_labels[i]);
        lv_obj_center(label);

        /* 将按钮添加到默认组，以便键盘导航 */
        lv_group_add_obj(g, btn);
    }

		const char * btn_label[] = {"Do"};
    for(int i = 0; i < 1; i++) {
        lv_obj_t * btn1 = lv_btn_create(scr);
        lv_obj_set_width(btn1, 240);
        lv_obj_set_height(btn1, 20);
        lv_obj_set_style_bg_color(btn1, lv_palette_main(LV_PALETTE_BLUE), 0);
        lv_obj_add_event_cb(btn1, btn_event_wifi_sc, LV_EVENT_CLICKED, NULL);

        lv_obj_t * label1 = lv_label_create(btn1);
				lv_obj_set_width(label1, 220); 
//				lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP); 
        lv_label_set_text(label1, btn_label[i]);
//        lv_obj_center(label1);

        /* 将按钮添加到默认组，以便键盘导航 */
        lv_group_add_obj(g, btn1);
    }
		
    /* 创建一个提示标签，显示当前焦点或按键状态 */
//    lv_obj_t * info_label = lv_label_create(scr);
//    lv_label_set_text(info_label, "NULL");
//    lv_obj_set_style_text_color(info_label, lv_palette_main(LV_PALETTE_GREY), 0);
   // lv_group_add_obj(g, info_label);  /* 标签也可以获得焦点，但通常不需要；这里仅演示 */

    /* 可选：设置第一个按钮为初始焦点 */
    lv_group_focus_obj(lv_obj_get_child(scr, 0));  /* 第一个按钮 */
}

//void Inf_Update(void)
//{   
//	char *data;
//	Data_Read(data);
//	lv_label_set_text(info_label, data);
//}
