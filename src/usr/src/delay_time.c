#include "delay_time.h"
#include "stm32f10x.h"
//#define SOFT_DELAY        //?????
//#define SYSTICK_DELAY    //???????


#define TIMER_DELAY                                    TIM2
#define TIMER_DELAY_PERIOD                    666
#define TIMER_DELAY_PRESCALER                72
#define TIMER_DELAY_CLOCK                        RCC_APB1Periph_TIM2
#define TIMER_DELAY_IRQ_HANDLER            TIM4_IRQHandler

#define TIMER_DELAY_PREEMPTION_PRIORITY        2
#define TIMER_DELAY_SUB_PRIORITY                    2
#define TIMER_DELAY_IRQ_CHANNEL                        TIM4_IRQn

static __IO u16    counter_delay_ms;




//---------------------------------------------------------------------------


//???????. ????????????????, ???????,??do 
static u8 fac_us = 0;    //us?????
static u16 fac_ms = 0;    //ms?????


//-------------------------------------------------------------------



void decrement_delay(void)
{
    if(counter_delay_ms != 0)
    {
        /* Decrement the counter */
        counter_delay_ms--;
    }
}

void delay_init(u8 SYSCLK)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    //NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(TIMER_DELAY_CLOCK, ENABLE);

	  TIM_DeInit(TIMER_DELAY);
    TIM_TimeBaseStructure.TIM_Period                 = TIMER_DELAY_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler         = TIMER_DELAY_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER_DELAY, &TIM_TimeBaseStructure);
	
    TIM_ClearFlag(TIMER_DELAY,TIM_FLAG_Update);
	  TIM_ITConfig(TIMER_DELAY,TIM_IT_Update,ENABLE);
    //TIM_UpdateRequestConfig(TIMER_DELAY, TIM_UpdateSource_Global);
    //TIM_ClearITPendingBit(TIMER_DELAY, TIM_IT_Update);

    /* Enable TIMER Update interrupt */
    //TIM_ITConfig(TIMER_DELAY, TIM_IT_Update, ENABLE);

    // Disable timers
    TIM_Cmd(TIMER_DELAY, DISABLE);

//    /* Enable and set TIMER IRQ used for delays */
//    NVIC_InitStructure.NVIC_IRQChannel                                          = TIMER_DELAY_IRQ_CHANNEL;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority     = TIMER_DELAY_PREEMPTION_PRIORITY;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority                  = TIMER_DELAY_SUB_PRIORITY;
//    NVIC_InitStructure.NVIC_IRQChannelCmd                                 = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
}

void TIMER_DELAY_IRQ_HANDLER(void)
{
	  if (TIM_GetITStatus(TIMER_DELAY, TIM_IT_Update) != RESET) 
		{
				/* Clear TIMER update interrupt flag */
				TIM_ClearITPendingBit(TIMER_DELAY, TIM_IT_Update);
				decrement_delay();
		}
}

void delay_ms(uint16_t delay)
{
    counter_delay_ms = delay;

    TIM_SetCounter(TIMER_DELAY, 0);
    /* TIM4 enable counter */
    TIM_Cmd(TIMER_DELAY, ENABLE);
    /* Wait for 'delay' milliseconds */
    while(counter_delay_ms != 0){TIMER_DELAY_IRQ_HANDLER();};
    /* TIM4 disable counter */
    TIM_Cmd(TIMER_DELAY, DISABLE);
}



