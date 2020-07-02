#ifndef __DIGITAL_LED_H
#define __DIGITAL_LED_H


/*include-----------------------------------------------*/
#include "stm32f10x.h"
/*define-----------------------------------------------*/

typedef enum
{
	ANIMATION_NONE			= 0x00,
	ANIMATION_DOT			,
	ANIMATION_REFRESH		,
}ANIMATION;
/* Variable -------------------------------------------*/

/* Function -------------------------------------------*/
//数码管LED初始化
void DigitalLed_ParameterInit(void);

//设置LED显示的数值
void DigitalLed_SetValue(float value);
//设置动画
void DigitalLed_SetAnimation(ANIMATION animation);
#endif
