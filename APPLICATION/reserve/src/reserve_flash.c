#include "reserve_flash.h"

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//�ر������ж�
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//���������ж�
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
static uint16_t Reserve_ReadHalfWord(u32 addr)
{
	return *(vu16*)addr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
static void Reserve_WriteNoCheck(uint32_t addr,uint16_t *p_buf,uint16_t num)   
{ 			 		 
	u16 i;
	for(i = 0 ; i < num ; i ++)
	{
		FLASH_ProgramHalfWord(addr,p_buf[i]);
	    addr += 2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void Reserve_FlashWrite(uint32_t addr,uint16_t *p_buf,uint16_t num)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	
	if(addr<STM32_FLASH_BASE||(addr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE))){
		
		return;//�Ƿ���ַ
	}
	//���ж�
	INTX_DISABLE();
	
	FLASH_Unlock();						//����
	offaddr=addr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(num<=secremain)secremain=num;//�����ڸ�������Χ
	while(1) 
	{	
		Reserve_FlashRead(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=p_buf[i];	  
			}
			Reserve_WriteNoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else Reserve_WriteNoCheck(addr,p_buf,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(num==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	p_buf+=secremain;  	//ָ��ƫ��
			addr+=secremain;	//д��ַƫ��	   
		   	num-=secremain;	//�ֽ�(16λ)���ݼ�
			if(num>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=num;//��һ����������д����
		}	 
	};	
	FLASH_Lock();//����
	//�����ж�
	INTX_ENABLE();
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void Reserve_FlashRead(uint32_t addr,uint16_t *p_buf,uint16_t num)   	
{
	u16 i;
	for(i = 0 ; i < num ; i ++)
	{
		p_buf[i]=Reserve_ReadHalfWord(addr);//��ȡ2���ֽ�.
		addr += 2;//ƫ��2���ֽ�.	
	}
}




