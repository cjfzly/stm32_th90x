/****************************************************************
**1���ں��ڿ���ϵͳ���жϣ����������жϣ������жϵ�����
**2���Զ����������,�Կ��ư���봮�ڽ��մ�����
**���չ�����
**��1����Ӷ�ʱ��AD������������Ҫ�ﵽ1K���ϣ��ɼ�2s
**��2����ʼ�ƶ���ѹ�ɼ����ƺ���
**��3����ʼ�Զ������Է�ʽ���ԣ������㲥������˯�ߵ������˳��
**��4����������Խ�uart3�򿪣��Ե�·���ٴ����޸�
2019/06/26���Ĵ��ڴ�ӡ��Ϣ��ͨ������

**�ṹ�Ż��Ľ�������20191011��
**��1���Ż�flash�洢��ʽ
**��2���Ż�waitingʱ����ԭ����ʱLED��˸��Ϊ��ʱ��������˸�������ڴ���������Ӧ
**��3�����waitingʱ��ϵͳ�����������������ʱ����2�и������ݣ�����buffer���������
**��4�������ڲ����ڼ�Դ���1�����ݽ��д��� 20191012
**��5�����Թ���������˯�ߵ����ڼ䣬�����������ȶ����� 20191014
**�������⣺��������˯�ߵ�����������������
*****************************************************************/
#include "usr.h"

volatile int adc_flag=0;
volatile char t3finish_flag=0;
volatile char uart_irflag=0;
volatile int  systime_count1; 


//�ö�ʱ������LED��
volatile int main_beep_adc_flag=0;
volatile char main_t2finish_flag=0;


double  adc_sum=0;
u16 t;


//��������
int j=50;
int y=0;
static char k=50;
float i_worke_average=0,i_sleep_average=0,i_base_average=0,i_tempt=0;


#define save_size 50						   
char tempt_save[save_size];

						  
//TH902_V1.1.1D_201904
char *STR_COMPRE[]={"TH902_V1.1.1","<INVALID_ERR>","<id=05L>","<57600>","<256000>","error!","ok!",
					"<TH902>","MAC=fc:0","Set min conn ERROR!","Set max conn ERROR!","Set min conn OK!",
					"Set max conn OK!","FLASH Test Passed! 32M","FLASH Test Passed! 64M","FLASH Test Passed! 128M"};						   				   
						   
//app1����ȫ�ֱ���
#define ADC_REC_LEN  			50  	//�����������ֽ��� 200
u8      str_ADC[ADC_REC_LEN];
//���������洢����
#define BASE_I_NUMBER 4
#define BASE_I_DELECT 1
float  base_i_num[BASE_I_NUMBER];
					
//���������洢����
#define WORK_I_NUMBER  15 //Ĭ������Ϊ15
#define WORK_DELECT    1  //ȥ�������������С�ĸ���������4 ����ȥ����������4�����������С��4����
#define WORK_I_MAX     70
#define WORK_I_MIN     15				
float  work_i_num[WORK_I_NUMBER];
float  work_i_temp[5];
					
//˯�ߵ����洢����
#define SLEEP_I_NUMBER 7 //Ĭ������7
#define SLEEP_DELECT   1 //ȥ�������������С�ĸ���������4 ����ȥ����������4�����������С��4����
#define SLEEP_I_MAX   20
#define SLEEP_I_MIN   0
float  sleep_i_num[SLEEP_I_NUMBER];


//�����õ����ж�ֵ
#define TRUE   1
#define FALSE  0

//flash�洢ϵͳ������Ϣ
#define FLASH_SAVE_ADDR  0X0801E000		//120kflash��ʼ����ϵͳ����,Ԥ��8k��Ϊϵͳ�����洢��Ԫ
#define FLASH_SAVE_ADDR1 0X0801E400     //��121kλ��
#define FLASH_SAVE_ADDR2 0X0801E800     //��122kλ��
#define FLASH_SAVE_ADDR3 0X0801EC00		//��123Kλ��
#define FLASH_SAVE_ADDR4 0X0801F000     //��124kλ��
#define FLASH_SAVE_ADDR5 0X0801F400     //��125kλ��

char th902_rtc_ch[1];

char  v_number=0;
int   rtc_v1=0.0;
int   rtc_v2=0.0;
int   f_ch=0;

char th902_v_save[50];
char th902_v_temp[50];
char th902_rtc_save[20]={'t','_','c','a','l',' '};
char th902_rtc_save1[20]={'t','_','c','a','l',' '};
char th902_rtc_save2[20]={' '},th902_rtc_save2_temp[20];
char th902_rtc_save3[20]={' '},th902_rtc_save3_temp[20];


void fuc_select(int i);
void select_onerow(int i);
void select_do(int i);
void fuc1(void);
char aly_time(char time[] , char i);
void rtc_write(void);
void rtc_judge(void);
float rtc_trance(char rtc[]);
void waiting(void);


//���������������˸�����3600Ϊ1s��
#define led_time 800  
void timer_run_led(int count_times,int time);

//����mac��������
void mac_translate(char *temp,u8 get[]);
u8 mac_get[20];
u8 MAC_get[20];
u8 a_A(u8 a_in);
void mac_MAC(u8 *mac_array,u8 MAC_array[]);

//���ڵ����������ȶ�
#define data_stable_number 6
#define data_work_off_value 20
#define data_sleep_off_value 10
#define data_stable_overtime 70  //��Ĭ��70--60�����ݣ�1.17*data_stable_number*�ɼ����������Ĭ�ϲɼ�10�����г�ʱ��



float data_stable_tempt[100];
int   data_stable_count=0;
int time_stable_begin=0,time_stable_end=0;
float data_if_stable(int number,float data,float off_data_value,int overtime);
char data_stable_flag=0;




void printf_system_infortation(void)
{
	printf("\r\n\r\ntest_board:%s\r\n",STR(BIN_NAME));					//��ӡ��ǰbin�ļ����ļ���
//	printf("****Compile time : %s %s****\r\n",__DATE__,__TIME__);
	printf("TH902_soft_version:%s\r\n",th902_v_save);
	printf("f_base:%s %d f_write:%s %d f_ch=%d\r\n",th902_rtc_save3_temp,rtc_v2,th902_rtc_save2_temp,rtc_v1,f_ch);
}

//ϵͳ��ʱ�˳�ʱ���¼
int time_begin=0,time_end=0;

void flash_get(void)
{
	int i=0;
	char soft_length=0;
	float padj=0.0;
	float padj2=0.0;
	
	//��ȡ�汾�Ų���
	flash_read_bites(FLASH_SAVE_ADDR,th902_v_temp,30);
	if(th902_v_temp[0]==0x55&&th902_v_temp[1]==0x5A)
    {
		soft_length=th902_v_temp[2];
		v_number=soft_length;
		for(i=0;i<soft_length;i++)
		{
			th902_v_save[i]=th902_v_temp[i+3];  
		}
    }
	
	//��ȡУ׼Ƶ��
	flash_read_bites(FLASH_SAVE_ADDR1,th902_rtc_save2,20);
	if(th902_rtc_save2[0]==0x55&&th902_rtc_save2[1]==0x5A)
    {
		soft_length=th902_rtc_save2[2];
		for(i=0;i<soft_length;i++)
		{
			th902_rtc_save2_temp[i]=th902_rtc_save2[i+3];  
			th902_rtc_save[6+i]=th902_rtc_save2[i+3]; 
		}
    }
	
	//��ȡУ��Ƶ��
	flash_read_bites(FLASH_SAVE_ADDR2,th902_rtc_save3,20);
	if(th902_rtc_save3[0]==0x55&&th902_rtc_save3[1]==0x5A)
    {
		soft_length=th902_rtc_save3[2];
		for(i=0;i<soft_length;i++)
		{
			th902_rtc_save3_temp[i]=th902_rtc_save3[i+3]; 
			th902_rtc_save1[6+i]=th902_rtc_save3[i+3]; 
		}
    }
	
	//��ȡrtcģʽѡ��
	flash_read_bites(FLASH_SAVE_ADDR3,th902_rtc_ch,1);
	if(th902_rtc_ch[0]=='0')
	{
		f_ch=0;
	}
	else if(th902_rtc_ch[0]=='1')
	{
		f_ch=1;
	}
	
	//RTC_WRITE
	padj=(32.768-rtc_trance(th902_rtc_save))*1000000/rtc_trance(th902_rtc_save);
	rtc_v1=padj/0.9535+0.5;
	
	//RTC_BASE
	padj2=(32.768-rtc_trance(th902_rtc_save1))*1000000/rtc_trance(th902_rtc_save1);
	rtc_v2=padj2/0.9535+0.5;
}

//ϵͳ��ʼ������
void main_init()
{
	/* ����ϵͳʱ��Ϊ72M */ 
   	SystemInit();
	
	delay_init();
	gpio_config();
	key_intit();
	
//	GPIOConfig();
//	Init_BH1750();       //��ʼ��BH1750
	
	delay_ms(100);
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	
	delay_ms(100);
	uart1_init(115200);       //��ʼ��stm32���ڣ�������Ϊ115200
	uart2_init(9600);         //��ʼ�����԰洮�ڣ�������Ϊ9600
	Adc_Init();
	
	//1M=1000K   72M=72000K   7199��Ƶ��72000k/7200=10k
	//���������Ҫ���ߵ�Ƶ�ʣ����Ե�����Ƶϵ��
	TIM1_Int_Init(1,7199);
	TIM2_Int_Init(1,7199); //10khz�ļ���Ƶ��
    TIM3_Int_Init(9,7199);//1Khz�ļ���Ƶ��
	TIM4_Int_Init(9,7199);//1Khz�ļ���Ƶ�� 1ms
	
	flash_get();
	memset(tempt_save,0,save_size*sizeof(char));
	memset(str_temp,0,200*sizeof(char));
	USART_RX_STA=0;
}

int main(void)
{	
	main_init();
	printf_system_infortation();
	
	
	waiting();
	while(1)
	{
		fuc1();    //TH902���Ժ���
		time_end=systime_count1/5000;
		if(time_end-time_begin>120)
		{
			printf("init overtime!\r\n");
			test_fail();
		}
    }
}
































































//�ȴ����Լܰ��º���
void waiting(void)
{
	int bat_v=0;
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	bat_v=adc_value(Get_Adc_Average3(ADC_Channel_1,10));
	delay_ms(100);
	
	//������ʱ��2�����ڿ���LED�Ƶ���˸
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIMx�����жϱ�־ 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM2�ж�,��������ж�
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM
	
	while(1)
	{
		check_uart_data();
        USART1_REC_DATA();
		bat_v=adc_value(Get_Adc_Average3(ADC_Channel_1,10));
		if(bat_v>3000)
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);
			break;
		}
		timer_run_led(main_beep_adc_flag,led_time);		
	}main_beep_adc_flag=0;
	
	//�رն�ʱ��2
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIMx�����жϱ�־ 
	TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE ); //ʹ��ָ����TIM2�ж�,��������ж�
	TIM_Cmd(TIM2, DISABLE);  //�ر�TIME2
	//�ر�����LED��
	close_all_led();
	
	
	 //�����˳�BootLoader����ʱ
	 uart2_init(115200);     
	 delay_ms(10);
	 th902_r();
	 uart2_init(9600);
	
	 //��ʱ��1��ϵͳ����״̬LED2
	 TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //���TIM1�����жϱ�־ 
	 TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM1�ж�,��������ж�
	 TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
	 time_begin=systime_count1/5000;
}


//ʱ���ַ����ݵ���ʵʱ��ת����������ֻ������TH90Xϵ�а汾��Ϣ����
//�����ȡ,֧��1000�굽9999����ж�
char aly_time(char time[] , char i)
{
	int   year=0,yue=0,tian=0;
	char  v_x=v_number-21;
	//��ʽ�汾
	if(i>0) 
	{
		//���ת��
		year=str_to_value(time[13+v_x])*1000+str_to_value(time[14+v_x])*100+str_to_value(time[15+v_x])*10+str_to_value(time[16+v_x]);
		yue=str_to_value(time[17+v_x])*10+str_to_value(time[18+v_x]);
		tian=str_to_value(time[19+v_x])*10+str_to_value(time[20+v_x]);
		if(2019<=year&&0<yue&&yue<13&&0<tian&&tian<31)
		{
			return TRUE;

		}
		else
		{
			printf("��ʽ�汾%d��%d��%d��",year,yue,tian);
			if('A'<=time[21+v_x]&&time[21+v_x]<='Z')
			{
				printf("%c��\r\n",time[21+v_x]);
			}
			else
			{
				printf("\r\n");
			}
			
			test_fail();
		}
		
	}//��ʽ��
	
	//���԰�
	else   
	{
		//���ת��
		year=str_to_value(time[14+v_x])*1000+str_to_value(time[15+v_x])*100+str_to_value(time[16+v_x])*10+str_to_value(time[17+v_x]);
		yue=str_to_value(time[18+v_x])*10+str_to_value(time[19+v_x]);
		tian=str_to_value(time[20+v_x])*10+str_to_value(time[21+v_x]);
		if(2019<=year&&0<yue&&yue<13&&0<tian&&tian<31)
		{
			return TRUE;

		}
		else
		{
			printf("���԰汾%d��%d��%d��",year,yue,tian);
			if('A'<=time[22+v_x]&&time[22+v_x]<='Z')
			{
				printf("%c��\r\n",time[22+v_x]);
			}
			else
			{
				printf("\r\n");
			}
			test_fail();	
		}
	}//���԰�
	return FALSE;
}

//RTCֵ��д��
void rtc_write(void)
{
	char time=10;
	uart2_init(9600);
	delay_ms(100);   //�ȴ�����ƽ��
	do
	{
		memset(str_temp,0,200*sizeof(char));
		USART_RX_STA=0;
		uart_sendstring(USART2,th902_rtc_save);
		delay_ms(100);
	}while(time--);
	delay_ms(100);
	if(rtc_read()==rtc_v1)
	{
		printf("rtc_write_ok=%d\r\n",rtc_read());
		return;
	}
	else
	{
		printf("3.2 rtc_write-error=rtc_read:%d   rtc_write:%d\r\n",rtc_read(),rtc_v1);
		test_fail();
	}
			
}

//RTCֵ��У��
void rtc_judge()
{	
	if(rtc_read()==rtc_v2)
	{
		printf("rtc_check_ok=%d\r\n",rtc_read());
		return;
	}
	else
	{
		printf("3.1 rtc_check_error=rtc_read:%d  rtc_base:%d\r\n",rtc_read(),rtc_v2);
		test_fail();
	}	
}

//�����ж�������ӡͨ������
void fuc_select(int i)
{
	u8 temp_len=0;
	char v_y=v_number-21;
	switch(i)
	{
		//�汾���ж�
		case 0:
				if(version_comper(tempt_save,th902_v_save,v_number)==TRUE)
				{
					if(tempt_save[12+v_y]=='_')  //Ϊ��ʽ�汾����ʼ������ȡ�汾��Ϣ���бȽ�
					{
						aly_time(tempt_save,true);
					}
				}
				else
				{
					printf("1.1 soft_version_error��\r\n");
					printf("pre_version:%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n",
					tempt_save[0],tempt_save[1],tempt_save[2],tempt_save[3],tempt_save[4],tempt_save[5],
					tempt_save[6],tempt_save[7],tempt_save[8],tempt_save[9],tempt_save[10],tempt_save[11],
					tempt_save[12],tempt_save[13],tempt_save[14],tempt_save[15],
					tempt_save[16],tempt_save[17],tempt_save[18],tempt_save[19],tempt_save[20],tempt_save[21],
					tempt_save[22],tempt_save[23]);
					
					printf("save_version:%s\r\n",th902_v_save);
					if(tempt_save[12+v_y]=='D')
					{
						aly_time(tempt_save,false);
						test_fail();			   //���ε�����ɺ��ٴ򿪱��εİ汾�ж�
					}
					test_fail();
				}
				
		//RTC�ж�		
		case 1:
				if(str_temp[8]=='-')	//�ڲ���RTCΪ��ֵ����Ҫ�����ж��Ƿ�ͨ��
				{
					temp_len=strlen((char *)str_temp)-10;
					switch(temp_len)
					{
						case 1:
							printf("rtc=%d\r\n",-(str_to_value(str_temp[9])));
							break;
						case 2:
							printf("rtc=%d\r\n",-(str_to_value(str_temp[9])*10+str_to_value(str_temp[10])));
							break;
						case 3:
							printf("rtc=%d\r\n",-(str_to_value(str_temp[9])*100+str_to_value(str_temp[10])*10+str_to_value(str_temp[11])));
							break;
						case 4:
							printf("rtc=%d\r\n",-(str_to_value(str_temp[9])*1000+str_to_value(str_temp[10])*100+str_to_value(str_temp[11])*10+str_to_value(str_temp[12])));
							break;
						default:
							break;
					}
				}
				else 					//�ڲ���RTCΪ��ֵ����Ҫ�����ж��Ƿ�ͨ��
				{
					temp_len=strlen((char *)str_temp)-9;
					switch(temp_len)
					{
						case 1:
							printf("rtc=%d\r\n",(str_to_value(str_temp[8])));
							break;
						case 2:
							printf("rtc=%d\r\n",(str_to_value(str_temp[8])*10+str_to_value(str_temp[9])));
							break;
						case 3:
							printf("rtc=%d\r\n",(str_to_value(str_temp[8])*100+str_to_value(str_temp[9])*10+str_to_value(str_temp[10])));
							break;
						case 4:
							printf("rtc=%d\r\n",(str_to_value(str_temp[8])*1000+str_to_value(str_temp[9])*100+str_to_value(str_temp[10])*10+str_to_value(str_temp[11])));
							break;
						default:
							break;
					}
				
				}
			break;
		
		//����ģ���ж����¡��ɣ�
		case 2:
				if(A_to_B(tempt_save,STR_COMPRE[1])==TRUE)
				{
					printf("ble=old bluetooth\r\n");
					ble_flag=0;
				}
				else if(A_to_B(tempt_save,STR_COMPRE[2])==TRUE)
				{
					printf("ble=new bluetooth\r\n");
					ble_flag=1;
				}
			break;
		
		//���ڲ����ʵ��ж�
		case 3:
				if(A_to_B(tempt_save,STR_COMPRE[3])==TRUE)
				{
//					printf("���������������óɹ�\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[4])==TRUE)
				{
//					printf("���������������óɹ�\r\n");
				}
//				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
//				{
//					printf("���������ʵ��ж�ʧ��\r\n");
//					test_fail();
//				}
			break;
				
		//�������Ƶ��ж�		
		case 4:
				if(A_to_B(tempt_save,STR_COMPRE[7])==TRUE)
				{
//					printf("����������ȷ\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.2 get bt fac name:error!\r\n");
					printf("1.2 ble_error!  get bt fac name:error!\r\n");
					test_fail();
				}
			break;
		//�������Ʒ����ж�
		case 5:
				if(A_to_B(tempt_save,STR_COMPRE[6])==TRUE)
				{
//					printf("�������Ʒ�����ȷ\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.3 set bt fac name:error!\r\n");
					printf("1.2 ble_error!  set bt fac name:error!\r\n");
					test_fail();
				}
			break;
		
		//����MAC�ж�
		case 6:
				if(A_to_B(tempt_save,STR_COMPRE[8])==TRUE)
				{
//					printf("����MAC��ȷ\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.4 read bt mac:error!\r\n");
					printf("1.2 ble_error!  read bt mac:error!\r\n");
					test_fail();
				}
			break;
		
		//sn�����ж�
		case 7:
				if(A_to_B(tempt_save,STR_COMPRE[6])==TRUE)
				{
//					printf("SNУ��ͨ��\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.5 set bt sn:error!\r\n");
					printf("1.2 ble_error!  set bt sn:error!\r\n");
					test_fail();
				}
		
			break;
				
		//sn��ȡ�ж�
		case 8:
				if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.6 get bt sn:error!\r\n");
					printf("1.2 ble_error!  get bt sn:error!\r\n");
					test_fail();
				}
				
			break;
		
		//������С���Ӽ���ж�
		case 9:
				if(A_to_B(tempt_save,STR_COMPRE[11])==TRUE)
				{
//					printf("������С���Ӽ���ɹ�");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[9])==TRUE)
				{
//					printf("1.7 Set min conn:error!\r\n");
					printf("1.2 ble_error!  Set min conn:error!\r\n");
					test_fail();
				}
			break;
		
		//����������Ӽ���ж�
		case 10:
				if(A_to_B(tempt_save,STR_COMPRE[12])==TRUE)
				{
//					printf("����������Ӽ���ɹ�");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[10])==TRUE)
				{
//					printf("1.8 Set max conn:error!\r\n");
					printf("1.2 ble_error!  Set max conn:error!\r\n");
					test_fail();
				}
				
			break;
				
		//�����ж������㲥�Ƿ����óɹ�
		case 11:
				if(A_to_B(tempt_save,STR_COMPRE[6])==TRUE)
				{
//					printf("�����㲥ͨ��\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[5])==TRUE)
				{
//					printf("1.9 set bt adv:error!\r\n");
					printf("1.2 ble_error!  set bt adv:error!\r\n");
					test_fail();
				}
			break;
				
		//����12֤��adxl362����ʧ��
		case 12:
					printf("1.3 adxl36 is offline!\r\n");
					test_fail();
			break;
		
		//FLASH_ID���
		case 13:
			
			break;
		
		//���FLASH��С,��������ܻ��������FLASH���ж�
		case 14:
				if(A_to_B(tempt_save,STR_COMPRE[13])==TRUE)
				{
//					printf("FLASH:32M\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[14])==TRUE)
				{
//					printf("FLASH:64M\r\n");
				}
				else if(A_to_B(tempt_save,STR_COMPRE[15])==TRUE)
				{
//					printf("FLASH:128M\r\n");
				}
			break;
		
		//TH902��ʼ��������־
		case 15:
			
			break;
		
		
		case 16:
			
			break;
		
		//FLASH���ʧ�ܱ�־
		case 17:
			    printf("1.4 flash test:error!\r\n");
				test_fail();
			break;
		default:
			break;
	}
}


//�Բ��ҵ����ַ�������Ӧ��ִ�к���
void select_do(int i)
{
	switch(i)
	{
		case 0:
				printf("--------------1.SELF_CHECK-----%dms\r\n",systime_count1/5);
				printf("version:%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n",
					tempt_save[0],tempt_save[1],tempt_save[2],tempt_save[3],tempt_save[4],tempt_save[5],
					tempt_save[6],tempt_save[7],tempt_save[8],tempt_save[9],tempt_save[10],tempt_save[11],
					tempt_save[12],tempt_save[13],tempt_save[14],tempt_save[15],
					tempt_save[16],tempt_save[17],tempt_save[18],tempt_save[19],tempt_save[20],tempt_save[21],
					tempt_save[22],tempt_save[23],tempt_save[24]);
				fuc_select(0);
		        break;
		case 2:
				fuc_select(2);
		        break;
		case 3:
				fuc_select(3);
		        break;
        case 4:
				fuc_select(4);
		        break;
		case 5:
				fuc_select(5);
		        break;		
		case 6:
				//���ڻ�ȡ��������
				mac_translate(tempt_save,mac_get);
				mac_MAC(mac_get,MAC_get);
		
				printf("mac=%s\r\n",MAC_get);
				fuc_select(6);
		        break;
		case 7:
				fuc_select(7);
		        break;
		case 8:
				fuc_select(8);
		        break;
		case 11:
				fuc_select(11);
		        break;	
		case 12:
				fuc_select(12);
		        break;
		case 13:
				fuc_select(14);
		        break;
		case 15:                    //���ڼ�����
				printf("--------------2.BEEP_CHECK-----%dms\r\n",systime_count1/5);
		        beep_test();
				printf("--------------3.RTC_ADJ--------%dms\r\n",systime_count1/5);
				if(f_ch==0)
				{
					rtc_judge();
				}
				else if(f_ch==1)
				{
					rtc_write();
				}
				printf("--------------4.LED_CHECK------%dms\r\n",systime_count1/5);
				light_test();
				delay_ms(5000);
				printf("--------------5.CHARGE_V-------%dms\r\n",systime_count1/5);
				test_v();
				delay_ms(1000);
				printf("--------------6.CHARGE_I-------%dms\r\n",systime_count1/5);
				test_charge_i();
				printf("--------------7.BASE_I---------%dms\r\n",systime_count1/5);
				base_i();
		        break;	
		default:
			    break;
	}
	

}
//���ڻ�ȡͬһ���ַ����е��ַ�
void select_onerow(int i)
{
	switch(i)
	{
		case 1:
				fuc_select(1);
		        break;
		case 9:
		        fuc_select(9);
		        break;
		case 10:
				fuc_select(10);
		        break;
		case 13:
				fuc_select(13);
		        break;
		case 16:
				fuc_select(16);
		        break;
		case 17:
				fuc_select(17);
				break;
		default:
			    break;
	}

}


/**************************************************************
**TH902���Թ��ܺ���
**
**
**
**
***************************************************************/
void fuc1(void)
{
	
	if(uart_irflag==1)
	{
		uart_irflag=0;
		memset(str_temp,0,200*sizeof(char));
	}
	if(usrt_flg>0)
	{
		j=compar_str(str_temp);
		USART_RX_STA=0;
		memset(tempt_save,0,save_size*sizeof(char));
		//������ȡĿ�����һ������
		if(usrt_flg2==2)
		{
			for(t=0;t<35;t++)
			{
				tempt_save[t]=str_temp[t];
			}
			select_do(k);
			usrt_flg2=3;
		}
		//������ȡͬһ�е�����
		if(j==0||j==1||j==2||j==3||j==4||j==5||j==6||j==7||j==8||j==9||j==10||j==11||j==12||j==13||j==14||j==15||j==16)
		{
			if(j==14)
			{
				select_onerow(14);
			}
			if(j==1)
			{
				select_onerow(1);
			}
			if(j==9)
			{
				select_onerow(9);
			}
			if(j==10)
			{
				select_onerow(10);
			}
			if(j==13)
			{
				select_onerow(13);
			}
			if(j==16||j==18)
			{
				select_onerow(16);
			}
			if(j==17)
			{
				select_onerow(17);
			}
			 usrt_flg2=1;
			 k=j;
		}
		
	   //�˴�������������
		else
		{
			uart_irflag=1;
		}
		usrt_flg=0;
	}
		
	if(adc_flag>0&&t3finish_flag==1&&work_i_flag==1)
	{
		static int i=0;
		
		t3finish_flag=0;//1ms��ʱ����־
		//24ŷ��������
		adc_sum+=1.0*3300000/4096/480*Get_Adc(ADC_Channel_5);
		if(adc_flag==1000)
		{
			adc_flag=0;
			adc_sum=adc_sum/1000-i_base_average;
//			printf("work_i:%fuA\r\n",adc_sum);
			
			//���������ж��Ƿ����ȶ����״̬
			if(i==0)
			{
				i=10;
				time_stable_begin=systime_count1/5000;
			}
			i++;
			i_worke_average=data_if_stable(data_stable_number,adc_sum,data_work_off_value,data_stable_overtime);
			if(data_stable_flag==1)
			{
				work_i_flag=0; 
				sleep_i_flag=1;
				data_stable_flag=0;
				
				if(WORK_I_MIN<=i_worke_average&&i_worke_average<=WORK_I_MAX)
				{
					printf("work_i_ok=%fuA(15~70)\r\n",i_worke_average);
					
					TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); //ʹ��ָ����TIM3�ж�,��������ж�
					TIM_Cmd(TIM3, DISABLE);  //ʹ��TIMx
					printf("--------------10.SLEEP_I-------%dms\r\n",systime_count1/5);
					test_sleep_i();
				}
				else
				{
					TIM_Cmd(TIM3, DISABLE); 
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); 
					TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
					printf("9.1 work_i_error=%fuA(15~70)\r\n",i_worke_average);
					
					delay_ms(500);
					test_fail();
				}
			}
			if(data_stable_flag==2)
			{
				work_i_flag=0; 
				sleep_i_flag=1;
				data_stable_flag=0;
				
				TIM_Cmd(TIM3, DISABLE); 
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); 
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
				printf("9.2 work_i_overtime!\r\n");
				
				delay_ms(500);
				test_fail();	
			}		
		}
	}
		
	
	
	
	if(adc_flag>0&&t3finish_flag==1&&sleep_i_flag==1)
	{
		static int i=0;
		t3finish_flag=0;//1ms��ʱ����־
		//24ŷ��������
		adc_sum+=1.0*3300000/4096/480*Get_Adc(ADC_Channel_5);
		if(adc_flag==1000)
		{
			adc_flag=0;
			adc_sum=adc_sum/1000-i_base_average;
			printf("sleep_i:%fuA\r\n",adc_sum);
			
			//���������ж��Ƿ����ȶ����״̬
			if(i==0)
			{
				i=10;
				time_stable_begin=systime_count1/5000;
			}
			i++;
			
			i_sleep_average=data_if_stable(data_stable_number,adc_sum,data_sleep_off_value,data_stable_overtime);
			if(data_stable_flag==1)
			{
				sleep_i_flag=0;
				data_stable_flag=0;
				
				if(SLEEP_I_MIN<=i_sleep_average&&i_sleep_average<=SLEEP_I_MAX)
				{
					TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); //ʹ��ָ����TIM3�ж�,��������ж�
					TIM_Cmd(TIM3, DISABLE);  //ʹ��TIMx
					
					printf("sleep_i_ok=%fuA(0~20)\r\n",i_sleep_average);
					
					uart2_init(9600);
					GPIO_ResetBits(GPIOB,GPIO_Pin_4);    //����watch_gnd---GND
					delay_ms(500);
					GPIO_ResetBits(GPIOB,GPIO_Pin_3);    //�Ͽ�ģ����BGND��ͨBAT-
					delay_ms(10);
					GPIO_ResetBits(GPIOB,GPIO_Pin_7);    //�Ͽ��ⲿģ���ع���
					delay_ms(10);
					test_success();
				}
				else
				{
					TIM_Cmd(TIM3, DISABLE); 
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); 
					TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
					printf("10.1 sleep_i_error=%fuA(0~20)\r\n",i_sleep_average);
					
					delay_ms(500);
					test_fail();
				}
			}
			if(data_stable_flag==2)
			{
				sleep_i_flag=0;
				data_stable_flag=0;
				
				TIM_Cmd(TIM3, DISABLE); 
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); 
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
				printf("10.2 sleep_i_overtime!\r\n");
				
				delay_ms(500);
				test_fail();	
			}		
			
		}
	}
		
	if(adc_flag>0&&t3finish_flag==1&&base_i_flag==1)
	{
		static int i=0;
		t3finish_flag=0;//1ms��ʱ����־
		//24ŷ��������
		adc_sum+=1.0*3300000/4096/480*Get_Adc(ADC_Channel_5);
		if(adc_flag==1000)
		{
			adc_flag=0;
			adc_sum/=1000;
//			printf("base_i:%fuA\r\n",adc_sum);
			base_i_num[i]=adc_sum;
			i++;
			if(i==BASE_I_NUMBER)
			{
				i=0;
				base_i_flag=0;
				array_sort(base_i_num,BASE_I_NUMBER);
				i_base_average=delect_average(base_i_num,BASE_I_DELECT,BASE_I_NUMBER);
				printf("base_i_ok=%fuA(0~500)\r\n",i_base_average);
				
				//�����������󣬻�Ӱ����湤���������ľ���
				if(i_base_average>500)
				{
					printf("7.1 base_i_error=%fuA(0~500)\r\n",i_base_average);
					test_danger();
				}
				
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx�����жϱ�־ 
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); //ʹ��ָ����TIM3�ж�,��������ж�
				TIM_Cmd(TIM3, DISABLE);  //ʹ��TIMx
				printf("--------------8.WORK_V---------%dms\r\n",systime_count1/5);
				test_work_i();
				
			}
		}
	}

}


float rtc_trance(char rtc[])
{
	float rtc_value=0.0;
	rtc_value=str_to_value(rtc[6])*10+str_to_value(rtc[7])+1.0*(str_to_value(rtc[9])*10000000+str_to_value(rtc[10])*1000000+str_to_value(rtc[11])*100000
			  +str_to_value(rtc[12])*10000+str_to_value(rtc[13])*1000+str_to_value(rtc[14])*100+str_to_value(rtc[15])*10
			  +str_to_value(rtc[16]))/100000000;
	return rtc_value;
}


//����MAC��������
void mac_translate(char *temp,u8 get[])
{
	int i=4,j=0,count=0;
	for(i=4;i<strlen((char *)temp);i++)
	{
		if(count==5)
		{
			if(temp[i]!='\r')
			{
				get[j]=temp[i];
				j++;
			}
			else
			{
				break;
			}
		}
		else
		{
			if(temp[i]==':')
			{
				++count;
				get[j]=temp[i];
				j++;
			}
			else
			{
				get[j]=temp[i];
				j++;		
			}		
		}	
	}
}

void mac_MAC(u8 *mac_array,u8 MAC_array[])
{
	int i=0,m=0;
	int mac_a=0,mac_b=0,mac_c=0,mac_d=0,mac_e=0;
	int len=0;
	len=strlen((char *)mac_array);
	for (i=0;i<len;i++)
	{
		if(mac_array[i]==':')
		{
			++m;
			if(m==1)
			{
				mac_a=i;
			}
			else if(m==2)
			{
				mac_b=i;
			}
			else if(m==3)
			{
				mac_c=i;
			}
			else if(m==4)
			{
				mac_d=i;
			}
			else if(m==5)
			{
				mac_e=i;
			}
		}	
	}	
    if(mac_a==2)
	{
		MAC_array[0]=a_A(mac_array[0]);
		MAC_array[1]=a_A(mac_array[1]);
		MAC_array[2]=':';
	}		
	if(mac_b==4)
	{
		MAC_array[3]='0';
		MAC_array[4]=a_A(mac_array[3]);
		MAC_array[5]=':';
	}
	if(mac_b==5)
	{
		MAC_array[3]=a_A(mac_array[3]);
		MAC_array[4]=a_A(mac_array[4]);
		MAC_array[5]=':';
	}		
	if(mac_c==6)
	{
		MAC_array[6]='0';
		MAC_array[7]=a_A(mac_array[5]);
		MAC_array[8]=':';
	}
	if(mac_c==7)
	{
		MAC_array[6]=a_A(mac_array[5]);
		MAC_array[7]=a_A(mac_array[6]);
		MAC_array[8]=':';
	}	
	if(mac_d==8)
	{
		MAC_array[9]='0';
		MAC_array[10]=a_A(mac_array[7]);
		MAC_array[11]=':';
	}
	if(mac_d==9)
	{
		MAC_array[9]=a_A(mac_array[7]);
		MAC_array[10]=a_A(mac_array[8]);
		MAC_array[11]=':';
	}
	if(mac_e==10)
	{
		MAC_array[12]='0';
		MAC_array[13]=a_A(mac_array[9]);
		MAC_array[14]=':';
	}
	if(mac_e==11)
	{
		MAC_array[12]=a_A(mac_array[9]);
		MAC_array[13]=a_A(mac_array[10]);
		MAC_array[14]=':';
	}
	if((len-mac_e)==2)
	{
		MAC_array[15]='0';
		MAC_array[16]=a_A(mac_array[11]);
	}
	if((len-mac_e)==3)
	{
		MAC_array[15]=a_A(mac_array[11]);
		MAC_array[16]=a_A(mac_array[12]);
	}
		
}

u8 a_A(u8 a_in)
{
	if('a'<=a_in&&a_in<='z')
	{
		return a_in-32;
	}
	if('0'<=a_in&&a_in<='9')
	{
		return a_in;
	}
	if('A'<=a_in&&a_in<='Z')
	{
		return a_in;
	}
	return a_in;
}


//��ʱ�����������Ч��
void timer_run_led(int count_times,int time)
{
		if(0<=count_times&&count_times<time*1)
		{
			LED1=1;
			LED2=0; //��
			LED3=1;
		}	
		if(time*1<=count_times&&count_times<time*2)
		{
			LED1=1;
			LED2=1;
			LED3=0; //��
		}
		if(time*2<=count_times&&count_times<time*3)
		{
			LED1=0; //��
			LED2=1;
			LED3=1;
		}
		if(time*3<=count_times&&count_times<time*4)
		{
			LED1=1;
			LED2=1;
			LED3=0;
		}	
		if(count_times>=time*4)
		{
			main_beep_adc_flag=0;
		}
}


/******************�����ж������ȶ��ĺ���*******************
**����1���ɼ�������
**����2��ǰ�󸡶�ֵ
**����3����ʱʱ��
**����4�����ȶ�����
**����ֵ��
	     case 1:�����ȶ�ʱ�������ȶ������ƽ��ֵ
		 case 2:���ݲ��ȶ�ʱ�����ؿ�
**********************************************************/
float data_if_stable(int number,float data,float off_data_value,int overtime)
{
	int i=0;
	float abs_data;
	
	time_stable_end=systime_count1/5000;
	if((time_stable_end-time_stable_begin)>=overtime)
	{
		data_stable_flag=2;
//		printf("overtime=%ds\r\n",overtime);
		return NULL;
	}
	
	if(data_stable_count<number)
	{
		data_stable_tempt[data_stable_count]=data;
//		printf("data_tempt=%f\r\n",data_stable_tempt[data_stable_count]);
		data_stable_count++;		
	}
	else 
	{
		data_stable_count=0;
		for(i=0;i<number-1;i++)
		{
			if((data_stable_tempt[i+1]-data_stable_tempt[i])>=0)
			{
				abs_data=data_stable_tempt[i+1]-data_stable_tempt[i];
			}
			else
			{
				abs_data=data_stable_tempt[i]-data_stable_tempt[i+1];
			}
			if(abs_data<=off_data_value)
			{
				continue;
			}
			else
			{
//				printf("%d�鲻�ȶ������½���\r\n",number);
				data_stable_flag=0;
				memset(data_stable_tempt,0,100*sizeof(float));
				return NULL;
			}
		}
		data_stable_flag=1;
		array_sort(data_stable_tempt,number);
		i_tempt=delect_average(data_stable_tempt,0,number);
		memset(data_stable_tempt,0,100*sizeof(float));
//		printf("%d���ȶ���ͨ��\r\n",number);
		return i_tempt;	
	}
	return NULL;
}










