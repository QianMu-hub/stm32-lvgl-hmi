#ifndef USER_IT_H
#define USER_IT_H

/**
 * @brief  TIM6 全局中断服务函数原型（TIM6 作为 HAL 时基，内部调用 HAL_TIM_IRQHandler 处理中断）
 *         注意：本文件对应 BSP\user_it.c 中的实现已被注释掉，实际生效的定义在 Core\Src\stm32f4xx_it.c
 * @param  无
 * @retval 无
 */
void TIM6_DAC_IRQHandler(void);



#endif
