#include "flash_test.h"
#include "stm32f10x_flash.h"
#include "stm32f10x.h"
#include "sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
/************************************************
*本次学习读写flash目的在于：
*1、对flash进行写和读的操作
*2、对flash完成对半字和字的读写操作
*3、为完成以后升级程序作出准备
*
*
*
*
*************************************************/

#if FLASH_SIZE<256
#define FLASH_PAGE 0x400
#else
#define FLASH_PAGE 0x800
#endif



#define FLASH_SAVE_ADDR  0X0801E000
#define FLASH_SAVE_ADDR1 0X0801E400     //第121k位置
#define FLASH_SAVE_ADDR2 0X0801E800     //第122k位置

#define FLASH_SAVE_ADDR3 0X0801EC00		//第123K位置
#define FLASH_SAVE_ADDR4 0X0801F000     //第124k位置

#define FLASH_SAVE_ADDR5 0X0801F400     //第125k位置

u16 save_data0[2048],save_data1[2048];  //计划最大一次性读出4K字节
u32 save_data2[1024];					//计划最大一次性读出4K字节

u16 write_data0[10]={0x1122,0x2233,0x0012,0x0012,0x0012,0x0012,0x0012,0x0012,0x0012,0x0013};

void flash_read_halfword(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void flash_read_bites(u32 ReadAddr,char *rBuffer,u16 NumToRead);
void flash_read_word(u32 ReadAddr,u32 *rBuffer,u16 NumToRead);
void flash_write_halfword(u32 WriteAddr,u16 *wBuffer,u16 NumToWrite);
FLASH_Status flash_status=FLASH_COMPLETE;


void flash_test(void)
{
	int i=0;
	printf("write data=%d\r\n",strlen((char *)write_data0));
	flash_write_halfword(FLASH_SAVE_ADDR1,write_data0,10);//以半字的形式来进行写入
//	flash_write_halfword(FLASH_SAVE_ADDR3,write_data0,strlen((char *)write_data0)-1);
//	flash_write_halfword(FLASH_SAVE_ADDR4,write_data0,strlen((char *)write_data0)-1);
//	printf("read data\r\n");
	flash_read_halfword(FLASH_SAVE_ADDR1,save_data0,1024);
	flash_read_word(FLASH_SAVE_ADDR1,save_data2,1024);
}

//读取某个地址下的一个字节的函数
u8 flash_readbites(u32 faddr)
{
	return *(vu8*)faddr;
}

//读取某个地址下的一个半字函数
u16 flash_readhalfword(u32 faddr)
{
	return *(vu16*)faddr; 
}
//读取某个地址下的一个字函数
u32 flash_readword(u32 faddr)
{
	return *(vu32*)faddr; 
}

//读取一定长度的字节放在数组pBuffer中
void flash_read_bites(u32 ReadAddr,char *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readbites(ReadAddr);//读取字节.
		ReadAddr+=1;//偏移一个字节.	
	}
//	printf("length=%d\r\n",NumToRead);
//	for(j=0;j<NumToRead;j++)
//	{
//		printf("%c ",rBuffer[j]);
//	}
//	printf("\r\n");
}

//读取一定长度的字节后打印
void flash_rd_bites(u32 ReadAddr,u16 NumToRead)
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		printf("%x ",flash_readbites(ReadAddr));//读取字节.
		ReadAddr+=1;//偏移一个字节.	
	}
	printf("\r\n");
}

//读取一定长度的半字放在数组pBuffer中
void flash_read_halfword(u32 ReadAddr,u16 *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readhalfword(ReadAddr);//读取半字.
		ReadAddr+=2;//偏移半字.	
	}
	printf("length=%d\r\n",NumToRead);
	for(j=0;j<NumToRead;j++)
	{
		printf("%x ",rBuffer[j]);
	}
	printf("\r\n");
}

//读取一定长度的字放在数组pBuffer中
void flash_read_word(u32 ReadAddr,u32 *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readword(ReadAddr);//读取字节.
		ReadAddr+=4;//偏移一个字节.	
	}
	printf("length=%d\r\n",NumToRead);
	for(j=0;j<NumToRead;j++)
	{
		printf("%x ",rBuffer[j]);
	}
	printf("\r\n");
}

//以半字的空间写入字节函数 
void flash_write_u16(u32 WriteAddr,u16 wBuffer[],u16 NumToWrite)   
{ 	
	int i=0;
//	printf("buffer_size=%d\r\n",strlen((char *)wBuffer));
	for(i=0;i<NumToWrite;i++)
	{
		flash_status=FLASH_ProgramHalfWord(WriteAddr,wBuffer[i]);
		while(flash_status!=FLASH_COMPLETE);
		WriteAddr+=2;
//		printf("break%d\r\n",i);
	}
} 

//写入一定长度的半字的数据到flash中
void flash_write_halfword(u32 WriteAddr,u16 wBuffer[],u16 NumToWrite)
{
	float i=0,j=0,m,n,x;
	int erase_begin;
	int erase_end;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	//由要写入的长度来判读要檫除的页数
	i=(WriteAddr-FLASH_BASE)*1.0/FLASH_PAGE;
	j=(WriteAddr+NumToWrite*2-FLASH_BASE)*1.0/FLASH_PAGE;
//	printf("test_i=%f   test_j=%f\r\n",i,j);
	if((i-(int)i)>=0)//没有进行四舍五入
	{
		m=(int)i; 
	}
	else			//进行了四舍五入
	{
		m=(int)i-1;
	}
	
	if((j-(int)j)>=0)//没有进行四舍五入
	{
		n=(int)j; 
	}
	else			//进行了四舍五入
	{
		n=(int)j-1;
	}  
//	printf("pages=%f num=%f m=%f n=%f\r\n",i,j,m,n);
	for(x=m;x<=n;x++)
	{
		flash_status=FLASH_ErasePage(FLASH_BASE+FLASH_PAGE*x);
		while(flash_status!=FLASH_COMPLETE);
//		printf("erase_page=%f\r\n",x);
	}
	
//	printf("write_length=%d\r\n",NumToWrite);
	flash_write_u16(WriteAddr,wBuffer,NumToWrite);
	
	FLASH_Lock();//上锁
}

//写入一定长度的字的数据到flash中
void flash_write_word(u32 WriteAddr,u32 wBuffer[],u16 NumToWrite)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_BSY|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//清除结束标志、忙标志、编写错误标志、页面写保护标志
	
//	flash_status=FLASH_ErasePage();
}







