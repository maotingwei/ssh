#include "includes.h"
#include "timers.h"


//===========================相关变量申明=======================================
//static TimerHandle_t 	handle_led;	//LED定时器句柄


//===========================相关函数申明=======================================
////LED初始化
//static void LED_Init(void);
////LED定时器回调函数
//static void LED_TmrCallback(TimerHandle_t xTimer);




/* 
 * 函数名：xTaskStart_Handler
 * 描述  : 起始任务服务函数
 * 输入  ：pvParameters
 * 输出  : 无
 */
extern TaskHandle_t StartTask_Handler;

void xTaskStart_Handler(void *pvParameters)
{
	//初始化LED
	DigitalLed_ParameterInit();
	
	//初始化AM2320
	Am2320_ParameterInit();
	
	//删除开始任务
	vTaskDelete(StartTask_Handler); 
}


///* 
// * 函数名		：	LED_Init
// * 描述			:	LED指示灯初始化
// * 输入			:	无
// * 输出			:	无
// * 说明			:	无
// */
//static void LED_Init(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	LED_APBxCLK_CMD(LED_GPIO_CLK,ENABLE);//LED
//		
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
//	GPIO_InitStructure.GPIO_Pin=LED_GPIO_Pin;
//	GPIO_Init(LED_GPIOx,&GPIO_InitStructure);
//}
///* 
// * 函数名		：	LED_TmrCallback
// * 描述			:	LED定时器回调函数
// * 输入			:	无
// * 输出			:	无
// * 说明			:	无
// */
//static void LED_TmrCallback(TimerHandle_t xTimer)
//{
//	//实现闪烁
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


//============================所有文件都可能用到的函数==================================

/* 
 * 函数名		：	Get_Absolute
 * 描述			:	获取绝对值
 * 输入			:	float value
 * 输出			:	返回绝对值
 * 说明			:	无
 */
float Get_Absolute(float value)
{
	return (value>0? value:-value);
}


/* 
 * 函数名		：	delay_us
 * 描述			:	延时nus
 * 输入			:	nus:0~204522252(最大值即2^32/fac_us@fac_us=168)	   
 * 输出			:	无
 * 说明			:	无
 */							   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus * SystemCoreClock/1000000; 	//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};										    
}

/* 
 * 函数名		：	delay_ms
 * 描述			:	ms延时
 * 输入			:	ms
 * 输出			:	无
 * 说明			:	无
 */
void delay_ms(uint16_t ms)
{
	vTaskDelay(ms);
}
