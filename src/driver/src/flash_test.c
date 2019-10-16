#include "flash_test.h"
#include "stm32f10x_flash.h"
#include "stm32f10x.h"
#include "sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
/************************************************
*����ѧϰ��дflashĿ�����ڣ�
*1����flash����д�Ͷ��Ĳ���
*2����flash��ɶ԰��ֺ��ֵĶ�д����
*3��Ϊ����Ժ�������������׼��
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
#define FLASH_SAVE_ADDR1 0X0801E400     //��121kλ��
#define FLASH_SAVE_ADDR2 0X0801E800     //��122kλ��

#define FLASH_SAVE_ADDR3 0X0801EC00		//��123Kλ��
#define FLASH_SAVE_ADDR4 0X0801F000     //��124kλ��

#define FLASH_SAVE_ADDR5 0X0801F400     //��125kλ��

u16 save_data0[2048],save_data1[2048];  //�ƻ����һ���Զ���4K�ֽ�
u32 save_data2[1024];					//�ƻ����һ���Զ���4K�ֽ�

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
	flash_write_halfword(FLASH_SAVE_ADDR1,write_data0,10);//�԰��ֵ���ʽ������д��
//	flash_write_halfword(FLASH_SAVE_ADDR3,write_data0,strlen((char *)write_data0)-1);
//	flash_write_halfword(FLASH_SAVE_ADDR4,write_data0,strlen((char *)write_data0)-1);
//	printf("read data\r\n");
	flash_read_halfword(FLASH_SAVE_ADDR1,save_data0,1024);
	flash_read_word(FLASH_SAVE_ADDR1,save_data2,1024);
}

//��ȡĳ����ַ�µ�һ���ֽڵĺ���
u8 flash_readbites(u32 faddr)
{
	return *(vu8*)faddr;
}

//��ȡĳ����ַ�µ�һ�����ֺ���
u16 flash_readhalfword(u32 faddr)
{
	return *(vu16*)faddr; 
}
//��ȡĳ����ַ�µ�һ���ֺ���
u32 flash_readword(u32 faddr)
{
	return *(vu32*)faddr; 
}

//��ȡһ�����ȵ��ֽڷ�������pBuffer��
void flash_read_bites(u32 ReadAddr,char *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readbites(ReadAddr);//��ȡ�ֽ�.
		ReadAddr+=1;//ƫ��һ���ֽ�.	
	}
//	printf("length=%d\r\n",NumToRead);
//	for(j=0;j<NumToRead;j++)
//	{
//		printf("%c ",rBuffer[j]);
//	}
//	printf("\r\n");
}

//��ȡһ�����ȵ��ֽں��ӡ
void flash_rd_bites(u32 ReadAddr,u16 NumToRead)
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		printf("%x ",flash_readbites(ReadAddr));//��ȡ�ֽ�.
		ReadAddr+=1;//ƫ��һ���ֽ�.	
	}
	printf("\r\n");
}

//��ȡһ�����ȵİ��ַ�������pBuffer��
void flash_read_halfword(u32 ReadAddr,u16 *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readhalfword(ReadAddr);//��ȡ����.
		ReadAddr+=2;//ƫ�ư���.	
	}
	printf("length=%d\r\n",NumToRead);
	for(j=0;j<NumToRead;j++)
	{
		printf("%x ",rBuffer[j]);
	}
	printf("\r\n");
}

//��ȡһ�����ȵ��ַ�������pBuffer��
void flash_read_word(u32 ReadAddr,u32 *rBuffer,u16 NumToRead)   	
{
	u16 i,j;
	for(i=0;i<NumToRead;i++)
	{
		rBuffer[i]=flash_readword(ReadAddr);//��ȡ�ֽ�.
		ReadAddr+=4;//ƫ��һ���ֽ�.	
	}
	printf("length=%d\r\n",NumToRead);
	for(j=0;j<NumToRead;j++)
	{
		printf("%x ",rBuffer[j]);
	}
	printf("\r\n");
}

//�԰��ֵĿռ�д���ֽں��� 
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

//д��һ�����ȵİ��ֵ����ݵ�flash��
void flash_write_halfword(u32 WriteAddr,u16 wBuffer[],u16 NumToWrite)
{
	float i=0,j=0,m,n,x;
	int erase_begin;
	int erase_end;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	
	//��Ҫд��ĳ������ж�Ҫ�߳���ҳ��
	i=(WriteAddr-FLASH_BASE)*1.0/FLASH_PAGE;
	j=(WriteAddr+NumToWrite*2-FLASH_BASE)*1.0/FLASH_PAGE;
//	printf("test_i=%f   test_j=%f\r\n",i,j);
	if((i-(int)i)>=0)//û�н�����������
	{
		m=(int)i; 
	}
	else			//��������������
	{
		m=(int)i-1;
	}
	
	if((j-(int)j)>=0)//û�н�����������
	{
		n=(int)j; 
	}
	else			//��������������
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
	
	FLASH_Lock();//����
}

//д��һ�����ȵ��ֵ����ݵ�flash��
void flash_write_word(u32 WriteAddr,u32 wBuffer[],u16 NumToWrite)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_BSY|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//���������־��æ��־����д�����־��ҳ��д������־
	
//	flash_status=FLASH_ErasePage();
}







