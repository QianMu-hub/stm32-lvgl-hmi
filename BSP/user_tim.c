          
#include "user_tim.h"   
#include "tim.h"


 TIM_HandleTypeDef htim2;  
 void IndependentTimer_Init(void)
 {
     __HAL_RCC_TIM2_CLK_ENABLE();  

     htim2.Instance = TIM2;
     htim2.Init.Prescaler = 0;                                  
     htim2.Init.CounterMode = TIM_COUNTERMODE_UP;               
     htim2.Init.Period = 0xFFFFFFFF;                            
     htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;          
     htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; 

     HAL_TIM_Base_Init(&htim2);
     HAL_TIM_Base_Start(&htim2);  


 }

 // 可在 main.c 或其他地方定义

// 定义用于传递捕获值的队列句柄（若使用 FreeRTOS）
//extern QueueHandle_t xIRCaptureQueue;
TIM_HandleTypeDef        htim2;
/**
  * @brief  TIM2 输入捕获初始化（用于红外解码）
  * @param  无
  * @retval 无
  */
void IR_Timer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    // 1. 使能 TIM2 和 GPIOA 时钟
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. 配置 PA0 为复用功能，推挽输出，上拉（根据红外接收头极性可选）
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;         // 红外接收头平时高电平，可选择上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;   // PA0 复用为 TIM2_CH1
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 定时器基础配置
    htim2.Instance = TIM2;
    // 计算预分频：定时器时钟 = 84MHz (假设 APB1 为 42MHz，且 APB1 预分频不为 1，则定时器时钟为 2*42=84MHz)
    // 欲得 1MHz 计数，预分频 = 84 - 1
    htim2.Init.Prescaler = 84 - 1;               // 计数时钟 1MHz (1µs 步进)
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;               // 32 位最大周期，不会溢出
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim2);

    // 4. 配置输入捕获通道
    sConfigIC.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;   // 双边沿捕获（可根据需求改为上升沿或下降沿）
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;  // 直接映射到 TI1
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;            // 不分频
    sConfigIC.ICFilter = 0;                             // 无输入滤波
    HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

    // 5. 启动输入捕获并使能中断
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    // 6. 设置中断优先级（低于 SysTick，建议 5~10）
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
