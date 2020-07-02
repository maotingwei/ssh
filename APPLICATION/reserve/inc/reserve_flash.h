#ifndef __RESERVE_FLASH_H__
#define __RESERVE_FLASH_H__ 
#include"stm32f10x.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 256	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 1          //ʹ��FLASHд��(0��������;1��ʹ��)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE  0x08000000 	//STM32 FLASH����ʼ��ַ

void Reserve_FlashWrite(uint32_t addr,uint16_t *p_buf,uint16_t num);//��ָ����ַ��ʼд��ָ�����ȵ�����
void Reserve_FlashRead(uint32_t addr,uint16_t *p_buf,uint16_t num); //��ָ����ַ��ʼ����ָ�����ȵ�����	

#endif

















