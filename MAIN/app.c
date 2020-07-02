#include "includes.h"
#include "timers.h"


//===========================��ر�������=======================================
//static TimerHandle_t 	handle_led;	//LED��ʱ�����


//===========================��غ�������=======================================
////LED��ʼ��
//static void LED_Init(void);
////LED��ʱ���ص�����
//static void LED_TmrCallback(TimerHandle_t xTimer);




/* 
 * ��������xTaskStart_Handler
 * ����  : ��ʼ���������
 * ����  ��pvParameters
 * ���  : ��
 */
extern TaskHandle_t StartTask_Handler;

void xTaskStart_Handler(void *pvParameters)
{
	//��ʼ��LED
	DigitalLed_ParameterInit();
	
	//��ʼ��AM2320
	Am2320_ParameterInit();
	
	//ɾ����ʼ����
	vTaskDelete(StartTask_Handler); 
}


///* 
// * ������		��	LED_Init
// * ����			:	LEDָʾ�Ƴ�ʼ��
// * ����			:	��
// * ���			:	��
// * ˵��			:	��
// */
//static void LED_Init(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	LED_APBxCLK_CMD(LED_GPIO_CLK,ENABLE);//LED
//		
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//�������
//	GPIO_InitStructure.GPIO_Pin=LED_GPIO_Pin;
//	GPIO_Init(LED_GPIOx,&GPIO_InitStructure);
//}
///* 
// * ������		��	LED_TmrCallback
// * ����			:	LED��ʱ���ص�����
// * ����			:	��
// * ���			:	��
// * ˵��			:	��
// */
//static void LED_TmrCallback(TimerHandle_t xTimer)
//{
//	//ʵ����˸
//	static uint8_t twinkle = 0;
//	twinkle++;
//	if(twinkle < 1)
//		LED_GPIOx->ODR &= ~LED_GPIO_Pin;
//	else if(twinkle < 2)
//		LED_GPIOx->ODR |= LED_GPIO_Pin;
//	else if(twinkle < 3)
//		LED_GPIOx->ODR &= ~LED_GPIO_Pin;
//	else if(twinkle < 4)
//		LED_GPIOx->ODR |= LED_GPIO_Pin;
//	else if(twinkle < 9)
//		LED_GPIOx->ODR &= ~LED_GPIO_Pin;
//	else
//		twinkle = 0;
//}


//============================�����ļ��������õ��ĺ���==================================

/* 
 * ������		��	Get_Absolute
 * ����			:	��ȡ����ֵ
 * ����			:	float value
 * ���			:	���ؾ���ֵ
 * ˵��			:	��
 */
float Get_Absolute(float value)
{
	return (value>0? value:-value);
}


/* 
 * ������		��	delay_us
 * ����			:	��ʱnus
 * ����			:	nus:0~204522252(���ֵ��2^32/fac_us@fac_us=168)	   
 * ���			:	��
 * ˵��			:	��
 */							   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus * SystemCoreClock/1000000; 	//��Ҫ�Ľ����� 
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};										    
}

/* 
 * ������		��	delay_ms
 * ����			:	ms��ʱ
 * ����			:	ms
 * ���			:	��
 * ˵��			:	��
 */
void delay_ms(uint16_t ms)
{
	vTaskDelay(ms);
}
