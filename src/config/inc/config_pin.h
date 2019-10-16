/*******************************************
**
**�Կ��ư���Ҫ�ĹܽŽ��ж���
**
*******************************************/
#ifndef _CONFIG_PIN_H
#define _CONFIG_PIN_H

#include "sys.h"

//ledָʾ�ƹܽŵ�����
#define LED1   PCout(15) //PC15    4 
#define LED2   PBout(11) //PB11    22

#define LED3   PAout(15) //PA15    38
#define LED4   PAout(11) //PA11

//�̵����ܽ�����
#ifdef JDQ
#define JDQ1   PBout(3)     //BGND��GND������ͨ                    PB3
#define JDQ2   PBout(4)     //��ص���GND��ͨ��                    PB4
#define JDQ3   PBout(5)     //�ⲿ��ظ���ģ��                     PB5 
#define JDQ4   PBout(6)     //��ظ���GND��ͨ����                  PB6  41
#define JDQ5   PBout(7)     //��ص�Դ���ⲿ�����ͨ���� 4V_SW      PB7
#define JDQ6   PBout(12)    //�ֱ�5V��ѹ���                       PB12
#define JDQ7   PBout(13)    //���ư�ģ��BAT��ѹ���                 PB13  
#define JDQ8   PBout(14)    //�ֱ�V3.3��ѹ���                     PB14
#define JDQ9   PBout(15)    //�ֱ�VDD��ѹ���                      PB15
#define JDQ10  PAout(8)     //5V����                               PA8      
#endif


//ADC�����ܽ�����,���ڵ�������
#ifdef ADC
#define ADC1  1
#define ADC2  2
#endif

//���ú���
/**********תΪ�ַ���********/
#define _STR(s)     #s
#define STR(str)    _STR(str)	

void gpio_config(void);
void uart2_to_gpio(void);
#endif

