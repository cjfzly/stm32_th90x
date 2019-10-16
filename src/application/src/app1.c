//自动化测试程序app1设计
/************************
**1、首先进行电压测试
**2、实现对胸贴版本信息进行测试（加速度、MAC、Flash）
**   蓝牙的连接与断开还需要再加入一个串口（可以连接APP来进行下一步的测试操作）
**   时钟项测试还需再加入一个串口
**   测试结果记录项还需再外置flash存储，（后期可以考虑通过计算机导出或者是手机APP进行同步）   
**	 在测试的过程中，输入的电压是有变化的，4.9-5.1V变化
**	 在后期的调试上面，主要是对代码的优化，打算将本次的测试环境应用在生产测试（需购买6套传感器器件（光线、声音、电胶带））
**
**
**
**
*************************/
#include "app1.h"
#include "delay.h"
#include "adc.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "light.h"
#include "stm32f10x_usart.h"
#include "flash.h"

volatile u8   work_i_flag=0,sleep_i_flag=0,base_i_flag=0;
volatile u8 beep_f;
volatile u8   ble_flag=0;
volatile int  systime_count=0; 

volatile int beep_adc_flag=0;
volatile char t2finish_flag=0;

volatile int beep_i_flag=0;
volatile char t4finish_flag=0;
volatile char uart_irflag1=0;
//电压值
u16 TS_V;
float adc_i,bp_i_average=0;

//做电压阈值判断，单位毫伏
#define V5_V           5000
#define VBAT_V         4000
#define V33_V          3300
#define W_VDD          4200//unused
#define W_VDD_MAX      5000
#define W_VDD_MIN      3900
#define W_VDD1         4000

#define CHARGE_I_MAX   60 
#define CHARGE_I_MIN   48

#define V5_OFFSET      500 //不同的电源适配器所供的电源由原来5200mV更改为5500mV
#define V5_OFFSET1     300

#define VBAT_OFFSET    200
#define VBAT_OFFSET1    350

#define V33_OFFSET     100
#define V33_OFFSET1     200

#define WV_OFFSET      260
#define WV_OFFSET1     200

#define STR_TH902_SIZE 18
/**********************************************************************************************************















***********************************************************************************************************/
char *STR_TH902[]={"help","r","t_clk","t_cal","t_bat","t_led","t_sleep","t_checklead","t_openuart",
					"t_beep","t_rtcadj","t_led_off","t_led_on","t_8232_on","t_8232_off","t_362_on",
					"t_362_off","t_bt_avd_on"};

#define STR_SIZE 20
char *STR_SAVE[STR_SIZE]={"bin name:","RTC adj","get bt module id","get bt bandrate","get bt fac name:",
					       "set bt fac name","read bt mac","set bt sn","get bt sn:",
					       "Set min conn OK!","Set max conn OK!","set bt adv",
                           "ADXL362 is offline!","flash id :","FLASH Test" ,"th902 init end","t_led_off",
						   "FLASH Test Failed!","t_led_off\rLed turn off:\r TH902#"};
#define LED_SIZE 5
char *LED_GET[LED_SIZE]={"LED_ALL_CLOSE","LED_GREEN_OPEN","LED_RED_OPEN","LED_ALL_OPEN"};
//蜂鸣器电流存储数组
#define BEEP_I_S       250  
#define BEEP_I_NUMBER  100
#define BEEP_I_DELECT  0
						   
#define BEEP_S         1  //100us
#define BEEP_NUMBER    200  //
#define BEEP_MIN       200
						   
#define i_beep_pass1  74.5
#define i_beep_pass   9.0
float    beep_i_open[BEEP_I_NUMBER];
						   
float    beep_v_detect[BEEP_NUMBER];		   
float    beep_i_temp=0;
float    beep_i_average=0;
float    beep_i_sum=0;					   
						   
				
//充电电流存储数组
#define CHARGE_I_NUMBER  5
#define CHARGE_DELECT    0 //去除排序后最大和最小的个数（例：4 代表去除排序最大的4个和排序后最小的4个）
float    charge_i_num[CHARGE_I_NUMBER];					


//LED灯电流存储数组
#define LIGHT_I_NUMBER 80
#define LIGHT_I_DELECT 5
float   light_i_num[LIGHT_I_NUMBER];


					
////light变量声明
#define  LIGHT_MIN 9
#define  LIGHT_NUMBER 12
#define  LIGHT_DELECT 1
uchar    BUF[8];                         //接收数据缓存区
float    Light_Buf[LIGHT_NUMBER];		 
int     dis_data;                       //变量		
int   mcy;


//采集电池电压变量
u16 bat_v=0;
//测试结束提醒变量
uint32_t delay_count=0;
uint32_t delay_led=0;	

//经常用到的判断值
#define TRUE   1
#define FALSE  0


//用于对比字符串
u8 A_to_B(char A[],char B[])
{
	char j=0;
	for(j=0;j<strlen(B)+1;j++)
	{
		if(A[j]==B[j])	
		continue;
		if(j==strlen(B))
		{
			return TRUE;
		}
		if(A[j]!=B[j])
		{
			return FALSE;
		}
	}
	return FALSE;
}
//用于版本的比较
u8 version_comper(char A[],char B[],char i)
{
	char j=0;
	for(j=0;j<i+1;j++)
	{
		if(A[j]==B[j])	
		continue;
		if(j==i)
		{
			return TRUE;
		}
		if(A[j]!=B[j])
		{
			return FALSE;
		}
	}
	return FALSE;
}

//将数字字符转化为数字
int str_to_value(char str)
{
	return str-'0';
}

//用于查找字符串
u8 compar_str(u8 str_s[])
{
	int i=0,j=0,k=0;
	for(i=0;i<STR_SIZE;i++)
	{
		for(j=0;j<strlen(STR_SAVE[i])+1;j++)
		{
			k=strlen(STR_SAVE[i]);
			if(str_s[j]==STR_SAVE[i][j])	
			continue;
			if(j==k)
			{
				return i;
			}
			if(str_s[j]!=STR_SAVE[i][j])
			{
				break;
			}
		}
		if(i==STR_SIZE)//所有的都查询完成，没有该字符串
		{
			return 51;
		}
		else		  //没有查询完成，继续往下查询
		continue;
	
	}
	return 50;	
}


//用于查找LED状态
u8 compar_str_led(u8 str_s[])
{
	int i=0,j=0,k=0;
	for(i=0;i<LED_SIZE;i++)
	{
		for(j=0;j<strlen(LED_GET[i])+1;j++)
		{
			k=strlen(LED_GET[i]);
			if(str_s[j]==LED_GET[i][j])	
			continue;
			if(j==k)
			{
				return i;
			}
			if(str_s[j]!=LED_GET[i][j])
			{
				break;
			}
		}
		if(i==LED_SIZE)//所有的都查询完成，没有该字符串
		{
			return 51;
		}
		else		  //没有查询完成，继续往下查询
		continue;
	
	}
	return 50;	
}
					
//用于检测失败时的等待函数
void test_fail(void)
{
	memset(str_temp,0,200*sizeof(char));
	USART_RX_STA=0;
	
	gpio_config();
	uart2_init(9600);         //初始化测试版串口，波特率为9600
	delay_ms(1000);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	bat_v=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
	delay_ms(100);
	
	//定时器1关闭系统运行状态LED2
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
	TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断
	TIM_Cmd(TIM1, DISABLE);  //使能TIMx
	LED2=1;
	printf("Test_End-----------------------%dms\r\n",systime_count/5);
	printf("-----------------FAIL------------------\r\n\r\n");
	while(1)
	{
		delay_count++;
		delay_led++;
		bat_v=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
		if(bat_v<150)
		{
			delay_ms(100);
			__disable_fault_irq();   
			NVIC_SystemReset();
		}
//		if(delay_count>30)
//		 {	
//			 delay_count=0;
//			 GPIO_SetBits(GPIOB,GPIO_Pin_14);
//			 delay_ms(300);
//			 GPIO_ResetBits(GPIOB,GPIO_Pin_14);
//		 }
		 if(delay_led>5)
		 {
			delay_led=0;
			delay_ms(200);
			LED1=!LED1;
		 }
	}
}

//用于检测成功时的等待函数
void test_success(void)
{
	memset(str_temp,0,200*sizeof(char));
	USART_RX_STA=0;
	
	gpio_config();
	uart2_init(9600);         //初始化测试版串口，波特率为9600
	delay_ms(1000);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	bat_v=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
	delay_ms(100);
	
	//定时器1关闭系统运行状态LED2
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
	TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断
	TIM_Cmd(TIM1, DISABLE);  //使能TIMx
	LED2=1;
	printf("Test_End-----------------------%dms\r\n",systime_count/5);
	printf("-----------------PASS-----------------\r\n\r\n");
	while(1)
	{
		delay_count++;
		delay_led++;
		bat_v=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
		if(bat_v<150)
		{
			delay_ms(100);
			__disable_fault_irq();   
			NVIC_SystemReset();
		}
//		if(delay_count>30)
//		 {	
//			 delay_count=0;
////			 GPIO_SetBits(GPIOB,GPIO_Pin_14);
//			 delay_ms(300);
////			 GPIO_ResetBits(GPIOB,GPIO_Pin_14);
//		 }
		  if(delay_led>5)
		 {
			delay_led=0;
			delay_ms(100);
			LED3=!LED3;
		 }
	}

}					

//用于检测测试环境有风险报警，需等待开发人员排查
void test_danger()
{
	gpio_config();
	uart2_init(9600);         //初始化测试版串口，波特率为9600
	delay_ms(1000);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	bat_v=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
	delay_ms(100);
	
	//定时器1关闭系统运行状态LED2
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
	TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断
	TIM_Cmd(TIM1, DISABLE);  //使能TIMx
	LED2=1;
	printf("控制板自我保护，请联系开发人员！\r\n1.所采集的电流过大\r\n2.控制板基础电流过大\r\n");	
	printf("Test_End-----------------------%dms\r\n",systime_count/5);
	printf("-----------------DANGER---------------\r\n\r\n");
	while(1)
	{
		 delay_led++;
		
		 if(delay_led>5)
		 {
			delay_led=0;
			delay_ms(100);
			LED1=!LED1;
			LED2=!LED2;
		 }
		 
	}


}

//用于数组的排序
void array_sort(float array[],int sum_numbers)
{
	int i, j, isSorted;
	float temp;
    for(i=0; i<sum_numbers-1; i++)
	{
        isSorted = 1; 
        for(j=0; j<sum_numbers-1-i; j++)
		{
            if(array[j] > array[j+1])
			{
                temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
                isSorted = 0; 
            }
        }
        if(isSorted) break; 
    }
	
}

//对排完序后再去最大和最小个数后的数组计算平均值
float delect_average(float array[],int delect_numbers,int sum_numbers)
{
	int i=0;
	float sum=0;
	for(i=delect_numbers; i<sum_numbers-delect_numbers; i++) //不要最大和最小3个数参与判断
	{
		sum+=array[i];
	}
	return sum/(sum_numbers-2*delect_numbers);
}

//用于求数据的占比
int pwm_value(float array[],int min_value,int numbers)
{
	int   i;
	static int j=0,k=0;
	for(i=0;i<numbers;i++)
	{
		if(array[i]<=min_value)
		{
			++j;
		}
		else
		{
			++k;
		}
	}
	return  j;
}
//检测电压


void test_v(void)
{
	float test_bat_v=0.0;
	float bat_v_temp=0.0;
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	delay_ms(2000);
	TS_V=adc_value(Get_Adc_Average2(ADC_Channel_0,20));
	
	//做5V电压判断
	 if((TS_V<=(V5_V+V5_OFFSET))&&(TS_V>=(V5_V-V5_OFFSET1)))
	 {
		 printf("a_v5_ok=%dmv(4700~5500)\r\n",TS_V);
	 }
	 else
	 {
		 printf("5.1 a_v5_error=%dmv(4700~5500)\r\n",TS_V);
		 test_fail();
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);	
	 
	
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	delay_ms(500);
    TS_V=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
	 
	//做vbat电压判断
	 if((TS_V<=(VBAT_V+VBAT_OFFSET1))&&(TS_V>=(VBAT_V-VBAT_OFFSET)))
	 {
		 printf("a_bat_ok=%dmv(3800~4350)\r\n",TS_V);
	 }
	 else
	 {
		 printf("5.2 a_bat_error=%dmv(3800~4350)\r\n",TS_V);
		 test_fail();
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);	
	
	//手表自己检测电压
	 bat_v_temp=test_bat();
	test_bat_v=bat_v_temp*1000;
	if((test_bat_v<=(VBAT_V+VBAT_OFFSET1))&&(test_bat_v>=(VBAT_V-VBAT_OFFSET)))
	 {
		 printf("c_bat_ok=%dmv(3800~4350)\r\n",(int)test_bat_v);
	 }
	 else
	 {
		 printf("5.3 c_bat_error=%dmv(3800~4350)\r\n",(int)test_bat_v);
		 test_fail();
	 } 
	 
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	delay_ms(500);
    TS_V=adc_value(Get_Adc_Average2(ADC_Channel_8,35));
	 
	//做vdd电压判断
	 if(TS_V<=W_VDD_MAX&&TS_V>=W_VDD_MIN)
	 {
		 printf("a_vdd_ok=%dmv(3900~5000)\r\n",TS_V);
	 }
	 else
	 {
		 printf("5.4 a_vdd_error=%dmv(3900~5000)\r\n",TS_V);
		 test_fail();
	 }
	 GPIO_ResetBits(GPIOB,GPIO_Pin_15);	
	 
	 
	 delay_ms(10);
	 GPIO_SetBits(GPIOB,GPIO_Pin_14);
	 delay_ms(1500);
     TS_V=adc_value(Get_Adc_Average2(ADC_Channel_9,20));
	//做3.3v电压判断
	 if((TS_V<=(V33_V+V33_OFFSET))&&(TS_V>=(V33_V-V33_OFFSET1)))
	 {
		 printf("a_3v3_ok=%dmv(3100~3400)\r\n",TS_V);
	 }
	 else
	 {
		 printf("5.5 a_3v3_error=%dmv(3100~3400)\r\n",TS_V);
		 test_fail();
	 }
	 
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);		
	
}


//单独电池供电后检测电压
void test_v1(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	delay_ms(3000);
	TS_V=adc_value(Get_Adc_Average2(ADC_Channel_0,20));
	
	//做5V电压判断
	 if(TS_V<=500)
	 {
		 printf("b_v5_ok=%dmv(0~500)\r\n",TS_V);
	 }
	 else
	 {
		 printf("8.1 b_v5_error=%dmv(0~500)\r\n",TS_V);
		 test_fail();
		 
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);	
	
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	delay_ms(500);
    TS_V=adc_value(Get_Adc_Average2(ADC_Channel_1,20));
	 
	//做vbat电压判断
	 if((TS_V<=(VBAT_V+VBAT_OFFSET1))&&(TS_V>=(VBAT_V-VBAT_OFFSET)))
	 {
		 printf("b_bat_ok=%dmv(3800~4350)\r\n",TS_V);
	 }
	 else
	 {
		 printf("8.2 b_bat_error=%dmv(3800~4350)\r\n",TS_V);
		 test_fail();
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	delay_ms(500);
    TS_V=adc_value(Get_Adc_Average2(ADC_Channel_8,35));
	 
	//做vdd电压判断
	 if((TS_V<=(W_VDD1+WV_OFFSET1))&&(TS_V>=(W_VDD1-WV_OFFSET1)))
	 {
		 printf("b_vdd_ok=%dmv(3800~4200)\r\n",TS_V);
	 }
	 else
	 {
		 printf("8.3 b_vdd_error=%dmv(3800~4200)\r\n",TS_V);
		 test_fail();
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);

	
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	delay_ms(500);
    TS_V=adc_value(Get_Adc_Average2(ADC_Channel_9,20));
	 //做3.3v电压判断
	 if((TS_V<=(V33_V+V33_OFFSET))&&(TS_V>=(V33_V-V33_OFFSET1)))
	 {
		 printf("b_3v3_ok=%dmv(3100~3400)\r\n",TS_V);
	 }
	 else
	 {
		 printf("8.4 b_3v3_error=%dmv(3100~3400)\r\n",TS_V);
		 test_fail();
	 }
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
}


//检测充电电流
void test_charge_i(void)
{
   int i=0;
   float charge_i=0;
	uart2_to_gpio();
	delay_ms(50);
   //用模拟负载来进行模拟充电电流，不能打开模拟外部电池供电
	GPIO_SetBits(GPIOB,GPIO_Pin_3);    //模拟电池BGND连通BAT-
	GPIO_SetBits(GPIOB,GPIO_Pin_7);    //采用外部模拟电池供电
	delay_ms(720); 
	
	//接通到撤除时间维持200ms
	GPIO_SetBits(GPIOB,GPIO_Pin_5);    //开模拟负载
	delay_ms(720);
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);  //关闭外部模拟电池供电
		
   for(i=0;i<CHARGE_I_NUMBER;i++)
	{
	   adc_i=adc_i_value1(Get_Adc_Average2(ADC_Channel_4,100));
	   charge_i_num[i]=adc_i;
//	   printf("charge_i_test=%f\r\n",charge_i_num[i]);
	}
	array_sort(charge_i_num,CHARGE_I_NUMBER);
	charge_i=delect_average(charge_i_num,CHARGE_DELECT,CHARGE_I_NUMBER);
	if(CHARGE_I_MIN<=charge_i&&charge_i<=CHARGE_I_MAX)
	{
		printf("charge_i_ok=%fmA(48~60)\r\n",charge_i);
	}
	else
	{
		printf("6.1 charge_i_error=:%fmA(48~60)\r\n",charge_i);
		test_fail();
	}
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);    //关闭模拟负载 
	delay_ms(200);
}


//TH902灭灯广播
void led_off(void)
{
	char time=10;
	uart2_init(9600);
	delay_ms(300);   //等待串口平稳
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_led_off\r\n");
		delay_ms(250);
		if(led_off_flag==1)
		{
			return;
		}
//		printf("t_led_off_time=%d\r\n",10-time);
	}while(time--);
	printf("t_led_off!\r\n");
}

void th902_sleep(void)
{
	char time=10;
	char sleep_flag=0;
	uart2_init(9600);
	delay_ms(300);   //等待串口平稳
	
	time=2;
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"\r\n");
		delay_ms(300);	
	}while(time--);
	
	
	time=10;	
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_sleep\r\n");
		delay_ms(250);
		if(str_temp[0]==0x74&&str_temp[1]==0x5F&&str_temp[2]==0x73&&
		   str_temp[3]==0x6C&&str_temp[4]==0x65&& str_temp[5]==0x65&&str_temp[6]==0x70)
		{
			return;
		}
		else if(str_temp[1]==0x74&&str_temp[2]==0x5F&&str_temp[3]==0x73&&
		   str_temp[4]==0x6C&&str_temp[5]==0x65&& str_temp[6]==0x65&&str_temp[7]==0x70)
		{
			return;
		}
//		printf("t_sleep_time=%d\r\n",10-time);
	}while(time--);
	printf("t_sleep!\r\n");
}

//命令读取电池电压
float test_bat(void)
{
	char time=10;
	float test_bat_v=0.0;
	uart2_init(9600);
	delay_ms(300);   //等待串口平稳
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_bat\r\n");
		test_bat_v= str_to_value(str_temp[11])+1.0*(str_to_value(str_temp[13])*100000+str_to_value(str_temp[14])*10000+str_to_value(str_temp[15])*1000
		           +str_to_value(str_temp[16])*100+str_to_value(str_temp[17])*10+str_to_value(str_temp[18]))/1000000;
		if(test_bat_v>4)
		{
			return test_bat_v;
		}
	}while(time--);
	printf("t_bat!\r\n");
}


//检测基础电流
void base_i(void)
{
	base_i_flag=1;
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );    //使能指定的TIM3中断,允许更新中断
    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
}

//检测工作电流,在主循环里面进行采集
void test_work_i(void)
{
	//请勿调整顺序，会导致电流检测芯片烧坏
	//采用1k的定时器3进行电流采集
	led_off();
	GPIO_SetBits(GPIOB,GPIO_Pin_3);      //模拟电池BGND连通BAT-
	GPIO_SetBits(GPIOB,GPIO_Pin_6);      //BAT-与watch_gnd
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	 //GND与watch_gnd
	GPIO_SetBits(GPIOB,GPIO_Pin_7);    //采用外部模拟电池供电
	delay_ms(100);
	delay_ms(200);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);  //关闭5v
	
	work_i_flag=1;
	
	delay_ms(10);
	test_v1();
	delay_ms(1000);
	uart2_to_gpio();
	GPIO_SetBits(GPIOB,GPIO_Pin_4);    //断开watch_gnd---GND
	
	//延迟3S后开始采集电流，此目的是为了避免刚开始的大电流
	delay_ms(5000);
	delay_ms(5000);
	printf("--------------9.WORK_I---------%dms\r\n",systime_count/5);;
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
	
	
}

//检测睡眠电流，在主循环里面进行采集
void test_sleep_i(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);    //打开watch_gnd---GND
	delay_ms(200);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);     //开启5v
	delay_ms(2000);                     //这里需要等待足够长的时间，才能使发送睡眠指令成功
	
	
	th902_sleep();
	
	GPIO_SetBits(GPIOB,GPIO_Pin_4);    //断开watch_gnd---GND
	delay_ms(200);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);  //关闭5v
		
	uart2_to_gpio();				   //禁止串口供电
	
	//解决在已收到睡眠时候，电流没有降下去的问题，加入等待延时，保证主机完全进入睡眠
	delay_ms(2000);
	delay_ms(2000);
	delay_ms(2000);

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx更新中断标志 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
	
}
//检测失败后，复位胸贴，进行再次的检测
void th902_r(void)
{
	char time=2;
	do
	{
		Usart_SendString(USART2,"\r\n");
		delay_ms(200);
	}while(time--);
	memset(str_temp,0,200*sizeof(char));
}

void beep_test(void)
{
	static int j=0,k=0,m=0,f=0,js=0;
	int   close_beep=0;
	int    beep_num=0;	
	int    ts_temp=0;
	char  try_time=0;
	memset(beep_v_detect,0,BEEP_NUMBER*sizeof(char));
	
	try_time=2;
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"\r\n");
		delay_ms(300);
	}while(try_time--);
	
	try_time=30;
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_beep 1\r\n");
		delay_ms(300);
		if(str_temp[1]=='t'&&str_temp[2]=='_'&&str_temp[3]=='b'&&str_temp[4]=='e'&&str_temp[5]=='e')
		{
			break;
		}
		else if(str_temp[0]=='t'&&str_temp[1]=='_'&&str_temp[2]=='b'&&str_temp[3]=='e'&&str_temp[4]=='e')
		{
			break;
		}
	}while(try_time--);
	printf("try_times=%d\r\n",30-try_time);
	delay_ms(20);
	
	
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIMx更新中断标志 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM2中断,允许更新中断
	TIM_Cmd(TIM2, ENABLE);  //使能TIM
	
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //清除TIMx更新中断标志 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
	TIM_Cmd(TIM4, ENABLE);  //使能TIMx
	
	while(1)
	{
		
		//再用timer4来进行电流采集
		if(beep_i_flag>0&&t4finish_flag==1)
		{
			m++;
			beep_i_temp=(Get_Adc2(ADC_Channel_4)*165/4096);
			beep_i_sum+=beep_i_temp;
			if(m==BEEP_I_S)
			{
				m=0;
				beep_i_flag=0;
				t4finish_flag=0;
				beep_i_average=beep_i_sum/BEEP_I_S;
				beep_i_open[f]=beep_i_average;
//				printf("beep_i:%fmA\r\n",beep_i_open[f]);
				beep_i_sum=0;
				f++;
				js++;
			 }
		 } 
		
		
		//用timer2来进行声音监控
		if(beep_adc_flag>0&&t2finish_flag==1)
		{
			j++;
			if(j==BEEP_S)
			{
				j=0;
				beep_adc_flag=0;
				t2finish_flag=0;
				beep_v_detect[k]=Get_Adc(ADC_Channel_6);
				k++;
				if(k==BEEP_NUMBER)
				{
					for(;ts_temp<k;ts_temp++)
					{
//						printf("%f\r\n",beep_v_detect[ts_temp]);
					}
					
					k=0;
					t2finish_flag=0;
					TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIMx更新中断标志 
					TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断
					TIM_Cmd(TIM2, DISABLE);  //使能TIMx
					
					f=0;
					t4finish_flag=0;
					TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //清除TIMx更新中断标志 
					TIM_ITConfig(TIM4,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断
					TIM_Cmd(TIM4, DISABLE);  //使能TIMx
					
					beep_num=pwm_value(beep_v_detect,BEEP_MIN,BEEP_NUMBER);
//					printf("beep_pwm=%d(40~200)\r\n",beep_num);
					bp_i_average=delect_average(beep_i_open,0,js);
					printf("beep_i_ok=%fmA(60~100)\r\n",bp_i_average);
					
					
					//关闭蜂鸣器
					try_time=10;
					do
					{
						memset(str_temp,0,200*sizeof(char));
						USART_RX_STA=0;
						Usart_SendString(USART2,"t_beep 0\r\n");
						delay_ms(300);
						if(str_temp[1]=='t'&&str_temp[2]=='_'&&str_temp[3]=='b'&&str_temp[4]=='e'&&str_temp[5]=='e')
						{
							try_time=3;
							break;
						}
						else if(str_temp[0]=='t'&&str_temp[1]=='_'&&str_temp[2]=='b'&&str_temp[3]=='e'&&str_temp[4]=='e')
						{
							try_time=3;
							break;
						}
					}while(try_time--);
					
					
					
					//蜂鸣器通过条件判断
					if(bp_i_average>60&&bp_i_average<100)
					{
						break;
					}
					else
					{
						printf("2.1 beep_i_error=%fmA(60~100)\r\n",bp_i_average);
						test_fail();
					}
				}
			 }
		 } 
	 }
}


//if(ble_flag==1&&beep_i_average>i_beep_pass1)    //新蓝牙模块蜂鸣器判定
//{
//	Usart_SendString(USART2,"t_beep 0\r\n");
//	memset(str_temp,0,200*sizeof(char));
//	USART_RX_STA=0;
//	Usart_SendString(USART2,"t_beep 0\r\n");
//	memset(str_temp,0,200*sizeof(char));
//	USART_RX_STA=0;
//	break;
//}
//else if(ble_flag==0&&beep_i_average>i_beep_pass)//旧蓝牙模块蜂鸣器判定
//{
//	Usart_SendString(USART2,"t_beep 0\r\n");
//	memset(str_temp,0,200*sizeof(char));
//	USART_RX_STA=0;
//	Usart_SendString(USART2,"t_beep 0\r\n");
//	memset(str_temp,0,200*sizeof(char));
//	USART_RX_STA=0;
//	break;
//}


float led_i_average(float A[6])
{
	int i=0;
	float i_led_average=0.0,i_led_sum=0.0;
    
	delay_ms(200);
//	LED3=1;
	for(i=0;i<LIGHT_I_NUMBER;i++)
	{
	   adc_i=adc_i_value1(Get_Adc_Average2(ADC_Channel_4,10));
	   light_i_num[i]=adc_i;
	   i_led_sum+=adc_i;
//	   printf("led_i_test=%f\r\n",charge_i_num[i]);	
	}	
	i_led_average=delect_average(light_i_num,LIGHT_I_DELECT,LIGHT_I_NUMBER);
//	i_led_average=i_led_sum/100;
//	LED3=0;
	return i_led_average;	
}


void light_test(void)
{
	char  light_i=0;
	int   i=0,j=50,time1=0,time2=0;
	float temp=0;
	char  try_time=0;
	int   get_id=10;
	float led1_i=0.0,led2_i=0.0,led3_i=0.0,led4_i=0.0;
	char  led1_flag=0,led2_flag=0,led3_flag=0,led4_flag=0;
	//做四个存储4组LED的电流的数组
	float led_close[6],led_green[6],led_red[6],led_open[6];
	
	try_time=5;
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_led\r\n");
		delay_ms(300);
		if(str_temp[1]=='t'&&str_temp[2]=='_'&&str_temp[3]=='l'&&str_temp[4]=='e'&&str_temp[5]=='d')
		{
			try_time=3;
			break;
		}
		else if(str_temp[0]=='t'&&str_temp[1]=='_'&&str_temp[2]=='l'&&str_temp[3]=='e'&&str_temp[4]=='d')
		{
			try_time=3;
			break;
		}
	}while(try_time--);
	
	//LED灯闪烁时，对5V供电处电流进行采集
	uart2_init(9600);
	delay_ms(500);   //等待串口平稳
	while(1)
	{
		if(uart_irflag1==1)
		{
			uart_irflag1=0;
			memset(str_temp,0,200*sizeof(char));
		}
		
		if(usrt_flg>0)
		{
			j=compar_str_led(str_temp);
			USART_RX_STA=0;				
			if(j==0||j==1||j==2||j==3)
			{
//				printf("led_get=%d\r\n",j);
			}
			//此处可以用来调试
			else
			{
				uart_irflag1=1;
			}
			usrt_flg=0;
		}
	   switch(j)
	   {
		   case 0:
			   j=10;
			   led1_flag=1;
			   led1_i=led_i_average(led_close);
//			   printf("led_close=%fmA\r\n",led1_i);
			   break;
		   case 1:
			   j=10;
			   led2_flag=1;
			   led2_i=led_i_average(led_green);
//			   printf("led_green=%fmA\r\n",led2_i);
			   break;
		   case 2:
			   j=10;
			   led3_flag=1;
			   led3_i=led_i_average(led_red);
//			   printf("led_red=%fmA\r\n",led3_i);
			   break;
		   case 3:
			   j=10;
			   led4_flag=1;
			   led4_i=led_i_average(led_open);
//			   printf("led_open=%fmA\r\n",led4_i);
			   break;	
		   default:
			   j=10;
			   break;
	   }
	   if(led1_flag==1&&led2_flag==1&&led3_flag==1&&led4_flag==1)
	   {
//		    led1_flag=0;
//		    led2_flag=0;
//		    led3_flag=0;
//		    led4_flag=0;
//			printf("red_led= %fmA\r\n",led3_i-led1_i);
//		    printf("green_led= %fmA\r\n",led2_i-led1_i);
//		    printf("\r\n\r\n\r\n\r\n");
		    //2019/10/9测试1500套后发现后做实验，进行以下更改：
		    //led2_i-led_i>0.20更改为led2_i-led1_i>0.15
		    //led3_i-led1_i>0.60更改为led3_i-led1_i>0.50
		     if(led2_i-led1_i>=0.15)
			 {
				if(led3_i-led1_i>=0.50)
				{
					printf("green=%f(>=0.15mA)\r\n",led2_i-led1_i);
					printf("red=%f(>=0.50mA)\r\n",led3_i-led1_i);
					printf("led_ok\r\n");
					break;
				}
				else
				{
					printf("4.1 red_led_error=%fmA(red:>=0.50mA)\r\n",led3_i-led1_i);
					test_fail();
				}
			 }
			 else
			 {
				if(led3_i-led1_i>=0.5)
				{
					printf("4.2 green_led_error=%fmA(green:>=0.15mA)\r\n",led2_i-led1_i);
				}
				else
				{
					printf("4.2 green_led_error=%fmA(green:>=0.15mA)  red_led_error=%fmA(red:>=0.50mA)\r\n",led2_i-led1_i,led3_i-led1_i);
				}
				test_fail();
			 }
	   }

	}
	
	
	try_time=20;
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"\r\n");//退出LED模式
//		time1=systime_count/5;
////		printf("send_enter\r\n");
//		while(1)
//		{
//			time2=systime_count/5;
//			if(time2-time1>500)
//			{
//				break;
//			}
//		}
		if(str_temp[1]=='T'&&str_temp[2]=='H'&&str_temp[3]=='9'&&str_temp[4]=='0'&&str_temp[5]=='2')
		{
			try_time=3;
			break;
		}
		delay_ms(100);
	}while(try_time--);
	
	
	#if 0
/*****************************************
	while(1)
	{
		Single_Write_BH1750(0x01);   // power on
		Single_Write_BH1750(0x10);   // H- resolution mode
		delay_ms(180);               //延时180ms
		mread();                     //连续读出数据，存储在BUF中
		dis_data=BUF[0];
		dis_data=(dis_data<<8)+BUF[1]; //合成数据 
		temp=(float)dis_data/1.2;
		Light_Buf[light_i]=temp;
//		printf("%f\r\n",Light_Buf[light_i]);
		light_i++;
		if(light_i==LIGHT_NUMBER)
		{
			light_i=0;
			array_sort(Light_Buf,LIGHT_NUMBER);
			if(Light_Buf[0]>LIGHT_MIN)
			{
				if(1.2<Light_Buf[LIGHT_NUMBER-1]/Light_Buf[0]&&Light_Buf[LIGHT_NUMBER-1]/Light_Buf[0]<4)
				{
					printf("G/R=%f(1.2~4.0)\r\n",Light_Buf[LIGHT_NUMBER-1]/Light_Buf[0]);
					for(i=0;i<LIGHT_NUMBER;i++)
					{
//						printf( "%f LX\r\n", Light_Buf[i]);
					}
					th902_r();
					break;
				}
				else
				{
					printf("LED_danger!G/R=%f\r\n",Light_Buf[LIGHT_NUMBER-1]/Light_Buf[0]);
					for(i=0;i<LIGHT_NUMBER;i++)
					{
						printf( "%f LX\r\n", Light_Buf[i]);
					}
					test_fail();
				}
				
			}
			
			else if(Light_Buf[0]<LIGHT_MIN)
			{
				printf("LED_bad！\r\n");
				for(i=0;i<LIGHT_NUMBER;i++)
				{
					printf( "%f LX\r\n", Light_Buf[i]);
				}
				printf("\r\n");
				test_fail();
			}

		}		 
	}
	
*****************************************************************************/
	#endif

	
		
}

int light_value(void)
{
	char  light_i=0;
	float temp=0;
	int   light_val=0;
	while(1)
	{
		Single_Write_BH1750(0x01);   // power on
		Single_Write_BH1750(0x10);   // H- resolution mode
		delay_ms(180);            //延时180ms
		mread();                   //连续读出数据，存储在BUF中
		dis_data=BUF[0];
		dis_data=(dis_data<<8)+BUF[1]; //合成数据 
		temp=(float)dis_data/1.2;
		Light_Buf[light_i]=temp;
		light_i++;
		if(light_i==LIGHT_NUMBER)
		{
			light_i=0;
			array_sort(Light_Buf,LIGHT_NUMBER);
			light_val=delect_average(Light_Buf,0,LIGHT_NUMBER);
			return light_val;
		}
	}
}

//读取RTC值
int rtc_read(void)
{
	char time=5;
	u8 temp_len=0;
	uart2_init(9600);
	delay_ms(100);   //等待串口平稳
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		Usart_SendString(USART2,"t_rtcadj\r\n");
		delay_ms(300);
		if(str_temp[9]=='R'&&str_temp[10]=='T'&&str_temp[11]=='C')
		{
			if(str_temp[17]=='-')
			{
				temp_len=strlen((char *)str_temp)-25;
				switch(temp_len)
				{
					case 1:
						return -(str_to_value(str_temp[18]));
					case 2:
						return -(str_to_value(str_temp[18])*10+str_to_value(str_temp[19]));
					case 3:
						return -(str_to_value(str_temp[18])*100+str_to_value(str_temp[19])*10+str_to_value(str_temp[20]));
					case 4:
						return -(str_to_value(str_temp[18])*1000+str_to_value(str_temp[19])*100+str_to_value(str_temp[20])*10+str_to_value(str_temp[21]));
					default:
						break;
				}
			}
			else
			{
				temp_len=strlen((char *)str_temp)-24;
				switch(temp_len)
				{
					case 1:
						return (str_to_value(str_temp[17]));
					case 2:
						return (str_to_value(str_temp[17])*10+str_to_value(str_temp[18]));
					case 3:
						return (str_to_value(str_temp[17])*100+str_to_value(str_temp[18])*10+str_to_value(str_temp[19]));
					case 4:
						return (str_to_value(str_temp[17])*1000+str_to_value(str_temp[18])*100+str_to_value(str_temp[19])*10+str_to_value(str_temp[20]));
					default:
						break;
				}
			
			}

		}
	}while(time--);
	
	memset(str_temp,0,200*sizeof(char));
	USART_RX_STA=0;
	return 0;
}







