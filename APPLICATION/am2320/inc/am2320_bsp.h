#ifndef __MORTAR_BSP_H
#define __MORTAR_BSP_H

/*include-----------------------------------------------*/
#include "stm32f10x.h"

/*define-----------------------------------------------*/

//AM2320 IO口
#define AM2320_SCL_APBxCLK_CMD				RCC_APB2PeriphClockCmd
#define AM2320_SCL_GPIO_CLK					RCC_APB2Periph_GPIOA
#define AM2320_SCL_GPIOx					GPIOA
#define AM2320_SCL_GPIO_Pin					GPIO_Pin_10

#define AM2320_SDA_APBxCLK_CMD				RCC_APB2PeriphClockCmd
#define AM2320_SDA_GPIO_CLK					RCC_APB2Periph_GPIOA
#define AM2320_SDA_GPIOx					GPIOA
#define AM2320_SDA_GPIO_Pin					GPIO_Pin_2

#define AM2320_SDA_TIM_APBxCLK_CMD			RCC_APB1PeriphClockCmd
#define AM2320_SDA_TIM_CLK					RCC_APB1Periph_TIM2
#define AM2320_SDA_TIM_TIMx 				TIM2
#define AM2320_SDA_TIM_CC   				TIM_Channel_3
#define AM2320_SDA_TIM_IT_CC 				TIM_IT_CC3

#define AM2320_SDA_TIM_IRQn					TIM2_IRQn
#define	AM2320_SDA_TIM_IRQHandler			TIM2_IRQHandler



/* Variable -----------------------------------------------*/


/* Function -----------------------------------------------*/

//AM2320初始化
int Am2320_Init(void);	

//SDA输入模式
void Am2320_SdaInMode(void);

//SDA输出模式
void Am2320_SdaOutMode(void);
#endif  //end H


