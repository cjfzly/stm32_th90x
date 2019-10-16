#include "stm32f10x.h"
#include "led.h"
#include "config_pin.h"
#include "delay.h"

void LED_Init()
{
	

}
//打开全部的LED
void LED_ALL_OPEN()
{
	
}
//跑马灯函数
void run_led(void)
{
	LED1=!LED1;
	delay_ms(100);
	LED3=!LED3;
	delay_ms(100);
	LED2=!LED2;
	delay_ms(100);
}

//关闭全部LED
void close_all_led(void)
{
    LED1=1;
	LED2=1;
	LED3=1;
}