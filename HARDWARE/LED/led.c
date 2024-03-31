#include "led.h"

/*******************************************************************************
* 函 数 名         : LED_Init
* 函数功能		   : LED初始化函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
//void LED_Init(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;//定义结构体变量
//	
//	RCC_APB2PeriphClockCmd(LED1_PORT_RCC,ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin=LED1_PIN;  //选择你要设置的IO口
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	 //设置推挽输出模式
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
//	GPIO_Init(LED1_PORT,&GPIO_InitStructure); 	   /* 初始化GPIO */
//	GPIO_SetBits(LED1_PORT,LED1_PIN);   //将LED端口拉高，熄灭所有LED

//}

#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;		//GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
}

void LED_SetCompare2(uint16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);
}

uint16_t LED_GetCapture2(void)
{
	return TIM_GetCapture2(TIM2);
}