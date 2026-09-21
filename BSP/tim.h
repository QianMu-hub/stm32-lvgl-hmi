/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
 * @brief  初始化 TIM1，配置为 PWM 输出模式（通道1，周期 999、比较值 499，约 50% 占空比）
 * @param  无
 * @retval 无
 */
void MX_TIM1_Init(void);
/**
 * @brief  初始化 TIM2，配置为 3 路 PWM 输出（通道1/2/3，PWM1 模式，初始占空比为 0）
 * @param  无
 * @retval 无
 */
void MX_TIM2_Init(void);
/**
 * @brief  初始化 TIM3，配置为 PWM 输出模式（通道1，周期 999、比较值 900，约 90% 占空比）
 * @param  无
 * @retval 无
 */
void MX_TIM3_Init(void);

/**
 * @brief  根据定时器句柄配置 PWM 输出引脚的复用功能（TIM1→PA8，TIM2→PA2/PA15/PB3，TIM3→PB4）
 * @param  htim  定时器句柄，用于判断需要配置哪个定时器实例的引脚
 * @retval 无
 */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

