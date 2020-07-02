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
//�����LED��ʼ��
void DigitalLed_ParameterInit(void);

//����LED��ʾ����ֵ
void DigitalLed_SetValue(float value);
//���ö���
void DigitalLed_SetAnimation(ANIMATION animation);
#endif
