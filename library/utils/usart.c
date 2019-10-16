#include "sys.h"
#include "usart.h"	  
#include "string.h"

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
u8 str_temp[USART_REC_LEN];
u16 USART_RX_STA=0;  
u8 usrt_flg=0,usrt_flg2=0;

volatile uint8_t UART2_Rec_Flag;
volatile uint8_t UART2_Rec_Data;
volatile u8   led_off_flag=0,t_sleep_flag=0;

//char *STR_SAVE[16]={"bin name:","RTC adj=","get bt module id"," get bt bandrate","set bt fac name ",\
//					"get bt fac name:","read bt mac"," get bt sn:","get bt bandrate","set bt adv",
//					"Set min conn OK!","Set max conn OK!",
//                    "ADXL362 is online!"," flash id : 0x1920c2","FLASH Test Passed!"};


void uart1_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
     //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  	
	
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 

}

void uart2_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;        
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //USART2 TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //USART2 RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound; 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	USART_InitStructure.USART_Parity = USART_Parity_No ; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}




void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		UART2_Rec_Data =USART_ReceiveData(USART2);	//读取接收到的数据
//		printf("%c",UART2_Rec_Data);
		if(UART2_Rec_Data=='\n')//一行接收完成
		{
			usrt_flg=1;
			if(usrt_flg2==1)
			{
				usrt_flg2=2;
			}
		}
		else
		{
			str_temp[USART_RX_STA]=UART2_Rec_Data ;
			USART_RX_STA++;
			if(USART_RX_STA==USART_REC_LEN)
			{
				USART_RX_STA=0;
				memset(str_temp,0,USART_REC_LEN*sizeof(char));
			}
		}
		
		if(str_temp[0]=='t'&&str_temp[1]=='_'&&str_temp[2]=='l'&&
		   str_temp[3]=='e'&&str_temp[4]=='d'&&str_temp[5]=='_')
		{
			led_off_flag=1;
		}
		else if(str_temp[1]=='t'&&str_temp[2]=='_'&&str_temp[3]=='l'&&
		   str_temp[4]=='e'&&str_temp[5]=='d'&&str_temp[6]=='_')
		{
			led_off_flag=1;
		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
} 




