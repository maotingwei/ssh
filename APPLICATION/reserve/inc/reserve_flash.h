#ifndef __RESERVE_FLASH_H__
#define __RESERVE_FLASH_H__ 
#include"stm32f10x.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//用户根据自己的需要设置
#define STM32_FLASH_SIZE 256	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1          //使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE  0x08000000 	//STM32 FLASH的起始地址

void Reserve_FlashWrite(uint32_t addr,uint16_t *p_buf,uint16_t num);//从指定地址开始写入指定长度的数据
void Reserve_FlashRead(uint32_t addr,uint16_t *p_buf,uint16_t num); //从指定地址开始读出指定长度的数据	

#endif

















