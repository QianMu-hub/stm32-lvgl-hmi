#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"           // 如果使用 FreeRTOS
#include "queue.h"



#ifndef USER_TIM_H
#define USER_TIM_H

/**
 * @brief  初始化 TIM2 输入捕获（红外解码用）：PA0 复用为 TIM2_CH1，1us 计数步进、双边沿捕获并使能 TIM2 中断
 * @param  无
 * @retval 无
 */
void IR_Timer_Init(void);
/**
 * @brief  将 TIM2 初始化为 32 位自由运行计数器（不分频、周期拉满），供红外解码读取时间戳
 * @param  无
 * @retval 无
 */
void IndependentTimer_Init(void); 

#endif
