#include "stm32f10x.h"
#include "config_pin.h"
#include "adc.h"
#include "sys.h"
#include "stdio.h"   //byTX
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

extern volatile u8 beep_f;

#ifndef _KEY_H
#define _KEY_H

#define key1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) //KEY_CLEAR   PB10
#define key2  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)  //KEY_START   PA6
#define key3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)  //KEY_STOP    PA7

#define key1_press 1
#define key2_press 2
#define key3_press 3

extern u8 KEY_Scan(u8 mode);
extern void key_intit(void);
void key_task(void);
#endif



