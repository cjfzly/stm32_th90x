/*
 this is written byTX, 2012.3.26
2012.3.26: 缓存多个命令？？
2013.12.25，V1.1更新，使可移植性更强
使用方法：
1、在中断中调用处理程序isr_uart(buf)
2、在cmd_tbl中更新命令，其中每二个参数是调用的程序，第一个参数是操作命令，最后一个是帮助
3、工程包含command.c文件
*/
#include "stdio.h"   //byTX
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "command.h"
#include "stm32f10x.h"
#include "delay_time.h"
#include "config_pin.h"
#include "adc.h"
#include "flash.h"
#include "flash_test.h"

#define str2num(x) atoi((char const *)(x))

uint8_t do_ft (int argc, uint8_t *argv[]);
uint8_t sc_ft (int argc, uint8_t *argv[]);
uint8_t send_save_pos_data (int argc, uint8_t *argv[]);
uint8_t printf_head_data (int argc, uint8_t *argv[]);
uint8_t printf_addr_pos_data (int argc, uint8_t *argv[]);
uint8_t do_cpu_reset(int argc, uint8_t *argv[]);
uint8_t io_reset_out (int argc, uint8_t *argv[]);
//extern void get_acd_data(uint8_t chanl_num);
uint8_t f_change(int argc, uint8_t *argv[]);
void version_trance(uint8_t *version,u8 version_first[],u8 version_second[]);

//#pragma language=extended
//#pragma section="Cmd_section"
uint8_t cmd_index=0;//命令索引
uint8_t cmd_length=0;//命令长度
uint8_t cmd_buf[CFG_MAX_BUF];//命令缓存
uint8_t cmd_buf_tbl[CFG_MAX_HISTORY][CFG_MAX_BUF];//用于存储历史命令
int cmd_index_tbl=0;//历史命令索引
int cmd_index_tbl_length=0;
char version_string[]="Control Board V1.0.0 20190619";  //bycjf

uint8_t usart1_buf=0;
uint8_t usart1_rec_data_flag=0;

//电压值
u16 adcx;


//频率写入切换标志位
char f_chanle=0;

#define FLASH_SAVE_ADDR  0X0801E000		//120kflash后开始保存系统参数,预留8k作为系统参数存储单元
#define FLASH_SAVE_ADDR1 0X0801E400     //第121k位置
#define FLASH_SAVE_ADDR2 0X0801E800     //第122k位置

#define FLASH_SAVE_ADDR3 0X0801EC00		//第123K位置
#define FLASH_SAVE_ADDR4 0X0801F000     //第124k位置

#define FLASH_SAVE_ADDR5 0X0801F400     //第125k位置

void check_uart_data(void)
{
   
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    usart1_buf = USART_ReceiveData(USART1);
    usart1_rec_data_flag=1;
    //isr_uart(temp);
    //USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    
  }

  else if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
  {
      USART_ReceiveData(USART1);      
  } 
}
	
void USART1_REC_DATA(void)
{
    //串口数据处理
    if(usart1_rec_data_flag)
    {         
         isr_uart(usart1_buf);
         usart1_rec_data_flag=0;
    }
}

bool isDecStr(uint8_t Str)
{
  if(Str>='0' && Str<='9')
    return true;
  return false;
}
bool isHexStr(uint8_t Str)
{
  if(Str>='0' && Str<='9')
    return true;
  if(Str>='A' && Str<='F')
    return true;
  if(Str>='a' && Str<='f')
    return true;  
  return false;
}

//将字符串转换成数字，支持10进制，16进制
uint32_t str_to_u(uint8_t *pcStr)
{
  uint32_t i;
  uint32_t ui32Val=0;
  if(pcStr[0]=='0' && ((pcStr[1]=='x') || (pcStr[1]=='X')))
  {//16进制
    i=2;
    while(isHexStr(pcStr[i]))
    {
      ui32Val*=16;
      if(pcStr[i]>='A' && pcStr[i]<='F')
        ui32Val+=(pcStr[i]-'A'+10);
      else if(pcStr[i]>='a' && pcStr[i]<='f')
        ui32Val+=(pcStr[i]-'a'+10);
      else
        ui32Val+=(pcStr[i]-'0');
      i++;
    }
  }
  else
  {
    i=0;
    while(isDecStr(pcStr[i]))
    {
      ui32Val*=10;
      ui32Val+=(pcStr[i]-'0');
      i++;
    }      
  }
  return ui32Val;
}
    

//解析版本号函数 TH902_V1.1.1_20190626
void version_trance(uint8_t *version,uint8_t version_first[],uint8_t version_second[])
{
	int i=0,j=0;
	int rember=0;//用于记住'_'的个数
//	printf("len=%d\r\n",strlen(version));
	for(i=0;i<strlen((char *)version);i++)
	{
		if(version[i]=='_')
		{
			rember+=1;
		}
		if(rember==0||rember==1)
		{
			version_first[i]=version[i];
		}		
		else if(rember==2)
		{			
			version_second[j]=version[i];
			j++;
			if(j==strlen((char *)version)-1)
			{
				return;
			}
		}
	}
}

uint8_t io_low_hig_out (int argc, uint8_t *argv[])
{
	 GPIO_SetBits(GPIOB,GPIO_Pin_11);
	 return 0;
}



uint8_t check_5v (int argc, uint8_t *argv[])
{
	//2.28 增加 WATCH_5V_ADC_SW PB12 打开采集开关，后关闭
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	delay_ms(10);
    adcx=adc_value(Get_Adc_Average2(ADC_Channel_0,10));
	printf("watch power v5:%dmv\r\n",adcx);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);	
	return 0;
}

//vbat
uint8_t check_vbat (int argc, uint8_t *argv[])
{
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	delay_ms(10);
    adcx=adc_value(Get_Adc_Average2(ADC_Channel_1,10));
	printf("watch power vbat:%dmv\r\n",adcx);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);	
	 return 0;
}
uint8_t check_v33 (int argc, uint8_t *argv[])
{
	//2.28 增加 WATCH_V3.3_ADC_SW PB14 打开采集开关，后关闭
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	delay_ms(10);
    adcx=adc_value(Get_Adc_Average2(ADC_Channel_9,10));
	printf("watch power v3.3:%dmv\r\n",adcx);
	delay_ms(5);
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);	
	 return 0;
}
uint8_t check_b_vbat (int argc, uint8_t *argv[])
{
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	delay_ms(10);
    adcx=adc_value(Get_Adc_Average2(ADC_Channel_1,10));
	printf("watch power vbat:%dmv\r\n",adcx);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);	
	 return 0;
}

//手表VDD电压检测命令 
uint8_t check_vdd (int argc, uint8_t *argv[])
{
	//2.28 增加 WATCH_VDD_ADC_SW PB15 打开采集开关，后关闭
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	delay_ms(10);
    adcx=adc_value(Get_Adc_Average2(ADC_Channel_8,10));
	printf("watch power vdd:%dmv\r\n",adcx);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);	
	return 0;
}
	

//复位命令 PB9
uint8_t io_low_hig_out_rst (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_9);
	}
	
	 return 0;
}

//PB9
uint8_t io_low_hig_dely_out_rst (int argc, uint8_t *argv[])
{		
  GPIO_SetBits(GPIOB,GPIO_Pin_9);
	delay_ms(200);
  GPIO_ResetBits(GPIOB,GPIO_Pin_9);	
	
	return 0;
}

//KEY1命令 PB8
uint8_t io_key1_out (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_8);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_8);
	}
	
	 return 0;
}

uint8_t io_key_out (int argc, uint8_t *argv[])
{
	uint8_t  in_data_key=0;
	uint16_t in_data_dely=0;
	
  if(argc!=3)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data_key=(uint8_t)str_to_u(argv[1]);
	in_data_dely=(uint16_t)str_to_u(argv[2]);
	
	if(in_data_dely==0) return 0;
	
	switch(in_data_key)
	{
		case 1: GPIO_SetBits(GPIOB,GPIO_Pin_8);   break;
		case 2: GPIO_SetBits(GPIOA,GPIO_Pin_12);   break; 
		case 3: GPIO_SetBits(GPIOA,GPIO_Pin_11);   break;
		case 4: GPIO_SetBits(GPIOC,GPIO_Pin_13);   break;
		//case 5: GPIO_SetBits(GPIOB,GPIO_Pin_8);   break;		
	}
	
  delay_ms(in_data_dely);
	
	switch(in_data_key)
	{
		case 1: GPIO_ResetBits(GPIOB,GPIO_Pin_8);   break;
		case 2: GPIO_ResetBits(GPIOA,GPIO_Pin_12);   break; 
		case 3: GPIO_ResetBits(GPIOA,GPIO_Pin_11);   break;
		case 4: GPIO_ResetBits(GPIOC,GPIO_Pin_13);   break;
		//case 5: GPIO_ResetBits(GPIOB,GPIO_Pin_8);   break;		
	}	
	 return 0;
}


//电池负与GND连通命令  PB5
//2019.2.28 改为ACTIVE_SW控制 PB6
uint8_t io_low_hig_out_bat1 (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_6);
	}	
	 return 0;
}

//BGND与GND连通命令  PB3
//2019.2.28 增加控制指令
uint8_t io_board_to_gnd(int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_3);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_3);
	}	
	 return 0;
}

//电池地与GND连通命  PB4
uint8_t io_low_hig_out_bat2 (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_4);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_4);
	}	
	 return 0;
}

//LED打开   "PA15 LED3" "PA4 lED1"  "PB11 lED2"
//2019.2.28 "PA4 lED1" 改为 "PC15 lED1" 
uint8_t io_led_on_out (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data>3)
	  {
    printf("bad command!\r\n");   
    return 0;
  }  
	
	switch(in_data)
	{
		case 1: GPIO_ResetBits(GPIOC,GPIO_Pin_15);   break;
		case 2: GPIO_ResetBits(GPIOB,GPIO_Pin_11);   break; 
		case 3: GPIO_ResetBits(GPIOA,GPIO_Pin_15);   break;		
	}	
	 return 0;
}

//LED关闭   "PA15 LED3" "PA4 lED1"  "PB11 lED2"
//2019.2.28 "PA4 lED1" 改为 "PC15 lED1" 
uint8_t io_led_off_out (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	switch(in_data)
	{
		case 1: GPIO_SetBits(GPIOC,GPIO_Pin_15);   break;		
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_11);   break; 
		case 3: GPIO_SetBits(GPIOA,GPIO_Pin_15);   break;		
	}	
	 return 0;
}

//5V控制 PA8
uint8_t io_low_hig_out_5v (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	}
	else
	{
		 GPIO_SetBits(GPIOA,GPIO_Pin_8);
	}	
	 return 0;
}

//led模式  PB15
uint8_t io_low_hig_out_led (int argc, uint8_t *argv[])
{
	uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_15);
	}	
	 return 0;
}

//检测手表在线 PB13
uint8_t watch_on_line_check (int argc, uint8_t *argv[])
{ 	 
	if(0==GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13))
	{
		 printf("board is on line\r\n");
	}
	else
	{
		 printf("board is off line\r\n");
	}	
	 return 0;
}


//电池电源与外部电池连通命令 4V_SW PB7
uint8_t io_low_hig_vcc_bat_link (int argc, uint8_t *argv[])
{	
   uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  
  //
  in_data=(uint8_t)str_to_u(argv[1]);
	
	if(in_data==0)
	{
		 GPIO_ResetBits(GPIOB,GPIO_Pin_7);
	}
	else
	{
		 GPIO_SetBits(GPIOB,GPIO_Pin_7);
	}	
	 return 0;
}

//extern void io_init(void);
uint8_t io_clean (int argc, uint8_t *argv[])
{
//   io_init();
	 gpio_config();
	 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 
	 return 0;
}

//写入批次测试的版本号
uint8_t th902_wv(int argc, uint8_t *argv[])
{
  char length=0;
  uint8_t i,j=0;
  char version[30];
  char save_data0[2048];
	
  if(strlen((char *)argv[1])==21||strlen((char *)argv[1])==22||strlen((char *)argv[1])==23)
  {
	  __disable_irq(); //关闭总中断
	  
	  length=strlen((char *)argv[1]);
//	  printf("argv_length=%d\r\n",length);
	  
	  version[0]=0x55;
	  version[1]=0x5A;
	  version[2]=(u16)length;
	  for(i=3;i<length+3;i++)
	  {
		  version[i]=argv[1][i-3];
	  }
	  printf("写入软件版本号：");
	  for(i=3;i<length+3;i++)
	  {
		  printf("%c",version[i]);
	  }printf("\r\n");
	  
	  flash_write_halfword(FLASH_SAVE_ADDR,(u16 *)version,2+(int)((length)/2));
	  flash_read_bites(FLASH_SAVE_ADDR,save_data0,1024);
	  
	  __enable_irq();
	  return 1;
  }
  else
  {
	printf("bad command!\r\n"); 
	return 0;
  }
	  
}
//读取批次测试的版本号
uint8_t th902_rv(int argc, uint8_t *argv[])
{ 
  int i=0;
  char soft_save[30];	
  u16   soft_length=0;
  if(argc>1)
  {
    printf("bad command!\r\n");   
    return 0;
  }
   __disable_irq(); //关闭总中断
  printf("存储软件版本号：");
  flash_read_bites(FLASH_SAVE_ADDR,soft_save,30);
  if(soft_save[0]==0x55&&soft_save[1]==0x5A)
  {
	soft_length=soft_save[2];
	  
	for(i=0;i<soft_length;i++)
	{
	  printf("%c",soft_save[i+3]);	  
	} printf("\r\n");
  }
  else
  {
	printf("no soft_vesion write,please write soft version!\r\n");   
	__enable_irq(); //开启总中断  
    return 0;  
  } 
   __enable_irq(); //开启总中断
  return 1;
}


//写入批次RTC校准值
uint8_t w_rtc(int argc, uint8_t *argv[])
{
  char length=0;
  uint8_t i,j=0;
  char rtc_save[30];
  char save_data0[30];
  u16   rtc_length=0;
	
	
  if(argc>2)
  {
    printf("bad command!\r\n");   
    return 0;
  }
  if(strlen((char *)argv[1])==11)
  {
	  __disable_irq(); //关闭总中断
	  
	  length=strlen((char *)argv[1]);
	  
	  rtc_save[0]=0x55;
	  rtc_save[1]=0x5A;
	  rtc_save[2]=(u16)length;
	  for(i=3;i<length+3;i++)
	  {
		  rtc_save[i]=argv[1][i-3];
	  }
	 
	  flash_write_halfword(FLASH_SAVE_ADDR1,(u16 *)rtc_save,2+(int)((length)/2));
	  
	  printf("写入批次频率：");	  
	  flash_read_bites(FLASH_SAVE_ADDR1,save_data0,30);
	  if(save_data0[0]==0x55&&save_data0[1]==0x5A)
	  { 
		rtc_length=save_data0[2];
		  
		for(i=0;i<rtc_length;i++)
		{
		  printf("%c",save_data0[i+3]);	  
		} printf("\r\n");
	  }
	  else
	  {
		printf("please write rtc again!\r\n");   
		__enable_irq(); //开启总中断  
		return 0;  
	  } 
	  
	  
	  __enable_irq();
	  return 1;
  }
  else
  {
	printf("data_length must be 11!\r\n"); 
	return 0;
  }
  
  
}

//读取批次RTC校准值
uint8_t r_rtc(int argc, uint8_t *argv[])
{
  int i=0;
  char rtc_save[30];	
  u16   rtc_length=0;
  if(argc>1)
  {
    printf("bad command!\r\n");   
    return 0;
  }
  
 
  printf("存储校准频率：");
  __set_PRIMASK(1); //关闭总中断
  flash_read_bites(FLASH_SAVE_ADDR1,rtc_save,30);
  if(rtc_save[0]==0x55&&rtc_save[1]==0x5A)
  { 
	rtc_length=rtc_save[2];
	  
	for(i=0;i<rtc_length;i++)
	{
	  printf("%c",rtc_save[i+3]);	  
	} printf("\r\n");
  }
  else
  {
	printf("no rtc write,please write rtc!\r\n");   
	__enable_irq(); //开启总中断  
    return 0;  
  } 
  
  __enable_irq(); //开启总中断  
  return 1;
}


//写入批次RTC校准值
uint8_t w_rtc_base(int argc, uint8_t *argv[])
{
char length=0;
  uint8_t i,j=0;
  char rtc_base_save[30];
  char save_data0[30];
  u16   rtc_length=0;	
	
  if(argc>2)
  {
    printf("bad command!\r\n");   
    return 0;
  }
  if(strlen((char *)argv[1])==11)
  {
	  __disable_irq(); //关闭总中断
	  
	  length=strlen((char *)argv[1]);
	  
	  rtc_base_save[0]=0x55;
	  rtc_base_save[1]=0x5A;
	  rtc_base_save[2]=(u16)length;
	  for(i=3;i<length+3;i++)
	  {
		  rtc_base_save[i]=argv[1][i-3];
	  }
	 
	  flash_write_halfword(FLASH_SAVE_ADDR2,(u16 *)rtc_base_save,2+(int)((length)/2));
	  
	  printf("写入基础频率：");	  
	  flash_read_bites(FLASH_SAVE_ADDR2,save_data0,30);
	  if(save_data0[0]==0x55&&save_data0[1]==0x5A)
	  { 
		rtc_length=save_data0[2];
		  
		for(i=0;i<rtc_length;i++)
		{
		  printf("%c",save_data0[i+3]);	  
		} printf("\r\n");
	  }
	  else
	  {
		printf("please write rtc_base again!\r\n");   
		__enable_irq(); //开启总中断  
		return 0;  
	  } 
	  
	  
	  __enable_irq();
	  return 1;
  }
  else
  {
	printf("data_length must be 11!\r\n"); 
	return 0;
  }
  
}

//读取批次RTC校准值
uint8_t r_rtc_base(int argc, uint8_t *argv[])
{
   int i=0;
  char rtc_save[30];	
  u16   rtc_length=0;
  if(argc>1)
  {
    printf("bad command!\r\n");   
    return 0;
  }
  
 
  printf("存储校准频率：");
  __set_PRIMASK(1); //关闭总中断
  flash_read_bites(FLASH_SAVE_ADDR2,rtc_save,30);
  if(rtc_save[0]==0x55&&rtc_save[1]==0x5A)
  { 
	rtc_length=rtc_save[2];
	  
	for(i=0;i<rtc_length;i++)
	{
	  printf("%c",rtc_save[i+3]);	  
	} printf("\r\n");
  }
  else
  {
	printf("no rtc write,please write rtc!\r\n");   
	__enable_irq(); //开启总中断  
    return 0;  
  } 
  
  __enable_irq(); //开启总中断  
  return 1;
}

//频率写入通道切换
uint8_t f_change(int argc, uint8_t *argv[])
{	
   char f_ch_save[1];	
   uint8_t in_data=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }
  
  //
  in_data=(uint8_t)str_to_u(argv[1]);
    if(in_data==0)
    {
		__set_PRIMASK(1); //关闭总中断
		flash_write_halfword(FLASH_SAVE_ADDR3,(u16*)argv[1],1);
		flash_read_bites(FLASH_SAVE_ADDR3,f_ch_save,1);
		
		printf("f_chanle=%s\r\n",(char *)f_ch_save);//显示读到的字符串
		__set_PRIMASK(0); //打开总中断 
    }
	else if(in_data==1)
	{
		__set_PRIMASK(1); //关闭总中断
		flash_write_halfword(FLASH_SAVE_ADDR3,(u16*)argv[1],1);
		flash_read_bites(FLASH_SAVE_ADDR3,f_ch_save,1);
	
		printf("f_chanle=%s\r\n",(char *)f_ch_save);//显示读到的字符串
		__set_PRIMASK(0); //打开总中断 
	}
	else
	{
		printf("bad command!\r\n");   
		return 0;
	}
	 return 0;
}


//#pragma location="Cmd_section"
cmd_tbl_t cmd_tbl[] =
{
    {"version",do_version," - print monitor version"}, 
    {"test",do_test," - print"},
	{"help",do_help," - show help"},
    {"r",do_cpu_reset, "- reset the cpu"},
	{"wa_rst",io_low_hig_out_rst, "	- 复位命令 参数 n  n=0 L  n>0 H"},
	{"wa_gnd2batn",io_low_hig_out_bat1, "-电池负与GND连通命令 参数  n  n=0 L  n>0 H"},
	{"wa_wgnd2gnd",io_low_hig_out_bat2, "-电池地与GND连通命令 参数 n  n=0 L  n>0 H"},
	{"b_led_on",io_led_on_out, "- LED打开 参数 n=1,2,3"},
	{"b_led_off",io_led_off_out, "- LED关闭 参数 n=1,2,3"},
	{"wa_v5",io_low_hig_out_5v, "	- 5V控制 参数 n  n=0 L  n>0 H"},
	{"b_ledmod",io_low_hig_out_led, "- 控制板LED模块控制命令 参数 n  n=0 L  n>0 H"},
	{"wa_pwr_5v",check_5v, "- 手表5V电压检测命令"},
	{"wa_pwr_vdd",check_vdd, "- 手表VDD电压检测命令"},
	{"wa_pwr_vbat",check_vbat, "- 手表VBAT电压检测命令"},
	{"wa_pwr_v33",check_v33, "- 手表V3.3电压检测命令 "},
	{"b_pwr_vbat",check_b_vbat, "- 控制板模拟BAT电压检测命令 "},
	{"w_bat2ext",io_low_hig_vcc_bat_link, "手表电池电压与外部电池连通命令（默认LDO供电)N=0 L N>0 H"},
	{"b_io_clear",io_clean, "- 控制板IO恢复默认配置，不同于STM32复位"},
	{"wa_board_gnd",io_board_to_gnd, "- BGND与GND进行连通"},
	{"wv",th902_wv, "写入flash中TH902版本号(格式：wv TH902_V1.1.1_20190626)"},
	{"rv",th902_rv, "读取flash中TH902版本号"},
	{"w_rtc_base",w_rtc_base, "写入默认频率值"},
	{"r_rtc_base",r_rtc_base, "读取默认频率值"},
	{"w_rtc",w_rtc, "写入频率值(格式：w_rtc 32.79556107)"},
	{"r_rtc",r_rtc, "读取频率值"},
	{"f_ch",f_change, "频率选择通道"},
};

void cpu_soft_reset(void)
{	
	__disable_fault_irq();   
  NVIC_SystemReset();
}

uint8_t do_cpu_reset(int argc, uint8_t *argv[])
{
  if(argc>1)
  {
    printf("bad command!\r\n");
    return 0;
  }
  cpu_soft_reset();
  return 1;
}

//保存的数据，头信息
uint8_t printf_addr_pos_data (int argc, uint8_t *argv[])
{
  uint32_t addr_in=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  addr_in=(uint32_t)str_to_u(argv[1]);
    

    return 0;
}

uint8_t io_reset_out (int argc, uint8_t *argv[])
{
  uint8_t addr_in=0;
  if(argc!=2)
  {
    printf("bad command!\r\n");   
    return 0;
  }  
  
  addr_in=(uint8_t)str_to_u(argv[1]);
    
	printf("indata=%d\r\n",addr_in);

   return 0;
}




//保存的数据，头信息
uint8_t printf_head_data (int argc, uint8_t *argv[])
{
    
    return 0;
}


/*
  发送定位数据
*/
uint8_t send_save_pos_data (int argc, uint8_t *argv[])
{

    return 0;
}
/*
* 打印时差测试参数
*/
uint8_t sc_ft (int argc, uint8_t *argv[])
{    

    
    return 0;
}





/*
argc是输入的参数个数
arv的参数列表，注意arg[0]是函数名，真正的参数从argv[1]开始
*/
uint8_t do_version (int argc, uint8_t *argv[])
{
	//extern char version_string[]; //byTX
	printf ("%s\r\n", version_string);
	return 0;
}


uint8_t do_test(int argc, uint8_t *argv[])// @ "Cmd_section"
{
  uint8_t i;
  printf("this is test command!%d args input\r\n",argc);
  for(i=0;i<argc;i++)
    printf ("\targ[%d]=%s\r\n", i,argv[i]);
  return 1;
}

uint8_t do_help(int argc, uint8_t *argv[])
{
  uint8_t i;
  cmd_tbl_t *cmdtp;
  if(argc>2)
  {
    printf("bad args!\r\n");
    return NULL;
  }
  if(argc==1)
  {
    for(i=0;i<NUM_OF_CMD;i++)
    {
      printf("%s\t\t%s\r\n",cmd_tbl[i].name,cmd_tbl[i].usage);
    }
  }
  else
  {
    cmdtp=find_cmd(argv[1]);
    if(cmdtp==NULL)
    {
      printf("find no command!\r\n");
      return NULL;
    }
    printf("%s\t%s\r\n",cmdtp->name, cmdtp->usage);
  }
  return 1;
}

/************************************************/
/************************************************/
//查找指定命令，返回命令在数组的索引
//返回NULL则没有找到
cmd_tbl_t *find_cmd(uint8_t *cmd)
{
  int i;
  for(i=0;i<NUM_OF_CMD;i++)
  {
    if(strcmp(cmd_tbl[i].name,(const char*)cmd)==0)
    {
      return &cmd_tbl[i];
    }
  }
  return NULL;
}
//拆解参数
//返回NULL则失败，>0则成功
//返回值为当前输入的参数个数
uint8_t parse_line(uint8_t *line, uint8_t *argv[])
{
  int i=0,j=0;
  while(line[i]!=0)
  {
    if(i>CFG_MAX_BUF)
    {
      printf("command too long!\r\n");
      return NULL;
    }
    if(line[i]==ASCII_SPACE)//以空格区别不同参数
      line[i]=0;
    else 
    {
      if((i==0) | (i>0 && line[i-1]==0))//是第一个参数或前面是分隔符
      {
        argv[j]=line+i;
        j++;
        if(j>CFG_MAX_ARG)//参数过多
        {
          printf("there is too many args!\r\n");
          return NULL;
        }
      }
    } 
    i++;
  }
  if(j>0)
    return j;
  else
    return NULL;
}
//执行命令
uint8_t run_commnad(uint8_t *cmd)
{
  uint8_t num_arg=0;//输入的参数个数
  cmd_tbl_t *cmdtp;
  
  uint8_t *argv[CFG_MAX_ARG+1];//null terminated
  num_arg=parse_line(cmd, argv);
  if(NULL==num_arg)
  {//no command at all
    return NULL;
  }
  cmdtp=find_cmd(argv[0]);
  if(cmdtp==NULL)
  {
    printf("bad command!\r\n");
    return NULL;
  }
  //then run the command
  cmdtp->cmd(num_arg,argv);
  return 1;
}
//发送部分VT指令
void send_uart_cursor(uint8_t cmd)
{
  printf("%c%c%c",27,91,cmd);
}
void clean_command_line(void)
{
  uint8_t i;
  uint8_t temp_length=cmd_length;
  for(i=0;i<temp_length;i++)
    clean_cursor_word();
}
void clean_cursor_word(void)
{
  printf("%c%c%c",ASCII_BACKSPACE,ASCII_SPACE,ASCII_BACKSPACE);//退格，空格，再退格
  cmd_index--;
  cmd_length--;
}
//中断处理程序
void isr_uart(uint8_t inBuf)
{
    static uint8_t index_temp=0;   
    //先判断是否有上下左右输入
    //左右移动暂时不考虑
    if(inBuf==27)//ESC
    {
      index_temp=1;
      return;
    }
      
    if((index_temp==1) && (inBuf==91)) //[
    {
      index_temp=2;
      return;
    }
      
    if(index_temp==2)
    {
      index_temp=0;
      switch(inBuf)
      {
        case 'A'://UP
          //显示上一条指令
          //清除当前显示
          if(cmd_index_tbl_length>0)
            clean_command_line();
          //copy the cmd to buf
          if(cmd_index>0)
          {
            copy_new_command_line();
            cmd_index_tbl_length++;
            cmd_index_tbl--;
          }
          get_command_line(PREVIOUS_COMMAND);//FALSE表示上一个command，TRUE表示next command
          
          return;
        case 'B'://DOWN
          //显示下一条指令
          if(cmd_index_tbl<cmd_index_tbl_length)
          {
            clean_command_line();
            get_command_line(NEXT_COMMAND);//FALSE表示上一个command，TRUE表示next command
          }
          return;
        case 'C'://RIGHT
 /*         if(cmd_index<cmd_length)
          {
            send_uart_cursor('C');
            cmd_index++;
          }
          //光标右移
 */
          return;
        case 'D'://LEFT
 /*         if(cmd_index>0)
          {
            send_uart_cursor('D');          
            cmd_index--;
          }
*/          //光标左移
          return;
        default:
          break;
      }
    }
    else
    {
      index_temp=0;
    }
//注意secureCRT用是VT function
 /* 
 VT_CURSOR_DOWNVT_CURSOR_DOWN Sends ESC [ B  27 91 66
 VT_CURSOR_LEFT Sends ESC [ D  27 91 68  
 VT_CURSOR_RIGHT Sends ESC [ C  27 91 67
 VT_CURSOR_UP Sends ESC [ A  27 91 65
 */

    //send what received
   if((inBuf>31) && (inBuf<127) && (cmd_index<CFG_MAX_BUF-1))  
    {
      printf("%c",inBuf);
      cmd_buf[cmd_index]=inBuf;
      cmd_index++;
      cmd_length++;
    }
                          
    if((inBuf==8) && (cmd_index>0))//退格
    {
      clean_cursor_word();
    }
                          
    if(inBuf==0x0d)//回车
    {
      printf("\r\n");     
      cmd_buf[cmd_length]=0;//命令一定以0结束
      //拷贝命令到缓存
      if(cmd_length>0)
      {
        copy_new_command_line();
        //开始处理命令！！
        run_commnad(cmd_buf);
      }
      //cmd_index_tbl=cmd_index_tbl%CFG_MAX_HISTORY;
      cmd_index_tbl_length=cmd_index_tbl;
      
        //printf("the command is:%s\tlength=%d\r\n",cmd_buf,cmd_length);
      cmd_index=0;
      cmd_length=0;
      print_log();
    }
}

//保存新命令到缓存
void copy_new_command_line(void)
{
  //注意如果buf和以前保存的内容相同，则不保存？？
  memcpy(cmd_buf_tbl[cmd_index_tbl%CFG_MAX_HISTORY],cmd_buf,CFG_MAX_BUF);
  cmd_index_tbl++;
  //cmd_index_tbl_length=cmd_index_tbl;  
}
//获取缓冲区的命令，上一个或下一个
void get_command_line(uint8_t bool_next)
{//(cmd_index_tbl-1)%CFG_MAX_HISTORY 指向最后一个cmd buffer
  if((cmd_index_tbl>0) && (bool_next==PREVIOUS_COMMAND))
  {
    cmd_index_tbl--;
    memcpy(cmd_buf,cmd_buf_tbl[cmd_index_tbl%CFG_MAX_HISTORY],CFG_MAX_BUF);
    //更新index及length
    cmd_index=strlen((char *)cmd_buf);
    cmd_length=cmd_index;
    //then display the new command
    printf("%s",cmd_buf);
  }
  if((cmd_index_tbl<cmd_index_tbl_length) && (bool_next==NEXT_COMMAND))
  {
    memcpy(cmd_buf,cmd_buf_tbl[cmd_index_tbl%CFG_MAX_HISTORY],CFG_MAX_BUF);
    cmd_index_tbl++;
    //then display the new command
    printf("%s",cmd_buf);
    //更新index及length
    cmd_index=strlen((char *)cmd_buf);
    cmd_length=cmd_index;
  }
}
/*
//因为定义的buf不是char型，所以求字符串长度不能用sizeof
int size_of_str(uint8_t *buf)
{
  int len=0;
  while(buf[len]==0)
  {
    len++;
    if(len>=CFG_MAX_BUF)
      break;
  }
  return len;
}*/
/*print the log*/
void print_log(void)
{
  printf("WT#");
}

