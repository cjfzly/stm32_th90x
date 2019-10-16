#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
#include "stdio.h"   
#include "string.h"
#include "adc.h"

void TIM1_Int_Init(u16 arr,u16 psc);
void TIM2_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);
#endif

