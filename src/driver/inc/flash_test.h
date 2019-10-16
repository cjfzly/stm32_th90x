#include "sys.h"


//用于验证对flash的读写操作
#ifndef _FLASH_TEST_H
#define _FLASH_TEST_H

//设置MCU的容量
#define FLASH_SIZE 128


void flash_test(void);
void flash_read_halfword(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void flash_read_bites(u32 ReadAddr,char *rBuffer,u16 NumToRead);
void flash_read_word(u32 ReadAddr,u32 *rBuffer,u16 NumToRead);
void flash_write_halfword(u32 WriteAddr,u16 *wBuffer,u16 NumToWrite);
void flash_rd_bites(u32 ReadAddr,u16 NumToRead);

#endif





