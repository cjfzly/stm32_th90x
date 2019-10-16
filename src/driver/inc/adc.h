#include "stm32f10x.h"
#include "stdio.h"   //byTX
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#ifndef _ADC_H
#define _ADC_H

void  Adc_Init(void);
u16 Get_Adc(u8 ch);
u16 Get_Adc2(u8 ch);
u16 Get_Adc_Average(u8 ch,u8 times);
float Get_Adc_Average2(u8 ch,u8 times);
float Get_Adc_Average3(u8 ch,u8 times);
u16 adc_value(u16 adc_value);
float  adc_i_value(u16 adc_value);
float  adc_i_value1(float adc_value);
int get_adc_avg(u8 ch,int times);
extern u16 adc_value(u16 adc_value);
#endif

