/*******************************************
**
**对控制板需要的管脚进行定义
**
*******************************************/
#ifndef _CONFIG_PIN_H
#define _CONFIG_PIN_H

#include "sys.h"

//led指示灯管脚的配置
#define LED1   PCout(15) //PC15    4 
#define LED2   PBout(11) //PB11    22

#define LED3   PAout(15) //PA15    38
#define LED4   PAout(11) //PA11

//继电器管脚配置
#ifdef JDQ
#define JDQ1   PBout(3)     //BGND与GND进行连通                    PB3
#define JDQ2   PBout(4)     //电池地与GND连通命                    PB4
#define JDQ3   PBout(5)     //外部电池负载模拟                     PB5 
#define JDQ4   PBout(6)     //电池负与GND连通命令                  PB6  41
#define JDQ5   PBout(7)     //电池电源与外部电池连通命令 4V_SW      PB7
#define JDQ6   PBout(12)    //手表5V电压检测                       PB12
#define JDQ7   PBout(13)    //控制板模拟BAT电压检测                 PB13  
#define JDQ8   PBout(14)    //手表V3.3电压检测                     PB14
#define JDQ9   PBout(15)    //手表VDD电压检测                      PB15
#define JDQ10  PAout(8)     //5V控制                               PA8      
#endif


//ADC采样管脚配置,用于电流采样
#ifdef ADC
#define ADC1  1
#define ADC2  2
#endif

//常用函数
/**********转为字符串********/
#define _STR(s)     #s
#define STR(str)    _STR(str)	

void gpio_config(void);
void uart2_to_gpio(void);
#endif

