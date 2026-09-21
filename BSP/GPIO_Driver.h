#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "ST7789.h"
#ifndef GPIO_DRIVER_H
#define GIPO_DRIVER_H

void ur_GPIO_Init(void);
void ur_SPI1_Init(void);
void ur_GPIO_IR_Init(void);
void ur_DMA_Init(void);


#endif
