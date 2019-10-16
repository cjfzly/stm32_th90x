/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_TIME_H
#define __DELAY_TIME_H

#ifdef __cplusplus
 extern "C" {
#endif

 /* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdint.h>
   
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
   
#ifdef __cplusplus
}
#endif

#endif 
