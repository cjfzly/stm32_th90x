#include "adc.h"
#include "delay.h"

		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3  8-9	


#define adc_adj 0

void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC2, ENABLE );	  //使能ADC2通道时钟
 	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	
	//PA0 PA1作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
    
	//PB0 PB1作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //复位ADC1 
	ADC_DeInit(ADC2);  //复位ADC2 
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Init(ADC2, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器  
  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	ADC_Cmd(ADC2, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	ADC_ResetCalibration(ADC2);	//使能复位校准 
	
	while(ADC_GetResetCalibrationStatus(ADC1)&&ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
	ADC_StartCalibration(ADC2);	 //开启AD校准
	
	while(ADC_GetCalibrationStatus(ADC1)&&ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}				  
//获得ADC值
//ch:通道值 0~3 
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

//获得ADC值
//ch:通道值 8-9
u16 Get_Adc2(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC2);	//返回最近一次ADC1规则组的转换结果
}


float  adc_i_value(u16 adc_value)
{
	float   tempt;
	tempt = (float)(4*adc_value)+10;
	return tempt;
}
float  adc_i_value1(float adc_value)
{
	float   tempt;
	tempt = (float)(165*adc_value/4096);
	return tempt;
}


u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
//		printf("%f\r\n",adc_i_value(Get_Adc(ch)));
		delay_ms(1);
	}
	return temp_val/times;
} 	 

float Get_Adc_Average2(u8 ch,u8 times)
{
	float temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc2(ch);
//		printf("%d\r\n",Get_Adc2(ch)+60);
		delay_ms(1);
	}
	return temp_val/times;
} 	 

//为了避免循环等待期间出现死机的情况，重新修改Get_Adc_Average2(u8 ch,u8 times)函数
float Get_Adc_Average3(u8 ch,u8 times)
{
	float temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc2(ch);
//		printf("%d\r\n",Get_Adc2(ch)+60);
		delay_ms(1);
	}
	return temp_val/times;
} 


u16 adc_value(u16 adc_value)
{
	u16 tempt;
	tempt = (u16)(6600*adc_value/4096);
	return tempt;
}

int get_adc_avg(u8 ch,int times)
{
	int AD_val=0;
	int a;
	for(a=0;a<times;a++)
		{
			AD_val+=Get_Adc(ch);
			delay_ms(1);
		}
	AD_val/=times;
	AD_val*=2;	
	return AD_val;
}




