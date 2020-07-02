#include "am2320_bsp.h"

/* 
 * 函数名		：	Am2320_Init
 * 描述			:	AM2320初始化
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
int Am2320_Init(void)
{
	//IO口配置结构体
	GPIO_InitTypeDef			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//SCL
	AM2320_SCL_APBxCLK_CMD(AM2320_SCL_GPIO_CLK , ENABLE);

	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin   = AM2320_SCL_GPIO_Pin;					
	GPIO_Init(AM2320_SCL_GPIOx , &GPIO_InitStruct);
	
	//SCL保持低进入单总线传输
	GPIO_ResetBits(AM2320_SCL_GPIOx, AM2320_SCL_GPIO_Pin);
	
	//SDA
	AM2320_SDA_APBxCLK_CMD(AM2320_SDA_GPIO_CLK , ENABLE);
	
	//输出模式
	Am2320_SdaOutMode();
	return 0;
}

/* 
 * 函数名		：	Am2320_SdaInMode
 * 描述			:	AM2320 SDA处于输入模式
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
void Am2320_SdaInMode(void)
{
	//GPIO配置结构体
	GPIO_InitTypeDef 			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//定时器配置结构体
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	
	//定时器输入配置机构体
	TIM_ICInitTypeDef  			TIM_ICInitStruct;  
	TIM_ICStructInit(&TIM_ICInitStruct);
	
	//中断配置结构体
	NVIC_InitTypeDef         	NVIC_InitStruct;
	
	//配置为输入
	GPIO_InitStruct.GPIO_Pin = AM2320_SDA_GPIO_Pin;
 	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
 	GPIO_Init(AM2320_SDA_GPIOx, &GPIO_InitStruct);
	
	//初始化TIM
	AM2320_SDA_TIM_APBxCLK_CMD(AM2320_SDA_TIM_CLK, ENABLE);//开启TIM时钟

	//配置定时器
	TIM_TimeBaseInitStruct.TIM_Prescaler=(SystemCoreClock/1000000 - 1);//定时器分频:设置为1us
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;//向上
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0; 
	TIM_TimeBaseInitStruct.TIM_Period= 10000 - 1;//10ms溢出
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;

	TIM_TimeBaseInit(AM2320_SDA_TIM_TIMx, &TIM_TimeBaseInitStruct);
	
	//配置为输入模式
	TIM_ICInitStruct.TIM_Channel = AM2320_SDA_TIM_CC;// 选择输入端
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM_ICInitStruct.TIM_ICFilter = 0x04;//IC4F=0011 配置输入滤波器 8个定时器时钟周期滤波
	TIM_ICInit(AM2320_SDA_TIM_TIMx, &TIM_ICInitStruct);//初始化定时器输入捕获通道
	
	//配置化中断
	NVIC_InitStruct.NVIC_IRQChannel = AM2320_SDA_TIM_IRQn; 
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;  //从优先级3级
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStruct);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	//清除中断
	TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, TIM_IT_Update);
	TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC);	
	
	//允许更新中断
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, TIM_IT_Update,ENABLE);
	
	//开启捕获中断
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC,ENABLE);
	
	//重置定时器
	AM2320_SDA_TIM_TIMx->CNT = 0;

	//使能定时器
	TIM_Cmd(AM2320_SDA_TIM_TIMx, ENABLE); 
}

/* 
 * 函数名		：	Am2320_SdaOutMode
 * 描述			:	AM2320 SDA处于输出模式
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
void Am2320_SdaOutMode(void)
{
	//关闭定时器
	TIM_Cmd(AM2320_SDA_TIM_TIMx, DISABLE); 
	
	//关闭中断
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, TIM_IT_Update, DISABLE);
	
	//关闭捕获中断Bit_RESET
	TIM_ITConfig(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC, DISABLE);
	
	//GPIO配置结构体
	GPIO_InitTypeDef 			GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	//配置为输出
	GPIO_InitStruct.GPIO_Pin = AM2320_SDA_GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(AM2320_SDA_GPIOx, &GPIO_InitStruct);
	GPIO_SetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
}

