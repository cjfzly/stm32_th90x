#include "sys.h"
#include "usart.h"	  
#include "string.h"

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
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
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
     //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  	
	
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 

}

void uart2_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;        
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //USART2 TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //USART2 RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
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




void USART2_IRQHandler(void)                	//����1�жϷ������
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		UART2_Rec_Data =USART_ReceiveData(USART2);	//��ȡ���յ�������
//		printf("%c",UART2_Rec_Data);
		if(UART2_Rec_Data=='\n')//һ�н������
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




