#include "led_bsp.h"

/* 
 * 函数名		：	DigitalLed_Init
 * 描述			:	LED数显管
 * 输入			:	void 
 * 输出			:	无
 * 说明			:	无
 */
void DigitalLed_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	//定义一个GPIO_InitTypeDef类型的结构体

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//使能复用时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//禁止JTAG	
	//=======================SCK=====================================
	DIGITAL_LED_SCK_APBxCLK_FUN(DIGITAL_LED_SCK_GPIO_CLK, ENABLE);	//打开GPIO的外设时钟

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_SCK_GPIO_Pin;	//选择控制的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置为上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置输出速率为50MHz
	GPIO_Init(DIGITAL_LED_SCK_GPIOx,&GPIO_InitStructure);	//调用库函数初始化
	
	//=======================DIO=====================================
	DIGITAL_LED_DIO_APBxCLK_FUN(DIGITAL_LED_DIO_GPIO_CLK, ENABLE);	//打开GPIO的外设时钟

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_DIO_GPIO_Pin;	//选择控制的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置为上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置输出速率为50MHz
	GPIO_Init(DIGITAL_LED_DIO_GPIOx,&GPIO_InitStructure);	//调用库函数初始化
	
	//=======================RSLK=====================================
	DIGITAL_LED_RSLK_APBxCLK_FUN(DIGITAL_LED_RSLK_GPIO_CLK, ENABLE);	//打开GPIO的外设时钟

	GPIO_InitStructure.GPIO_Pin = DIGITAL_LED_RSLK_GPIO_Pin;	//选择控制的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置为上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置输出速率为50MHz
	GPIO_Init(DIGITAL_LED_RSLK_GPIOx,&GPIO_InitStructure);	//调用库函数初始化
	
}





