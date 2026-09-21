/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/

#include <stdbool.h>
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "GPIO_Driver.h"
#include "ST7789.h"
#include "stm32f4xx_ll_dma.h"



/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
   // #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    240
#endif

#ifndef MY_DISP_VER_RES
   // #warning Please define or replace the macro MY_DISP_VER_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    320
#endif

#define BYTE_PER_PIXEL 2 /*will be 2 for RGB565 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
		lv_display_set_color_format(disp,LV_COLOR_FORMAT_RGB565_SWAPPED);


	
//    /* Example 1
//     * One buffer for partial rendering*/
//    LV_ATTRIBUTE_MEM_ALIGN
//    static uint8_t buf_1_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];            /*A buffer for 10 rows*/
//    lv_display_set_buffers(disp, buf_1_1, NULL, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Example 2
     * Two buffers for partial rendering
     * In flush_cb DMA or similar hardware should be used to update the display in the background.*/
    LV_ATTRIBUTE_MEM_ALIGN
    DISP_BUF volatile static uint8_t buf_2_1[MY_DISP_HOR_RES * 50 * BYTE_PER_PIXEL];
    LV_ATTRIBUTE_MEM_ALIGN
    DISP_BUF volatile static uint8_t buf_2_2[MY_DISP_HOR_RES * 50 * BYTE_PER_PIXEL];
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

//    /* Example 3
//     * Two buffers screen sized buffer for double buffering.
//     * Both LV_DISPLAY_RENDER_MODE_DIRECT and LV_DISPLAY_RENDER_MODE_FULL works, see their comments*/
//    LV_ATTRIBUTE_MEM_ALIGN
//    static uint8_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];

//    LV_ATTRIBUTE_MEM_ALIGN
//    static uint8_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];
//    lv_display_set_buffers(disp, buf_3_1, buf_3_2, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_DIRECT);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
 	ur_SPI1_Init();
	ips200_init ();
	ips200_clear();  
	/*You code here*/
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
	
void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{		
    if(disp_flush_enabled) 
		{	//disp_flush_enabled=false;  
            //LL_GPIO_TogglePin(GPIOB,LL_GPIO_PIN_2);
		
			uint32_t w = area->x2 - area->x1 +1;
			uint32_t h = area->y2 - area->y1 +1;

			// 设置屏幕窗口（如果需要，通过SPI发送命令）	
			LL_SPI_EnableDMAReq_TX(SPI1);
			IPS200_CS(0);
			ips200_set_region(area->x1,area->y1,area->x2,area->y2);	
			LL_DMA_SetMemoryAddress(DMA2, 3, (uint32_t)px_map);
			// SPI 为 8 位 + 字节 DMA，一个 RGB565 像素 = 2 字节，故长度 = w*h*2（字节数）
			LL_DMA_SetDataLength(DMA2, 3, w * h * 2);			
			LL_DMA_EnableStream(DMA2, 3);
			
			//ips200_fill(area->x1,area->y1,area->x2,area->y2,(uint16_t *)px_map);
		}

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
	//	lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
