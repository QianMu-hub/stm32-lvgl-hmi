#include "main.h"
#include "user_main.h"
/*******************************************************************************************************************************/
TaskHandle_t TaskHandel;

/*******************************************************************************************************************************/

/*******************************************************************************************************************************/

void lvgl_init(void);
void ESP8266_Init_Task(void *pvParameters);
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

  /* We should never get here as control is now taken by the scheduler */
  while (1)
  {
  }
}



/*******************************************************************************************************************************/

void ESP8266_Init_Task(void *pvParameters)
{

  ESP8266_Init();
  vTaskDelete(NULL);
}

/*******************************************************************************************************************************/

void Test_Task(void *pvParameters)
{
  while (1)
  {
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/*******************************************************************************************************************************/



/*******************************************************************************************************************************/

void lvgl_init(void)
{
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
}
