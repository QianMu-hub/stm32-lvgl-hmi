#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"           // 如果使用 FreeRTOS
#include "queue.h"



#ifndef USER_TIM_H
#define USER_TIM_H

void IR_Timer_Init(void);
void IndependentTimer_Init(void); 

#endif
