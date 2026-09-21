#include "main.h"
#include "user_main.h"
/*******************************************************************************************************************************/
TaskHandle_t TaskHandel;
void lvgl_init(void);
void ESP8266_Init_Task(void *pvParameters);

/**
 * @brief  程序入口：初始化 HAL、系统时钟、GPIO/SPI/DMA 等外设与 LVGL，创建界面和天气队列后建立各任务并启动 FreeRTOS 调度器。
 * @param  无
 * @retval 无（vTaskStartScheduler() 正常不返回，末尾 while(1) 仅为兜底死循环）
 */
int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_Init();
  ur_GPIO_Init();
  ur_SPI1_Init();
  ur_DMA_Init();
  lvgl_init();

  RTC_Init();
 

  ui_create();
  create_weather_queue();

  xTaskCreate(LVGL_task, "LVGL", 2048, NULL, 3, &LVGL_task_handle);
  xTaskCreate(weather_task, "Weather_Task", 512, NULL, 2, &weather_task_handle);
  xTaskCreate(Get_Weather_Data_Task, "Get_Weather_Data_Task", 128, NULL, 2, &Get_Weather_Data_Task_handle);
  xTaskCreate(ESP8266_Init_Task,"ESP8266_Init_Task",128,NULL,2,NULL),
  vTaskStartScheduler();


  while (1)
  {
    /*错误处理*/
  }
}



/**
 * @brief  ESP8266 初始化任务：调用 ESP8266_Init() 复位模块、设置为 STA 模式并连接 WiFi，初始化完成后删除自身任务。
 * @param  pvParameters  任务参数（未使用，xTaskCreate 时传入 NULL）
 * @retval 无
 */
void ESP8266_Init_Task(void *pvParameters)
{

  ESP8266_Init();
  vTaskDelete(NULL);
}



/**
 * @brief  测试任务：死循环每 100ms 翻转一次 PB2 电平，用于验证任务调度与 GPIO 输出（当前未被 xTaskCreate 创建）。
 * @param  pvParameters  任务参数（未使用）
 * @retval 无
 */
void Test_Task(void *pvParameters)
{
  while (1)
  {
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


/**
 * @brief  初始化 LVGL：调用 lv_init()，并完成显示移植层与输入设备移植层的初始化。
 * @param  无
 * @retval 无
 */
void lvgl_init(void)
{
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
}
