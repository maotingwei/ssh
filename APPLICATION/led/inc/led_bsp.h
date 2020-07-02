#ifndef __DIGITAL_LED_BSP_H
#define __DIGITAL_LED_BSP_H
/*include----------------------------------------------*/
#include	"stm32f10x.h"

/*define-----------------------------------------------*/

#define DIGITAL_LED_SCK_APBxCLK_FUN		RCC_APB2PeriphClockCmd
#define DIGITAL_LED_SCK_GPIO_CLK		RCC_APB2Periph_GPIOA
#define DIGITAL_LED_SCK_GPIOx			GPIOA
#define DIGITAL_LED_SCK_GPIO_Pin		GPIO_Pin_4

#define DIGITAL_LED_RSLK_APBxCLK_FUN	RCC_APB2PeriphClockCmd
#define DIGITAL_LED_RSLK_GPIO_CLK		RCC_APB2Periph_GPIOA
#define DIGITAL_LED_RSLK_GPIOx			GPIOA
#define DIGITAL_LED_RSLK_GPIO_Pin		GPIO_Pin_5

#define DIGITAL_LED_DIO_APBxCLK_FUN		RCC_APB2PeriphClockCmd
#define DIGITAL_LED_DIO_GPIO_CLK		RCC_APB2Periph_GPIOA
#define DIGITAL_LED_DIO_GPIOx			GPIOA
#define DIGITAL_LED_DIO_GPIO_Pin		GPIO_Pin_6




/* Variable -------------------------------------------*/

/* Function -------------------------------------------*/

void DigitalLed_Init(void);

#endif 
