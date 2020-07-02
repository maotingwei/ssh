#include "includes.h"
#include "am2320_bsp.h"
#include "task.h"
#include "timers.h"

//=========================相关宏定义============================
typedef enum
{
	AM2320_READ_FREE				= 0x00,
	AM2320_READ_START				,
	AM2320_READ_WAIT_TREL			, //起始时80us低电平响应
	AM2320_READ_WAIT_TREH			, //起始时80us高电平响应
	AM2320_READ_WAIT_TDATA			, //数据传输开始
	AM2320_READ_WAIT_DATA			, //数据传输时DATA的响应，上降沿响应
	AM2320_READ_END					, //读取结束
}AM2320_READ_STEP;

typedef enum
{
	AM2320_ERR_NONE					= 0x00,
	AM2320_ERR_TIM_OVER				,
}AM2320_ERROR;


typedef struct
{
	AM2320_ERROR		last_err;		//异常
	
	AM2320_READ_STEP	read_step;		//当前的步骤
	
	float				humi;			//当前的湿度
	float				temp;			//当前的温度
	
	uint8_t				data[5];		//临时的数据
	uint8_t				data_index;		//保存当前为读到的第几位
}AM2320;



//=======================全局变量定义================================

static TaskHandle_t			handle_am2320;
AM2320	am2320;

//=======================内部函数申明================================
//AM2320任务服务函数
static void  Task_Am2320(void *p_arg);

//读取开始
static void Am2320_ReadStart(AM2320 *p_am2320);

//延时us
extern void delay_us(u32 nus);
//延时ms
extern void delay_ms(uint16_t ms);






/* 
 * 函数名		：	Am2320_ParameterInit
 * 描述			:	参数初始化：硬件初始化，变量初始化
 * 输入			:	无
 * 输出			:	0 = 成功
 * 说明			:	无
 */
#define	TASK_AM2320_PRIO						2
#define	TASK_AM2320_STK_SIZE					200
void Am2320_ParameterInit(void)
{
	//IO口初始化
	Am2320_Init();
	
	//创建任务
	xTaskCreate((TaskFunction_t )Task_Am2320,			//任务函数
                (const char*    )"am2320",          	//任务名称
                (uint16_t       )TASK_AM2320_STK_SIZE,	//任务堆栈大小
                (void*          )NULL,					//传递给任务函数的参数
                (UBaseType_t    )TASK_AM2320_PRIO,		//任务优先级
                (TaskHandle_t*  )&handle_am2320);   	//任务句柄
	
	DigitalLed_SetAnimation(ANIMATION_REFRESH);
	
	vTaskDelay(3000);
}



/* 
 * 函数名		：	Task_Am2320
 * 描述			:	服务函数
 * 输入			:	void *p_arg
 * 输出			:	无
 * 说明			:	无
 */
static void  Task_Am2320(void *p_arg)
{
	while(1)
	{
		//开始读取温度
		Am2320_ReadStart(&am2320);
		
		//周期读取
		vTaskDelay(1000);
	}
}



/* 
 * 函数名		：	Am2320_SetTimOverValue
 * 描述			:	AM2320设置超时时间
 * 输入			:	us:微秒
 * 输出			:	无
 * 说明			:	无
 */
static void  Am2320_SetTimOverValue(uint16_t us)
{
	//设置超时值
	AM2320_SDA_TIM_TIMx->ARR = (us - 1);
}


/* 
 * 函数名		：	Am2320_SetTimOverValue
 * 描述			:	AM2320设置超时时间
 * 输入			:	us:微秒
 * 输出			:	无
 * 说明			:	无
 */
static void  Am2320_SetTimPolarity(uint16_t polarity)
{
	switch(AM2320_SDA_TIM_CC)
	{
	case TIM_Channel_1: TIM_OC1PolarityConfig(AM2320_SDA_TIM_TIMx, polarity); break;
	case TIM_Channel_2: TIM_OC2PolarityConfig(AM2320_SDA_TIM_TIMx, polarity); break;
	case TIM_Channel_3: TIM_OC3PolarityConfig(AM2320_SDA_TIM_TIMx, polarity); break;
	case TIM_Channel_4: TIM_OC4PolarityConfig(AM2320_SDA_TIM_TIMx, polarity); break;
	default:break;
	}
}

/* 
 * 函数名		：	Am2320_ReadByte
 * 描述			:	AM2320读取开始
 * 输入			:	p_am2320
 * 输出			:	返回读取到的字节数据
 * 说明			:	无
 */
static void Am2320_ReadStart(AM2320 *p_am2320)
{
	//开始读取
	p_am2320->read_step = AM2320_READ_START;
	p_am2320->last_err = AM2320_ERR_NONE;
	
	//复位读取到的数据
	p_am2320->data_index = 0;
	
	for(uint8_t i = 0; i < 5; i++){
		p_am2320->data[i] = 0x00;
	}
	
	//拉低总线,延时18ms
	Am2320_SdaOutMode();
	GPIO_ResetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
	delay_ms(15);
	
	//总线拉高,延时30us
	GPIO_SetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
	delay_us(30);
	
	//进入等待总线拉低响应
	p_am2320->read_step = AM2320_READ_WAIT_TREH;
	
	//进入读取阶段
	Am2320_SdaInMode();
	
	//设置为下降沿响应
	Am2320_SetTimPolarity(TIM_ICPolarity_Rising);
	
	//重置定时器
	AM2320_SDA_TIM_TIMx->CNT = 0;
	//设置超时时间:500us(实际为80us为响应)
	Am2320_SetTimOverValue(500);
}


/* 
 * 函数名		：	Am2320_ReadEnd
 * 描述			:	AM2320读取结束
 * 输入			:	p_am2320
 * 输出			:	返回读取到的字节数据
 * 说明			:	无
 */
static void Am2320_ReadEnd(AM2320 *p_am2320)
{
	static uint8_t count = 0;
	static enum{SHOW_TEMP = 0x00 , SHOW_HUMI = 0x01}show = SHOW_TEMP;
	static uint8_t err_count = 0;
	//开始读取
	p_am2320->read_step = AM2320_READ_END;
	
	if(p_am2320->last_err == AM2320_ERR_NONE){
		uint8_t humi_int = p_am2320->data[0];
		uint8_t humi_deci = p_am2320->data[1];
		
		uint8_t temp_int = p_am2320->data[2];
		uint8_t temp_deci = p_am2320->data[3];
		
		uint8_t check_sum = p_am2320->data[4];
		//计算结果
		uint8_t calc = humi_int + humi_deci + temp_int + temp_deci;
		
		//数据校验
		if(calc == check_sum){
			p_am2320->humi = (humi_int*256 + humi_deci)*0.1;
			p_am2320->temp = (temp_int*256 + temp_deci)*0.1;
			
			//读取完成结束
			DigitalLed_SetAnimation(ANIMATION_NONE);
			
			//显示温度
			if(show == SHOW_TEMP)
				DigitalLed_SetValue(am2320.temp);
			//显示湿度
			else if(show == SHOW_HUMI)
				DigitalLed_SetValue(am2320.humi);
			
			//隔一段时间切换显示对象
			if(++count > 5){
				count = 0;
				if(show == SHOW_TEMP)
					show = SHOW_HUMI;
				else if(show == SHOW_HUMI)
					show = SHOW_TEMP;
			}
		}
		
		err_count = 0;
	}
	else if(++ err_count > 3)
		DigitalLed_SetAnimation(ANIMATION_REFRESH);
	
	//输出模式
	Am2320_SdaOutMode();
}


/* 
 * 函数名		：	AM2320_SDA_TIM_IRQHandler
 * 描述			:	定时器中断
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
uint8_t a = 0,b = 0;
void AM2320_SDA_TIM_IRQHandler(void)
{
	AM2320	*p_am2320 = &am2320;
	//超时了
	if(TIM_GetITStatus(AM2320_SDA_TIM_TIMx, TIM_IT_Update) != RESET){
		//清除中断
		TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, TIM_IT_Update);	 
		
		p_am2320->last_err = AM2320_ERR_TIM_OVER;
		p_am2320->read_step = AM2320_READ_FREE;
		
		//读取结束
		Am2320_ReadEnd(p_am2320);	
	}
	
	//读取到边沿信号
	if(TIM_GetITStatus(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC)!=RESET){
		//清除中断
		TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC);
		//重置定时器
		AM2320_SDA_TIM_TIMx->CNT = 0;
		
		switch(p_am2320->read_step)
		{	
		case AM2320_READ_WAIT_TREH://80us低电平完成,开始80us高电平
			//确认读取到了高电平
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_SET){
				//进入等待传输开始
				p_am2320->read_step = AM2320_READ_WAIT_TDATA;
				                              
				//设置为上升沿捕获
				Am2320_SetTimPolarity(TIM_ICPolarity_Falling);
				
				//设置超时时间:80us: 80us信号低电平时间
				Am2320_SetTimOverValue(85+20);//最长85us+20us富余
				
				//读取到数据，指示点亮表示开始读取数据
				DigitalLed_SetAnimation(ANIMATION_DOT);
			}
			
		break;
		
		case AM2320_READ_WAIT_TDATA://80us高电平完成，开始50us低电平与信号电平时间
			//确认读取到了高电平
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_RESET){
				//开始数据传输
				p_am2320->read_step = AM2320_READ_WAIT_DATA;
				
				//设置为上升沿捕获
				Am2320_SetTimPolarity(TIM_ICPolarity_Falling);
				
				//设置超时时间:150us: '0'/'1'的高电平时间(75us) + 50us的低电平时间 + 20us富余
				Am2320_SetTimOverValue(55+75+20);
			}
		break;
		
		case AM2320_READ_WAIT_DATA:
			//确认读取到了低电平
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_RESET){
				//获取当前的耗时
				uint16_t rm_cnt = 0;
				switch(AM2320_SDA_TIM_CC)
				{
				case TIM_Channel_1: rm_cnt = TIM_GetCapture1(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_2: rm_cnt = TIM_GetCapture2(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_3: rm_cnt = TIM_GetCapture3(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_4: rm_cnt = TIM_GetCapture4(AM2320_SDA_TIM_TIMx); break;
				default:break;
				}
				
				//当前需要写入的位
				uint8_t byte = p_am2320->data_index / 8;
				uint8_t bit = p_am2320->data_index % 8;
				
				//判断是否为低电平时间
				if(rm_cnt > (50 + 68))//50us+'1'的最小时间68us
					p_am2320->data[byte] |= (uint8_t)0x01 << (7-bit);
				
				//判断是否读取到5个字节的数据读取到所有的数据
				if(++p_am2320->data_index >= 5*8){
					p_am2320->last_err = AM2320_ERR_NONE;
					//读取结束
					Am2320_ReadEnd(p_am2320);
				}
			}
		break;
		default:break;
		}
	}
}


