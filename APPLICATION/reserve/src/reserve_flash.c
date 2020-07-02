#include "reserve_flash.h"

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
static uint16_t Reserve_ReadHalfWord(u32 addr)
{
	return *(vu16*)addr; 
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
static void Reserve_WriteNoCheck(uint32_t addr,uint16_t *p_buf,uint16_t num)   
{ 			 		 
	u16 i;
	for(i = 0 ; i < num ; i ++)
	{
		FLASH_ProgramHalfWord(addr,p_buf[i]);
	    addr += 2;//地址增加2.
	}  
} 
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
void Reserve_FlashWrite(uint32_t addr,uint16_t *p_buf,uint16_t num)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	
	if(addr<STM32_FLASH_BASE||(addr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE))){
		
		return;//非法地址
	}
	//关中断
	INTX_DISABLE();
	
	FLASH_Unlock();						//解锁
	offaddr=addr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(num<=secremain)secremain=num;//不大于该扇区范围
	while(1) 
	{	
		Reserve_FlashRead(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=p_buf[i];	  
			}
			Reserve_WriteNoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区  
		}else Reserve_WriteNoCheck(addr,p_buf,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(num==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	p_buf+=secremain;  	//指针偏移
			addr+=secremain;	//写地址偏移	   
		   	num-=secremain;	//字节(16位)数递减
			if(num>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			else secremain=num;//下一个扇区可以写完了
		}	 
	};	
	FLASH_Lock();//上锁
	//开启中断
	INTX_ENABLE();
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void Reserve_FlashRead(uint32_t addr,uint16_t *p_buf,uint16_t num)   	
{
	u16 i;
	for(i = 0 ; i < num ; i ++)
	{
		p_buf[i]=Reserve_ReadHalfWord(addr);//读取2个字节.
		addr += 2;//偏移2个字节.	
	}
}




