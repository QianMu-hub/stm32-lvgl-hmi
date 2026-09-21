/**
 * @file lv_port_indev_template.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "driver_ir_receiver.h"
#include "GPIO_Driver.h"

/*********************
 *      DEFINES
 *********************/

/* 按键消抖窗口(ms)：同一按键在此窗口内重复出现则忽略，
 * 可抑制抖动/重复码导致的多次触发。 */
#define IR_DEBOUNCE_MS  200

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/


static void keypad_init(void);
static void keypad_read(lv_indev_t * indev, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);


/**********************
 *  STATIC VARIABLES
 **********************/

lv_indev_t * indev_keypad;

/* 遥控器方向键回调（在输入层拦截，不发给 LVGL 控件） */
static void (*ir_up_cb)(void)    = NULL;
static void (*ir_down_cb)(void)  = NULL;
static void (*ir_left_cb)(void)  = NULL;
static void (*ir_right_cb)(void) = NULL;

void lv_indev_set_ir_nav_cb(void (*up_cb)(void), void (*down_cb)(void),
                            void (*left_cb)(void), void (*right_cb)(void))
{
    ir_up_cb    = up_cb;
    ir_down_cb  = down_cb;
    ir_left_cb  = left_cb;
    ir_right_cb = right_cb;
}


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
 
    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    indev_keypad = lv_indev_create();
    lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keypad, keypad_read);


		/* 创建组并设置为默认组 */
		lv_group_t * group1 = lv_group_create();
		lv_group_set_default(group1);
		
		/* 将输入设备与组关联 */
		lv_indev_set_group(indev_keypad, group1);
		
    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/

}

/**********************
 *   STATIC FUNCTIONS
 **********************/



/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void)
{
    /*Your code comes here*/
		IRReceiver_Init();
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;
    static uint32_t last_act_key = 0;      /* 上次已接收的原始键(1-5) */
    static uint32_t last_accept_tick = 0;  /* 上次接收的时刻(ms) */

    /*Get the current x and y coordinates*/
   // mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        uint32_t now = lv_tick_get();

        /* 消抖：同一个键在去抖窗口内再次出现，视为抖动/重复，忽略 */
        if(act_key == last_act_key && (now - last_accept_tick) < IR_DEBOUNCE_MS) {
            data->state = LV_INDEV_STATE_RELEASED;
            data->key = last_key;
            return;
        }
        last_act_key     = act_key;
        last_accept_tick = now;

        data->state = LV_INDEV_STATE_PRESSED;

        /*Translate the keys to LVGL control characters according to your key definitions
         * 上/下(+/-)、左/右 都在输入层拦截，交给 UI 回调处理，不发给 LVGL，
         * 这样不会触发 LVGL 的 keypad 焦点/编辑逻辑。 */
        switch(act_key) {
            case 1: /* "-"：下 */
                if(ir_down_cb) ir_down_cb();
                act_key = 0;
                data->state = LV_INDEV_STATE_RELEASED;
                break;
            case 2: /* "+"：上 */
                if(ir_up_cb) ir_up_cb();
                act_key = 0;
                data->state = LV_INDEV_STATE_RELEASED;
                break;
            case 3: /* 左 */
                if(ir_left_cb) ir_left_cb();
                act_key = 0;
                data->state = LV_INDEV_STATE_RELEASED;
                break;
            case 4: /* 右 */
                if(ir_right_cb) ir_right_cb();
                act_key = 0;
                data->state = LV_INDEV_STATE_RELEASED;
                break;
            case 5:
                act_key = LV_KEY_ENTER;
                break;
        }

        last_key = act_key;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/
	

		uint8_t dev, data;
		static uint32_t last_valid_key = 0; 
		if (!IRReceiver_Read(&dev, &data))
		{
			switch (data)
        {
            case 0x98	:		last_valid_key =1;	return 1;
            case 0x02	:		last_valid_key =2;	return 2;
            case 0xe0	:		last_valid_key =3;	return 3;
            case 0x90	:		last_valid_key =4;	return 4;
            case 0xa8	:		last_valid_key =5;	return 5;
						case 0x00	:		return last_valid_key; 
            default		:	 	return 0;
        }					
		}	
		else 		
		return 0;	
}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
