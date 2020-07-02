
#include "includes.h"
#include "led_bsp.h"
#include "math.h"
#include "task.h"

//===========================相关配置宏定义=====================================

uint8_t DIGITAL_LED_CODE[] = 
{// 0	 1	  2	   3	4	 5	  6	   7	8	 9
	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};

typedef struct
{
	//需要显示的数据
	float digital_data;
	//显示的小数点位数
	uint8_t decimal_number;
	//显示的数码管数量
	uint8_t	led_num;
	
	//动画状态
	uint8_t animation;
}DIGITAL_LED_PARAMETER;


//===========================相关全局变量定义=====================================

DIGITAL_LED_PARAMETER		digital_parameter;





//===========================相关内部函数申明=====================================
//数字显示LED服务函数
static void  Task_DigitalLed(void *p_arg);
//数显LED发送数据
static void DigitalLed_SendData(uint8_t data);
//获取绝对值:在app.c中定义
extern float Get_Absolute(float value);


/* 
 * 函数名		：	Task_DigitalLED
 * 描述			:	数字LED显示程序
 * 输入			:	void *p_arg
 * 输出			:	无
 * 说明			:	无
 */
#define	TASK_LED_PRIO						5
#define	TASK_LED_STK_SIZE					200
static TaskHandle_t			handle_led;;	
void DigitalLed_ParameterInit(void)
{
	//初始化硬件
	DigitalLed_Init();
	
	//创建任务
	xTaskCreate((TaskFunction_t )Task_DigitalLed,	//任务函数
                (const char*    )"LED",          	//任务名称
                (uint16_t       )TASK_LED_STK_SIZE,	//任务堆栈大小
                (void*          )NULL,				//传递给任务函数的参数
                (UBaseType_t    )TASK_LED_PRIO,		//任务优先级
                (TaskHandle_t*  )&handle_led);   	//任务句柄
	vTaskDelay(10);		
}

/* 
 * 函数名		：	DigitalLed_SetValue
 * 描述			:	设置数字LED显示值
 * 输入			:	value:数值
 * 输出			:	无
 * 说明			:	无
 */
void DigitalLed_SetValue(float value)
{
	digital_parameter.digital_data = value;
}


/* 
 * 函数名		：	DigitalLed_SetAnimation
 * 描述			:	设置数字LED动画状态
 * 输入			:	state：状态
 * 输出			:	无
 * 说明			:	无
 */
void DigitalLed_SetAnimation(ANIMATION animation)
{
	digital_parameter.animation = animation;
}

/* 
 * 函数名		：	Task_DigitalLED
 * 描述			:	数字LED显示程序
 * 输入			:	void *p_arg
 * 输出			:	无
 * 说明			:	无
 */
static void Task_DigitalLed(void *p_arg)
{
	float float_data;//放大倍数
	
	//小数点显示两位
	digital_parameter.decimal_number = 1;
	//总显示位数3
	digital_parameter.led_num = 3;
	
	//初始化显示的数据
	digital_parameter.digital_data = 0;
	
	uint16_t animation_count = 0;
	uint8_t animation_step = 0;
	while(1)
	{
		if(digital_parameter.animation == ANIMATION_NONE
		|| digital_parameter.animation == ANIMATION_DOT){
			//数值过大
			if(Get_Absolute(digital_parameter.digital_data) >= 100){
				for(uint8_t i = digital_parameter.led_num ; i > 0 ; i --){
					DigitalLed_SendData(~0x40); 
					DigitalLed_SendData(0x01 << (i-1));
					
					//上升沿生效
					GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					vTaskDelay(1);
					GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					
					//显示时间
					vTaskDelay(1);	
				}
			}
			//正常数值
			else{
				float_data = digital_parameter.digital_data;
				//将要显示的数据小数点右移至需要显示的位数前面
				for(uint8_t i = 0 ; i < digital_parameter.decimal_number ; i++){
					float_data *= 10;
				}
				//整数余数
				uint16_t remainder = float_data;
				
				//零去除标志
				uint8_t no_zero = RESET;
				//显示对应得数据
				for(uint8_t i = digital_parameter.led_num ; i > 0 ; i --){
					//取余数
					remainder = remainder % (uint16_t)(pow(10,i));
					//取出要显示的位
					uint8_t data = (uint8_t)(remainder / pow(10,i-1));
					//首位为0的话关闭显示
					if(no_zero == RESET && data == 0 && i > (digital_parameter.decimal_number+1)){
						data = 0xFF;
					}
					else{
						no_zero = SET;
						//取出字模
						data = DIGITAL_LED_CODE[data];
						//检测是否需要添加上小数点
						if(i == (digital_parameter.decimal_number+1)){
							data &= 0x7F;
						}
					}
					//写入数据
					DigitalLed_SendData(data);
					DigitalLed_SendData(0x01 << (i-1));
					
					//上升沿生效
					GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					vTaskDelay(1);
					GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					
					//显示时间
					vTaskDelay(1);	
				}
			}
		}
		
		//动画显示
		switch(digital_parameter.animation){
		case ANIMATION_NONE:break;
		case ANIMATION_DOT:
			//写入数据
			DigitalLed_SendData(0x7f);
			DigitalLed_SendData(0x01 << (4-1));
			
			//上升沿生效
			GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
			vTaskDelay(1);
			GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
		break;
		case ANIMATION_REFRESH:
			uint8_t data[4];
			//以下部分实现了LED流水达到刷新的视觉效果
			switch(animation_step){
			case 0: 	data[3] = 0x01; data[2] = 0x01; data[1] = 0x01; data[0] = 0x00; break;
			case 1: 	data[3] = 0x00; data[2] = 0x01; data[1] = 0x01; data[0] = 0x01; break;
			case 2: 	data[3] = 0x00; data[2] = 0x00; data[1] = 0x01; data[0] = 0x03; break;
			case 3: 	data[3] = 0x00; data[2] = 0x00; data[1] = 0x00; data[0] = 0x07; break;
			case 4: 	data[3] = 0x00; data[2] = 0x00; data[1] = 0x00; data[0] = 0x0E; break;
			case 5: 	data[3] = 0x00; data[2] = 0x00; data[1] = 0x08; data[0] = 0x0C; break;
			case 6: 	data[3] = 0x00; data[2] = 0x08; data[1] = 0x08; data[0] = 0x08; break;
			case 7: 	data[3] = 0x08; data[2] = 0x08; data[1] = 0x08; data[0] = 0x00; break;
			case 8: 	data[3] = 0x18; data[2] = 0x08; data[1] = 0x00; data[0] = 0x00; break;
			case 9: 	data[3] = 0x38; data[2] = 0x00; data[1] = 0x00; data[0] = 0x00; break;
			case 10: 	data[3] = 0x31; data[2] = 0x00; data[1] = 0x00; data[0] = 0x00; break;
			case 11: 	data[3] = 0x11; data[2] = 0x01; data[1] = 0x00; data[0] = 0x00; break;
			}
			for(uint8_t i = 0; i < 4; i++){
				DigitalLed_SendData(~data[i]);
				DigitalLed_SendData(0x01 << i);
				//上升沿生效
				GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
				vTaskDelay(1);
				GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
			}
			//每一段时间后切换至下一个步骤达到流水效果
			if(++animation_count >= 20){
				animation_count = 0;
				if(++animation_step > 11)
					animation_step = 0;
			}
		break;
		}
	}
}



/* 
 * 函数名		：	DigitalLed_SendData
 * 描述			:	数显LED发送数据
 * 输入			:	data
 * 输出			:	无
 * 说明			:	无
 */
static void DigitalLed_SendData(uint8_t data)
{
	for(uint8_t i = 8 ; i >=1 ; i--){
		//发送位
		if(data&0x80)
			GPIO_SetBits(DIGITAL_LED_DIO_GPIOx , DIGITAL_LED_DIO_GPIO_Pin);
		else
			GPIO_ResetBits(DIGITAL_LED_DIO_GPIOx , DIGITAL_LED_DIO_GPIO_Pin);
		data <<= 1;
		//时钟上升沿
		GPIO_ResetBits(DIGITAL_LED_SCK_GPIOx , DIGITAL_LED_SCK_GPIO_Pin);
		GPIO_SetBits(DIGITAL_LED_SCK_GPIOx , DIGITAL_LED_SCK_GPIO_Pin);
	}
}






