#include "led_bsp.h"

/* 
 * ������		��	DigitalLed_Init
 * ����			:	LED���Թ�
 * ����			:	void 
 * ���			:	��
 * ˵��			:	��
 */
void DigitalLed_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//����һ��GPIO_InitTypeDef���͵Ľṹ��

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//ʹ�ܸ���ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//��ֹJTAG	
	//=======================SCK=====================================
	DIGITAL_LED_SCK_APBxCLK_FUN(DIGITAL_LED_SCK_GPIO_CLK, ENABLE);	//��GPIO������ʱ��

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_SCK_GPIO_Pin;	//ѡ����Ƶ�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//����Ϊ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�����������Ϊ50MHz
	GPIO_Init(DIGITAL_LED_SCK_GPIOx,&GPIO_InitStructure);	//���ÿ⺯����ʼ��
	
	//=======================DIO=====================================
	DIGITAL_LED_DIO_APBxCLK_FUN(DIGITAL_LED_DIO_GPIO_CLK, ENABLE);	//��GPIO������ʱ��

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_DIO_GPIO_Pin;	//ѡ����Ƶ�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//����Ϊ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�����������Ϊ50MHz
	GPIO_Init(DIGITAL_LED_DIO_GPIOx,&GPIO_InitStructure);	//���ÿ⺯����ʼ��
	
	//=======================RSLK=====================================
	DIGITAL_LED_RSLK_APBxCLK_FUN(DIGITAL_LED_RSLK_GPIO_CLK, ENABLE);	//��GPIO������ʱ��

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_RSLK_GPIO_Pin;	//ѡ����Ƶ�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//����Ϊ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�����������Ϊ50MHz
	GPIO_Init(DIGITAL_LED_RSLK_GPIOx,&GPIO_InitStructure);	//���ÿ⺯����ʼ��
	
}





