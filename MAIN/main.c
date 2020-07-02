#include "includes.h"
#include "task.h"
#include "timers.h"


//�������ȼ�
#define START_TASK_PRIO				1
//�����ջ��С	
#define START_STK_SIZE 				128  
//������
TaskHandle_t	StartTask_Handler;
//���������
extern void xTaskStart_Handler(void *pvParameters);

//ϵͳʱ������
void xTask_SysTickConfig(uint32_t os_rate);


#if PROJ_RELEASE_EN
//�������Ź���ʱ�����
static TimerHandle_t 	handle_wdg;	
//�������Ź���ʼ��
void IWDG_Init(u8 prer,u16 rlr);
//�������Ź�ι��
void IWDG_Feed(void);
//�������Ź���ʱ���ص�����
static void IWDG_TmrCallback(TimerHandle_t xTimer);
#endif


/* 
 * ��������main
 * ����  : ������
 * ����  ����
 * ���  : ��
 */
int main(void)
{
	//�����ж��������λ��
	#if PROJ_RELEASE_EN
	SCB->VTOR = IAP_APP_EXECUTION_FLASH_ADDR;
	#else
	SCB->VTOR = 0x8000000;
	#endif
	//�����ж���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	//����ϵͳʱ��
	xTask_SysTickConfig(configTICK_RATE_HZ);
	//������ʼ����
    xTaskCreate((TaskFunction_t )xTaskStart_Handler,	//������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������
	#if PROJ_RELEASE_EN
	//BOOTLOAD�������˶������Ź���������APP��ι��
	IWDG_Feed();
	//����ι����ʱ��
    handle_wdg=xTimerCreate((const char*		)"IWDG",
							(TickType_t			)500,
							(UBaseType_t		)pdTRUE,
							(void*				)1,
							(TimerCallbackFunction_t)IWDG_TmrCallback); 
	xTimerStart(handle_wdg,0);//�������ڶ�ʱ��
	#endif
    vTaskStartScheduler();//�����������
}


/* 
 * ��������SysTick_Config
 * ����  : ϵͳʱ������
 * ����  ��os_rate��ϵͳ���ٶ�hz
 * ���  : ��
 */
void xTask_SysTickConfig(uint32_t os_rate)
{
	uint32_t reload;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);//ѡ���ⲿʱ��  HCLK
	reload=SystemCoreClock/1000000;				//ÿ���ӵļ������� ��λΪM  
	reload*=1000000/os_rate;					//����configTICK_RATE_HZ�趨���ʱ��
												//reloadΪ24λ�Ĵ���,���ֵ:16777216,��72M��,Լ��0.233s����	
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;   	//����SYSTICK�ж�
	SysTick->LOAD=reload; 						//ÿ1/configTICK_RATE_HZ���ж�һ��	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;   	//����SYSTICK    
}	


/* 
 * ��������SysTick_Handler
 * ����  : SysTick�жϴ���
 * ����  ��os_rate��ϵͳ���ٶ�hz
 * ���  : ��
 */
extern void xPortSysTickHandler( void );
void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//ϵͳ�Ѿ�����
    {
        xPortSysTickHandler();	
    }
}

#if PROJ_RELEASE_EN
/* 
 * ������		��	IWDG_Init
 * ����			:	�������Ź���ʼ��
 * ����			:	prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
					rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
 * ���			:	��
 * ˵��			:	ʱ�����(���):Tout=((4*2^prer)*rlr)/40 (ms).
 */
void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����
	
	IWDG_SetPrescaler(prer);  //����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ64
	
	IWDG_SetReload(rlr);  //����IWDG��װ��ֵ
	
	IWDG_ReloadCounter();  //����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
	
	IWDG_Enable();  //ʹ��IWDG
}


/* 
 * ������		��	IWDG_Feed
 * ����			:	�������Ź�ι��
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload				   
}


/* 
 * ������		��	IWDG_TmrCallback
 * ����			:	���Ź���ʱ���ص�����
 * ����			:	��
 * ���			:	��
 * ˵��			:	��
 */
static void IWDG_TmrCallback(TimerHandle_t xTimer)
{
	//ι��
	IWDG_Feed();
}

#endif

