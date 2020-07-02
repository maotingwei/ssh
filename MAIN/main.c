#include "includes.h"
#include "task.h"
#include "timers.h"


//任务优先级
#define START_TASK_PRIO				1
//任务堆栈大小	
#define START_STK_SIZE 				128  
//任务句柄
TaskHandle_t	StartTask_Handler;
//任务服务函数
extern void xTaskStart_Handler(void *pvParameters);

//系统时钟配置
void xTask_SysTickConfig(uint32_t os_rate);


#if PROJ_RELEASE_EN
//独立看门狗定时器句柄
static TimerHandle_t 	handle_wdg;	
//独立看门狗初始化
void IWDG_Init(u8 prer,u16 rlr);
//独立看门狗喂狗
void IWDG_Feed(void);
//独立看门狗定时器回调函数
static void IWDG_TmrCallback(TimerHandle_t xTimer);
#endif


/* 
 * 函数名：main
 * 描述  : 主函数
 * 输入  ：无
 * 输出  : 无
 */
int main(void)
{
	//设置中断向量表的位置
	#if PROJ_RELEASE_EN
	SCB->VTOR = IAP_APP_EXECUTION_FLASH_ADDR;
	#else
	SCB->VTOR = 0x8000000;
	#endif
	//设置中断组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	//配置系统时钟
	xTask_SysTickConfig(configTICK_RATE_HZ);
	//创建开始任务
    xTaskCreate((TaskFunction_t )xTaskStart_Handler,	//任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄
	#if PROJ_RELEASE_EN
	//BOOTLOAD中启用了独立看门狗，所以在APP中喂狗
	IWDG_Feed();
	//创建喂狗定时器
    handle_wdg=xTimerCreate((const char*		)"IWDG",
							(TickType_t			)500,
							(UBaseType_t		)pdTRUE,
							(void*				)1,
							(TimerCallbackFunction_t)IWDG_TmrCallback); 
	xTimerStart(handle_wdg,0);//开启周期定时器
	#endif
    vTaskStartScheduler();//开启任务调度
}


/* 
 * 函数名：SysTick_Config
 * 描述  : 系统时钟配置
 * 输入  ：os_rate：系统的速度hz
 * 输出  : 无
 */
void xTask_SysTickConfig(uint32_t os_rate)
{
	uint32_t reload;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);//选择外部时钟  HCLK
	reload=SystemCoreClock/1000000;				//每秒钟的计数次数 单位为M  
	reload*=1000000/os_rate;					//根据configTICK_RATE_HZ设定溢出时间
												//reload为24位寄存器,最大值:16777216,在72M下,约合0.233s左右	
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;   	//开启SYSTICK中断
	SysTick->LOAD=reload; 						//每1/configTICK_RATE_HZ秒中断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;   	//开启SYSTICK    
}	


/* 
 * 函数名：SysTick_Handler
 * 描述  : SysTick中断处理
 * 输入  ：os_rate：系统的速度hz
 * 输出  : 无
 */
extern void xPortSysTickHandler( void );
void SysTick_Handler(void)
{	
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
    {
        xPortSysTickHandler();	
    }
}

#if PROJ_RELEASE_EN
/* 
 * 函数名		：	IWDG_Init
 * 描述			:	独立看门狗初始化
 * 输入			:	prer:分频数:0~7(只有低3位有效!)
					rlr:重装载寄存器值:低11位有效.
 * 输出			:	无
 * 说明			:	时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
 */
void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	
	IWDG_SetPrescaler(prer);  //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  //设置IWDG重装载值
	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
	
	IWDG_Enable();  //使能IWDG
}


/* 
 * 函数名		：	IWDG_Feed
 * 描述			:	独立看门狗喂狗
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload				   
}


/* 
 * 函数名		：	IWDG_TmrCallback
 * 描述			:	看门狗定时器回调函数
 * 输入			:	无
 * 输出			:	无
 * 说明			:	无
 */
static void IWDG_TmrCallback(TimerHandle_t xTimer)
{
	//喂狗
	IWDG_Feed();
}

#endif

