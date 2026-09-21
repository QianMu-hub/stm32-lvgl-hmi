#include "stm32f4xx_hal.h"
#include "user_it.h"
#include "driver_ir_receiver.h"
#include "stm32f4xx_ll_dma.h"
#include "lv_port_disp.h"
#include "stm32f4xx_ll_spi.h"
#include "ST7789.h"
#include "stm32f4xx_hal_uart.h"




//extern TIM_HandleTypeDef htim6;
//void TIM6_DAC_IRQHandler(void)
//{
// HAL_TIM_IRQHandler(&htim6);
//}

//extern void IRReceiver_IRQ_Callback(void);

void EXTI15_10_IRQHandler(void)
{
    // 检查是否是 EXTI Line 15 触发的中断
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET)
    {
        // 清除中断标志位
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);

        // 调用红外接收器驱动的中断回调函数
        IRReceiver_IRQ_Callback();

    }
}

void DMA2_Stream3_IRQHandler(void)
{
    // 检查传输完成标志
    if (LL_DMA_IsActiveFlag_TC3(DMA2))   // Stream3对应标志位TCIF3
    {
        // 清除标志
        LL_DMA_ClearFlag_TC3(DMA2);          
        LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);
				//LL_DMA_ConfigAddresses(DMA2,3,0,(uint32_t)&SPI1->DR,LL_DMA_DIRECTION_MEMORY_TO_PERIPH);				
        while (LL_SPI_IsActiveFlag_BSY(SPI1));        
				IPS200_CS(1);
				LL_SPI_DisableDMAReq_TX(SPI1);
//				LL_SPI_SetDataWidth(SPI1,	LL_SPI_DATAWIDTH_8BIT);
        lv_display_t *disp = lv_disp_get_default();
				//disp_flush_enabled=true;
        if (disp) 
				{
						lv_display_flush_ready(disp);
        }
    }
}
