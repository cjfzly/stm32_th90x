
#include "key.h"
#include "delay.h"
#include "usart.h"	
#include "app1.h"
#include "command.h"

u16 adc_tempt;

u8 KEY_Scan(u8 mode)
{	 
	if((key1==0||key2==0||key3==0))
	{
		delay_ms(60);//去抖动 
		if(key1==0)return key1_press;
		else if(key2==0)return key2_press;
		else if(key3==0)return key3_press;
	}	    
 	return 0;// 无按键按下
}
void key_task()
{
	uint8_t key_value;
	key_value=KEY_Scan(0);
	if(key_value)
    {						   
		switch(key_value)
		{			
			case key1_press:
				  printf("key3 down\r\n");
				  beep_f=1;
				break;
			case key2_press:
				  printf("key1 down\r\n");
				  uart2_init(115200);       //初始化stm32串口，波特率为115200
				  th902_r();
				  uart2_init(9600);
				  memset(str_temp,0,200*sizeof(char));
				break;
			case key3_press:
				  
			    break;
			
		}
	}
}


//PA6-S3,PA7-S2,PB10-S1
void key_intit(void)
{ 
		GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	   /* Enable GPIOA clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Configure PA.06/7 pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  /* Connect EXTI6/7 Line to PA.06/7 pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);

  /* Configure EXTI7/6 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line7|EXTI_Line6;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
\
  /* Enable and set EXTI9_5 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
}



