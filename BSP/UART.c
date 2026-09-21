#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "UART.h"
UART_HandleTypeDef uart6;

/**
 * @brief  初始化 ESP8266 通信所用串口 USART6：把 PC6(TX)/PC7(RX) 配为 USART6 复用功能，参数 115200-8-N-1，并使能 USART6 中断
 * @param  无
 * @retval 无
 */
void esp8266_uart_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;     
    GPIO_InitStruct.Pull = GPIO_NOPULL;               
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     
	GPIO_InitStruct.Alternate=GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;     
    GPIO_InitStruct.Pull = GPIO_NOPULL;               
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     
	GPIO_InitStruct.Alternate=GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


	__HAL_RCC_USART6_CLK_ENABLE();

	uart6.Instance=USART6;
	uart6.Init.BaudRate=115200;
	uart6.Init.Mode=UART_MODE_TX_RX;
	uart6.Init.StopBits=UART_STOPBITS_1;
	uart6.Init.WordLength=UART_WORDLENGTH_8B;
	uart6.Init.Parity=UART_PARITY_NONE;
	HAL_UART_Init(&uart6);

	NVIC_SetPriority(USART6_IRQn,4);
	HAL_NVIC_EnableIRQ(USART6_IRQn);

}

