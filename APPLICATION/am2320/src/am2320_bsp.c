#include "am2320_bsp.h"

/* 
 * ������		��	Am2320_Init
 * ����			:	AM2320��ʼ��
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
int Am2320_Init(void)
{
	//IO�����ýṹ��
	GPIO_InitTypeDef			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//SCL
	AM2320_SCL_APBxCLK_CMD(AM2320_SCL_GPIO_CLK , ENABLE);

	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin   = AM2320_SCL_GPIO_Pin;					
	GPIO_Init(AM2320_SCL_GPIOx , &GPIO_InitStruct);
	
	//SCL���ֵͽ��뵥���ߴ���
	GPIO_ResetBits(AM2320_SCL_GPIOx, AM2320_SCL_GPIO_Pin);
	
	//SDA
	AM2320_SDA_APBxCLK_CMD(AM2320_SDA_GPIO_CLK , ENABLE);
	
	//���ģʽ
	Am2320_SdaOutMode();
	return 0;
}

/* 
 * ������		��	Am2320_SdaInMode
 * ����			:	AM2320 SDA��������ģʽ
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
void Am2320_SdaInMode(void)
{
	//GPIO���ýṹ��
	GPIO_InitTypeDef 			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//��ʱ�����ýṹ��
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	
	//��ʱ���������û�����
	TIM_ICInitTypeDef  			TIM_ICInitStruct;  
	TIM_ICStructInit(&TIM_ICInitStruct);
	
	//�ж����ýṹ��
	NVIC_InitTypeDef         	NVIC_InitStruct;
	
	//����Ϊ����
	GPIO_InitStruct.GPIO_Pin = AM2320_SDA_GPIO_Pin;
 	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
 	GPIO_Init(AM2320_SDA_GPIOx, &GPIO_InitStruct);
	
	//��ʼ��TIM
	AM2320_SDA_TIM_APBxCLK_CMD(AM2320_SDA_TIM_CLK, ENABLE);//����TIMʱ��

	//���ö�ʱ��
	TIM_TimeBaseInitStruct.TIM_Prescaler=(SystemCoreClock/1000000 - 1);//��ʱ����Ƶ:����Ϊ1us
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;//����
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0; 
	TIM_TimeBaseInitStruct.TIM_Period= 10000 - 1;//10ms���
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;

	TIM_TimeBaseInit(AM2320_SDA_TIM_TIMx, &TIM_TimeBaseInitStruct);
	
	//����Ϊ����ģʽ
	TIM_ICInitStruct.TIM_Channel = AM2320_SDA_TIM_CC;// ѡ�������
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
	TIM_ICInitStruct.TIM_ICFilter = 0x04;//IC4F=0011 ���������˲��� 8����ʱ��ʱ�������˲�
	TIM_ICInit(AM2320_SDA_TIM_TIMx, &TIM_ICInitStruct);//��ʼ����ʱ�����벶��ͨ��
	
	//���û��ж�
	NVIC_InitStruct.NVIC_IRQChannel = AM2320_SDA_TIM_IRQn; 
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;  //�����ȼ�3��
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStruct);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
	
	//����ж�
	TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, TIM_IT_Update);
	TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC);	
	
	//��������ж�
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, TIM_IT_Update,ENABLE);
	
	//���������ж�
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC,ENABLE);
	
	//���ö�ʱ��
	AM2320_SDA_TIM_TIMx->CNT = 0;

	//ʹ�ܶ�ʱ��
	TIM_Cmd(AM2320_SDA_TIM_TIMx, ENABLE); 
}

/* 
 * ������		��	Am2320_SdaOutMode
 * ����			:	AM2320 SDA�������ģʽ
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
void Am2320_SdaOutMode(void)
{
	//�رն�ʱ��
	TIM_Cmd(AM2320_SDA_TIM_TIMx, DISABLE); 
	
	//�ر��ж�
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, TIM_IT_Update, DISABLE);
	
	//�رղ����ж�Bit_RESET
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC, DISABLE);
	
	//GPIO���ýṹ��
	GPIO_InitTypeDef 			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//����Ϊ���
	GPIO_InitStruct.GPIO_Pin = AM2320_SDA_GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AM2320_SDA_GPIOx, &GPIO_InitStruct);
	GPIO_SetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
}

