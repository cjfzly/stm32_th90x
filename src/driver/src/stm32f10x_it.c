/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h" 
#include "stdio.h"	
#include "config_pin.h"
#include "timer.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
extern volatile uint8_t UART2_Rec_Flag;
extern volatile uint8_t UART2_Rec_Data;
extern volatile int adc_flag;
extern volatile char t3finish_flag;

extern volatile int beep_adc_flag;
extern volatile char t2finish_flag;


extern volatile int main_beep_adc_flag;
extern volatile char main_t2finish_flag;


extern volatile int beep_i_flag;
extern volatile char t4finish_flag;

extern volatile int  systime_count,systime_count1; 
int     led_time=0;
//�жϽ�������
#define ARRAY_ISR_NUM 3000


struct array_isr_rec
{
	unsigned int first_w_num;
	unsigned int first_r_num;
	unsigned int secon_w_num;
	unsigned int secon_r_num;
	unsigned char error_flag; //�����־λ
};
//�����ṹ������
struct array_isr_rec array_isr1[ARRAY_ISR_NUM];
//�����ṹ��ָ��
struct array_isr_rec *array_p;

////ָ���ʼ��
//array_p = &array_isr1[0];

volatile int flag_5v=0,flag1_5v=0,flag_bat=0,flag1_bat=0;




void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{ 
  //please see my diary
  while (1)
  {
	
  }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
void SysTick_Handler(void)
{

}

void EXTI9_5_IRQHandler(void)
{
	//S2
  if(EXTI_GetITStatus(EXTI_Line7) != RESET)
  { 
	  close_all_led();
	  while(1)
	  {
		  uart2_to_gpio();
		  GPIO_SetBits(GPIOB,GPIO_Pin_7);      //�����ⲿģ���ع���
		  GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		  GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		  GPIO_ResetBits(GPIOB,GPIO_Pin_4);	   //GND��watch_gnd
		  GPIO_ResetBits(GPIOA,GPIO_Pin_8);    //�ر�5v
		  LED2=0;	
	  }
	  EXTI_ClearITPendingBit(EXTI_Line7);
/*
	  if(flag_5v==0)
	  {
		  flag1_5v=0;
		  GPIO_SetBits(GPIOA,GPIO_Pin_8);      //��5v
		  GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		  GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		  GPIO_ResetBits(GPIOB,GPIO_Pin_4);	 //GND��watch_gnd
		  GPIO_ResetBits(GPIOB,GPIO_Pin_7);    //�����ⲿģ���ع���
		  LED2=0;
	  }
	  else if(flag_5v==1)
	  {
		  flag1_5v=1;
		  GPIO_ResetBits(GPIOA,GPIO_Pin_8);    //�ر�5v
		  GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		  GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		  GPIO_SetBits(GPIOB,GPIO_Pin_4);	     //GND��watch_gnd
		  GPIO_ResetBits(GPIOB,GPIO_Pin_7);    //�����ⲿģ���ع���
		  LED2=1;	
	  }
	  if(flag1_5v==0)
	  {
		  flag_5v=1;
	  }
	  else
	  {
		  flag_5v=0;
	  }
*/
  }
	
	//S3
 if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {	
	 close_all_led();
	 while(1)
	 {
	     GPIO_SetBits(GPIOA,GPIO_Pin_8);      //��5v
		 GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		 GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		 GPIO_ResetBits(GPIOB,GPIO_Pin_4);	 //GND��watch_gnd
		 GPIO_ResetBits(GPIOB,GPIO_Pin_7);    //�����ⲿģ���ع���
		 LED3=0; 
	 }
/*	  
	if(flag_bat==0)
	{
		flag1_bat=0;
		uart2_to_gpio();
		GPIO_SetBits(GPIOB,GPIO_Pin_7);     //�����ⲿģ���ع���
		GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		GPIO_ResetBits(GPIOB,GPIO_Pin_4);	 //GND��watch_gnd
		GPIO_ResetBits(GPIOA,GPIO_Pin_8);   //�ر�5v
		LED3=0;	
	}
	else if(flag_bat==1)
	{
		flag1_bat=1;
		uart2_to_gpio();
		GPIO_ResetBits(GPIOA,GPIO_Pin_8);    //�ر�5v
		GPIO_SetBits(GPIOB,GPIO_Pin_3);      //ģ����BGND��ͨBAT-
		GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-��watch_gnd
		GPIO_SetBits(GPIOB,GPIO_Pin_4);	    //GND��watch_gnd
		GPIO_ResetBits(GPIOB,GPIO_Pin_7);    //�ر��ⲿģ���ع���
		LED3=1;
	}
	if(flag1_bat==0)
	{
		flag_bat=1;
	}
	else if(flag1_bat==1)
	{
		flag_bat=0;
	}
	
*/
	EXTI_ClearITPendingBit(EXTI_Line6);
  }
  
//  if(EXTI_GetITStatus(EXTI_Line10) != RESET)
//  {		  
//	 printf("EXTI_10_OK\r\n");  
//	 EXTI_ClearITPendingBit(EXTI_Line10);
//  }
	EXTI_ClearITPendingBit(EXTI_Line6);
	EXTI_ClearITPendingBit(EXTI_Line7);
}

//��ʱ��1�жϷ������
void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //���TIM2�����жϷ������
		{
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //���TIMx�����жϱ�־ 
			systime_count++;
			systime_count1++;
			led_time++;
			if(led_time==3600)
			{
				LED2=!LED2;
				led_time=0;
			}
			if(systime_count>0xfffffffd||systime_count1>0xfffffffd)
			{
				systime_count=0;
				systime_count1=0;
			}
		}
}


//��ʱ��2�жϷ������
void TIM2_IRQHandler(void)   //TIM2�ж�
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM2�����жϷ������
		{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
			beep_adc_flag++;
			main_beep_adc_flag++;
			t2finish_flag=1;//100us��ʱ����־
			main_t2finish_flag=1;
			if(beep_adc_flag>0xfffffffd)
			{
				beep_adc_flag=0;
			}
			if(main_beep_adc_flag>0xfffffffd)
			{
				main_beep_adc_flag=0;
			}
//			if(main_beep_adc_flag%3600==0)
//			{
//				printf("timer2_int\r\n");
//			}
		}
}

//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
		  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
		  adc_flag++;
		  t3finish_flag=1;//1ms��ʱ����־
		  if(adc_flag>0xfffffffd)
		  {
			 adc_flag=0;
		  }
		}
}

//��ʱ���жϴ洢�ͻ������ݣ��Ժ�����ƺͿ����л�����õ�����ģʽ
void TIM4_IRQHandler(void)   //TIM4�ж�
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���TIM4�����жϷ������
		{
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־
			beep_i_flag++;
			t4finish_flag=1;//1ms��ʱ����־
			if(beep_i_flag>0xfffffffd)
			{
			  beep_i_flag=0;
			}
		}
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
