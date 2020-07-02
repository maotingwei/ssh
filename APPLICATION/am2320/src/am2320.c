#include "includes.h"
#include "am2320_bsp.h"
#include "task.h"
#include "timers.h"

//=========================��غ궨��============================
typedef enum
{
	AM2320_READ_FREE				= 0x00,
	AM2320_READ_START				,
	AM2320_READ_WAIT_TREL			, //��ʼʱ80us�͵�ƽ��Ӧ
	AM2320_READ_WAIT_TREH			, //��ʼʱ80us�ߵ�ƽ��Ӧ
	AM2320_READ_WAIT_TDATA			, //���ݴ��俪ʼ
	AM2320_READ_WAIT_DATA			, //���ݴ���ʱDATA����Ӧ���Ͻ�����Ӧ
	AM2320_READ_END					, //��ȡ����
}AM2320_READ_STEP;

typedef enum
{
	AM2320_ERR_NONE					= 0x00,
	AM2320_ERR_TIM_OVER				,
}AM2320_ERROR;


typedef struct
{
	AM2320_ERROR		last_err;		//�쳣
	
	AM2320_READ_STEP	read_step;		//��ǰ�Ĳ���
	
	float				humi;			//��ǰ��ʪ��
	float				temp;			//��ǰ���¶�
	
	uint8_t				data[5];		//��ʱ������
	uint8_t				data_index;		//���浱ǰΪ�����ĵڼ�λ
}AM2320;



//=======================ȫ�ֱ�������================================

static TaskHandle_t			handle_am2320;
AM2320	am2320;

//=======================�ڲ���������================================
//AM2320���������
static void  Task_Am2320(void *p_arg);

//��ȡ��ʼ
static void Am2320_ReadStart(AM2320 *p_am2320);

//��ʱus
extern void delay_us(u32 nus);
//��ʱms
extern void delay_ms(uint16_t ms);






/* 
 * ������		��	Am2320_ParameterInit
 * ����			:	������ʼ����Ӳ����ʼ����������ʼ��
 * ����			:	��
 * ���			:	0 = �ɹ�
 * ˵��			:	��
 */
#define	TASK_AM2320_PRIO						2
#define	TASK_AM2320_STK_SIZE					200
void Am2320_ParameterInit(void)
{
	//IO�ڳ�ʼ��
	Am2320_Init();
	
	//��������
	xTaskCreate((TaskFunction_t )Task_Am2320,			//������
                (const char*    )"am2320",          	//��������
                (uint16_t       )TASK_AM2320_STK_SIZE,	//�����ջ��С
                (void*          )NULL,					//���ݸ��������Ĳ���
                (UBaseType_t    )TASK_AM2320_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&handle_am2320);   	//������
	
	DigitalLed_SetAnimation(ANIMATION_REFRESH);
	
	vTaskDelay(3000);
}



/* 
 * ������		��	Task_Am2320
 * ����			:	������
 * ����			:	void *p_arg
 * ���			:	��
 * ˵��			:	��
 */
static void  Task_Am2320(void *p_arg)
{
	while(1)
	{
		//��ʼ��ȡ�¶�
		Am2320_ReadStart(&am2320);
		
		//���ڶ�ȡ
		vTaskDelay(1000);
	}
}



/* 
 * ������		��	Am2320_SetTimOverValue
 * ����			:	AM2320���ó�ʱʱ��
 * ����			:	us:΢��
 * ���			:	��
 * ˵��			:	��
 */
static void  Am2320_SetTimOverValue(uint16_t us)
{
	//���ó�ʱֵ
	AM2320_SDA_TIM_TIMx->ARR = (us - 1);
}


/* 
 * ������		��	Am2320_SetTimOverValue
 * ����			:	AM2320���ó�ʱʱ��
 * ����			:	us:΢��
 * ���			:	��
 * ˵��			:	��
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
 * ������		��	Am2320_ReadByte
 * ����			:	AM2320��ȡ��ʼ
 * ����			:	p_am2320
 * ���			:	���ض�ȡ�����ֽ�����
 * ˵��			:	��
 */
static void Am2320_ReadStart(AM2320 *p_am2320)
{
	//��ʼ��ȡ
	p_am2320->read_step = AM2320_READ_START;
	p_am2320->last_err = AM2320_ERR_NONE;
	
	//��λ��ȡ��������
	p_am2320->data_index = 0;
	
	for(uint8_t i = 0; i < 5; i++){
		p_am2320->data[i] = 0x00;
	}
	
	//��������,��ʱ18ms
	Am2320_SdaOutMode();
	GPIO_ResetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
	delay_ms(15);
	
	//��������,��ʱ30us
	GPIO_SetBits(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin);
	delay_us(30);
	
	//����ȴ�����������Ӧ
	p_am2320->read_step = AM2320_READ_WAIT_TREH;
	
	//�����ȡ�׶�
	Am2320_SdaInMode();
	
	//����Ϊ�½�����Ӧ
	Am2320_SetTimPolarity(TIM_ICPolarity_Rising);
	
	//���ö�ʱ��
	AM2320_SDA_TIM_TIMx->CNT = 0;
	//���ó�ʱʱ��:500us(ʵ��Ϊ80usΪ��Ӧ)
	Am2320_SetTimOverValue(500);
}


/* 
 * ������		��	Am2320_ReadEnd
 * ����			:	AM2320��ȡ����
 * ����			:	p_am2320
 * ���			:	���ض�ȡ�����ֽ�����
 * ˵��			:	��
 */
static void Am2320_ReadEnd(AM2320 *p_am2320)
{
	static uint8_t count = 0;
	static enum{SHOW_TEMP = 0x00 , SHOW_HUMI = 0x01}show = SHOW_TEMP;
	static uint8_t err_count = 0;
	//��ʼ��ȡ
	p_am2320->read_step = AM2320_READ_END;
	
	if(p_am2320->last_err == AM2320_ERR_NONE){
		uint8_t humi_int = p_am2320->data[0];
		uint8_t humi_deci = p_am2320->data[1];
		
		uint8_t temp_int = p_am2320->data[2];
		uint8_t temp_deci = p_am2320->data[3];
		
		uint8_t check_sum = p_am2320->data[4];
		//������
		uint8_t calc = humi_int + humi_deci + temp_int + temp_deci;
		
		//����У��
		if(calc == check_sum){
			p_am2320->humi = (humi_int*256 + humi_deci)*0.1;
			p_am2320->temp = (temp_int*256 + temp_deci)*0.1;
			
			//��ȡ��ɽ���
			DigitalLed_SetAnimation(ANIMATION_NONE);
			
			//��ʾ�¶�
			if(show == SHOW_TEMP)
				DigitalLed_SetValue(am2320.temp);
			//��ʾʪ��
			else if(show == SHOW_HUMI)
				DigitalLed_SetValue(am2320.humi);
			
			//��һ��ʱ���л���ʾ����
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
	
	//���ģʽ
	Am2320_SdaOutMode();
}


/* 
 * ������		��	AM2320_SDA_TIM_IRQHandler
 * ����			:	��ʱ���ж�
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
uint8_t a = 0,b = 0;
void AM2320_SDA_TIM_IRQHandler(void)
{
	AM2320	*p_am2320 = &am2320;
	//��ʱ��
	if(TIM_GetITStatus(AM2320_SDA_TIM_TIMx, TIM_IT_Update) != RESET){
		//����ж�
		TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, TIM_IT_Update);	 
		
		p_am2320->last_err = AM2320_ERR_TIM_OVER;
		p_am2320->read_step = AM2320_READ_FREE;
		
		//��ȡ����
		Am2320_ReadEnd(p_am2320);	
	}
	
	//��ȡ�������ź�
	if(TIM_GetITStatus(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC)!=RESET){
		//����ж�
		TIM_ClearITPendingBit(AM2320_SDA_TIM_TIMx, AM2320_SDA_TIM_IT_CC);
		//���ö�ʱ��
		AM2320_SDA_TIM_TIMx->CNT = 0;
		
		switch(p_am2320->read_step)
		{	
		case AM2320_READ_WAIT_TREH://80us�͵�ƽ���,��ʼ80us�ߵ�ƽ
			//ȷ�϶�ȡ���˸ߵ�ƽ
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_SET){
				//����ȴ����俪ʼ
				p_am2320->read_step = AM2320_READ_WAIT_TDATA;
				                              
				//����Ϊ�����ز���
				Am2320_SetTimPolarity(TIM_ICPolarity_Falling);
				
				//���ó�ʱʱ��:80us: 80us�źŵ͵�ƽʱ��
				Am2320_SetTimOverValue(85+20);//�85us+20us����
				
				//��ȡ�����ݣ�ָʾ������ʾ��ʼ��ȡ����
				DigitalLed_SetAnimation(ANIMATION_DOT);
			}
			
		break;
		
		case AM2320_READ_WAIT_TDATA://80us�ߵ�ƽ��ɣ���ʼ50us�͵�ƽ���źŵ�ƽʱ��
			//ȷ�϶�ȡ���˸ߵ�ƽ
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_RESET){
				//��ʼ���ݴ���
				p_am2320->read_step = AM2320_READ_WAIT_DATA;
				
				//����Ϊ�����ز���
				Am2320_SetTimPolarity(TIM_ICPolarity_Falling);
				
				//���ó�ʱʱ��:150us: '0'/'1'�ĸߵ�ƽʱ��(75us) + 50us�ĵ͵�ƽʱ�� + 20us����
				Am2320_SetTimOverValue(55+75+20);
			}
		break;
		
		case AM2320_READ_WAIT_DATA:
			//ȷ�϶�ȡ���˵͵�ƽ
			if(GPIO_ReadInputDataBit(AM2320_SDA_GPIOx, AM2320_SDA_GPIO_Pin) == Bit_RESET){
				//��ȡ��ǰ�ĺ�ʱ
				uint16_t rm_cnt = 0;
				switch(AM2320_SDA_TIM_CC)
				{
				case TIM_Channel_1: rm_cnt = TIM_GetCapture1(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_2: rm_cnt = TIM_GetCapture2(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_3: rm_cnt = TIM_GetCapture3(AM2320_SDA_TIM_TIMx); break;
				case TIM_Channel_4: rm_cnt = TIM_GetCapture4(AM2320_SDA_TIM_TIMx); break;
				default:break;
				}
				
				//��ǰ��Ҫд���λ
				uint8_t byte = p_am2320->data_index / 8;
				uint8_t bit = p_am2320->data_index % 8;
				
				//�ж��Ƿ�Ϊ�͵�ƽʱ��
				if(rm_cnt > (50 + 68))//50us+'1'����Сʱ��68us
					p_am2320->data[byte] |= (uint8_t)0x01 << (7-bit);
				
				//�ж��Ƿ��ȡ��5���ֽڵ����ݶ�ȡ�����е�����
				if(++p_am2320->data_index >= 5*8){
					p_am2320->last_err = AM2320_ERR_NONE;
					//��ȡ����
					Am2320_ReadEnd(p_am2320);
				}
			}
		break;
		default:break;
		}
	}
}


