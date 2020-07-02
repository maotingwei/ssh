
#include "includes.h"
#include "led_bsp.h"
#include "math.h"
#include "task.h"

//===========================������ú궨��=====================================

uint8_t DIGITAL_LED_CODE[] = 
{// 0	 1	  2	   3	4	 5	  6	   7	8	 9
	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};

typedef struct
{
	//��Ҫ��ʾ������
	float digital_data;
	//��ʾ��С����λ��
	uint8_t decimal_number;
	//��ʾ�����������
	uint8_t	led_num;
	
	//����״̬
	uint8_t animation;
}DIGITAL_LED_PARAMETER;


//===========================���ȫ�ֱ�������=====================================

DIGITAL_LED_PARAMETER		digital_parameter;





//===========================����ڲ���������=====================================
//������ʾLED������
static void  Task_DigitalLed(void *p_arg);
//����LED��������
static void DigitalLed_SendData(uint8_t data);
//��ȡ����ֵ:��app.c�ж���
extern float Get_Absolute(float value);


/* 
 * ������		��	Task_DigitalLED
 * ����			:	����LED��ʾ����
 * ����			:	void *p_arg
 * ���			:	��
 * ˵��			:	��
 */
#define	TASK_LED_PRIO						5
#define	TASK_LED_STK_SIZE					200
static TaskHandle_t			handle_led;;	
void DigitalLed_ParameterInit(void)
{
	//��ʼ��Ӳ��
	DigitalLed_Init();
	
	//��������
	xTaskCreate((TaskFunction_t )Task_DigitalLed,	//������
                (const char*    )"LED",          	//��������
                (uint16_t       )TASK_LED_STK_SIZE,	//�����ջ��С
                (void*          )NULL,				//���ݸ��������Ĳ���
                (UBaseType_t    )TASK_LED_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&handle_led);   	//������
	vTaskDelay(10);		
}

/* 
 * ������		��	DigitalLed_SetValue
 * ����			:	��������LED��ʾֵ
 * ����			:	value:��ֵ
 * ���			:	��
 * ˵��			:	��
 */
void DigitalLed_SetValue(float value)
{
	digital_parameter.digital_data = value;
}


/* 
 * ������		��	DigitalLed_SetAnimation
 * ����			:	��������LED����״̬
 * ����			:	state��״̬
 * ���			:	��
 * ˵��			:	��
 */
void DigitalLed_SetAnimation(ANIMATION animation)
{
	digital_parameter.animation = animation;
}

/* 
 * ������		��	Task_DigitalLED
 * ����			:	����LED��ʾ����
 * ����			:	void *p_arg
 * ���			:	��
 * ˵��			:	��
 */
static void Task_DigitalLed(void *p_arg)
{
	float float_data;//�Ŵ���
	
	//С������ʾ��λ
	digital_parameter.decimal_number = 1;
	//����ʾλ��3
	digital_parameter.led_num = 3;
	
	//��ʼ����ʾ������
	digital_parameter.digital_data = 0;
	
	uint16_t animation_count = 0;
	uint8_t animation_step = 0;
	while(1)
	{
		if(digital_parameter.animation == ANIMATION_NONE
		|| digital_parameter.animation == ANIMATION_DOT){
			//��ֵ����
			if(Get_Absolute(digital_parameter.digital_data) >= 100){
				for(uint8_t i = digital_parameter.led_num ; i > 0 ; i --){
					DigitalLed_SendData(~0x40); 
					DigitalLed_SendData(0x01 << (i-1));
					
					//��������Ч
					GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					vTaskDelay(1);
					GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					
					//��ʾʱ��
					vTaskDelay(1);	
				}
			}
			//������ֵ
			else{
				float_data = digital_parameter.digital_data;
				//��Ҫ��ʾ������С������������Ҫ��ʾ��λ��ǰ��
				for(uint8_t i = 0 ; i < digital_parameter.decimal_number ; i++){
					float_data *= 10;
				}
				//��������
				uint16_t remainder = float_data;
				
				//��ȥ����־
				uint8_t no_zero = RESET;
				//��ʾ��Ӧ������
				for(uint8_t i = digital_parameter.led_num ; i > 0 ; i --){
					//ȡ����
					remainder = remainder % (uint16_t)(pow(10,i));
					//ȡ��Ҫ��ʾ��λ
					uint8_t data = (uint8_t)(remainder / pow(10,i-1));
					//��λΪ0�Ļ��ر���ʾ
					if(no_zero == RESET && data == 0 && i > (digital_parameter.decimal_number+1)){
						data = 0xFF;
					}
					else{
						no_zero = SET;
						//ȡ����ģ
						data = DIGITAL_LED_CODE[data];
						//����Ƿ���Ҫ�����С����
						if(i == (digital_parameter.decimal_number+1)){
							data &= 0x7F;
						}
					}
					//д������
					DigitalLed_SendData(data);
					DigitalLed_SendData(0x01 << (i-1));
					
					//��������Ч
					GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					vTaskDelay(1);
					GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
					
					//��ʾʱ��
					vTaskDelay(1);	
				}
			}
		}
		
		//������ʾ
		switch(digital_parameter.animation){
		case ANIMATION_NONE:break;
		case ANIMATION_DOT:
			//д������
			DigitalLed_SendData(0x7f);
			DigitalLed_SendData(0x01 << (4-1));
			
			//��������Ч
			GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
			vTaskDelay(1);
			GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
		break;
		case ANIMATION_REFRESH:
			uint8_t data[4];
			//���²���ʵ����LED��ˮ�ﵽˢ�µ��Ӿ�Ч��
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
				//��������Ч
				GPIO_ResetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
				vTaskDelay(1);
				GPIO_SetBits(DIGITAL_LED_RSLK_GPIOx , DIGITAL_LED_RSLK_GPIO_Pin);
			}
			//ÿһ��ʱ����л�����һ������ﵽ��ˮЧ��
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
 * ������		��	DigitalLed_SendData
 * ����			:	����LED��������
 * ����			:	data
 * ���			:	��
 * ˵��			:	��
 */
static void DigitalLed_SendData(uint8_t data)
{
	for(uint8_t i = 8 ; i >=1 ; i--){
		//����λ
		if(data&0x80)
			GPIO_SetBits(DIGITAL_LED_DIO_GPIOx , DIGITAL_LED_DIO_GPIO_Pin);
		else
			GPIO_ResetBits(DIGITAL_LED_DIO_GPIOx , DIGITAL_LED_DIO_GPIO_Pin);
		data <<= 1;
		//ʱ��������
		GPIO_ResetBits(DIGITAL_LED_SCK_GPIOx , DIGITAL_LED_SCK_GPIO_Pin);
		GPIO_SetBits(DIGITAL_LED_SCK_GPIOx , DIGITAL_LED_SCK_GPIO_Pin);
	}
}






