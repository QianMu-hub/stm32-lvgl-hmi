
#include "GPIO_Driver.h"

/**
 * @brief  使能 GPIOB 时钟，把 PB2 配置为低速推挽输出（在 LVGL 任务中定时翻转，用作运行指示）
 * @param  无
 * @retval 无
 */
void ur_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();


    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     
    GPIO_InitStruct.Pull = GPIO_NOPULL;             
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);




}

/**
 * @brief  初始化 SPI1（主机/8 位/软件 NSS/16 分频）并使能，同时把 SCK(PA5)、MOSI(PA7) 复用及 LCD 的 RES/DC/CS 控制脚配置为输出
 * @param  无
 * @retval 无
 */
void ur_SPI1_Init(void)
{
    // 使能 SPI1 时钟
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    // 使能 GPIOA 时钟
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    // 配置 SPI 引脚 (SCK=PA5, MOSI=PA7)
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_5);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_7, LL_GPIO_AF_5);

    // 配置控制引脚 (RES, DC, CS)
    LL_GPIO_SetPinMode(GPIOA, LCD_RES_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LCD_RES_Pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LCD_RES_Pin, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetOutputPin(GPIOA, LCD_RES_Pin);

    LL_GPIO_SetPinMode(GPIOA, LCD_DC_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LCD_DC_Pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LCD_DC_Pin, LL_GPIO_SPEED_FREQ_LOW);
    // DC 初始电平视需要而定，可先设为低
    LL_GPIO_ResetOutputPin(GPIOA, LCD_DC_Pin);

    LL_GPIO_SetPinMode(GPIOA, LCD_CS_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LCD_CS_Pin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LCD_CS_Pin, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetOutputPin(GPIOA, LCD_CS_Pin);  // CS 默认高（未选中）

    // 初始化 SPI 参数
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;  // 可先降低测试
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
		
    // 使能 SPI1
    LL_SPI_Enable(SPI1);

    // 软件 NSS 模式下必须设置 SSI 位为 1
    SPI1->CR1 |= SPI_CR1_SSI;

    // 清除可能存在的错误标志
    LL_SPI_ClearFlag_OVR(SPI1);	
}
/**
 * @brief  初始化 DMA2 数据流 3：把内存数据搬到 SPI1->DR 用于 LCD 刷屏（源地址与传输长度在每次刷屏时另行设置），并使能传输完成中断
 * @param  无
 * @retval 无
 */
void ur_DMA_Init(void)
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
	
	LL_DMA_InitTypeDef LL_DMA_Initstructure = {0};
	LL_DMA_Initstructure.Mode=LL_DMA_MODE_NORMAL;
	LL_DMA_Initstructure.Channel=LL_DMA_CHANNEL_3;
	LL_DMA_Initstructure.Direction=LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	LL_DMA_Initstructure.FIFOMode=LL_DMA_FIFOMODE_DISABLE;
	       
	LL_DMA_Initstructure.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	LL_DMA_Initstructure.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
	LL_DMA_Initstructure.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	LL_DMA_Initstructure.PeriphOrM2MSrcDataSize = LL_DMA_MDATAALIGN_BYTE;
	LL_DMA_Initstructure.PeriphOrM2MSrcAddress = (uint32_t)&SPI1->DR;
	
	LL_DMA_Initstructure.MemoryOrM2MDstAddress=0;
	LL_DMA_Initstructure.NbData=0;
	LL_DMA_Initstructure.Priority=LL_DMA_PRIORITY_MEDIUM;
	
	LL_DMA_Init(DMA2,3,&LL_DMA_Initstructure);
	LL_DMA_EnableIT_TC(DMA2,3);
	NVIC_SetPriority(DMA2_Stream3_IRQn,4);
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}
