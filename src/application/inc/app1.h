
#include "led.h"
#include "config_pin.h"
#ifndef _APP1_H
#define _APP1_H

extern volatile u8   work_i_flag,sleep_i_flag,base_i_flag;
extern volatile u8   ble_flag;


void test_v(void);
void base_i(void);
void test_work_i(void);
void test_sleep_i(void);
void test_charge_i(void);
void led_off(void);
void th902_sleep(void);
void th902_r(void);
float test_bat(void);
extern int str_to_value(char str);
extern u8 A_to_B(char A[],char B[]);
extern u8 version_comper(char A[],char B[],char i);
extern u8 compar_str(u8 str_s[]);
int rtc_read(void);

void beep_test(void);
void led_test(void);
void light_test(void);
extern int light_value(void);
void test_fail(void);
void test_success(void);
void test_danger();

extern void array_sort(float array[],int sum_numbers);
extern float delect_average(float array[],int delect_numbers,int sum_numbers);

#endif





