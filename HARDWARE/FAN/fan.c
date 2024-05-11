#include "fan.h"

/*******************************************************************************
* 函 数 名         : FAN_Init
* 函数功能		   : LED初始化函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

#include "stm32f10x.h"                  // Device header

// 定义PID控制器的参数
#define FAN_KP 2
#define FAN_KI 1
#define FAN_KD 0.5

// 定义PID控制器的变量
float FAN_previousError = 0;
float FAN_integral = 0;

void FAN_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		//GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	
	TIM_Cmd(TIM3, ENABLE);
}

void FAN_SetCompare3(uint16_t Compare)
{
	TIM_SetCompare3(TIM3, Compare);
}

uint16_t FAN_GetCapture3(void)
{
	return TIM_GetCapture3(TIM3);
}

// 计算PID控制器的输出
float FAN_PID_Controller(float setpoint, float current_value)
{
    float error = setpoint - current_value;// 计算误差
    FAN_integral += error;// 计算积分项
    float derivative = error - FAN_previousError;// 计算微分项
    FAN_previousError = error;
    float output = FAN_KP * error + FAN_KI * FAN_integral + FAN_KD * derivative;// 计算PID控制器的输出
    // 限制输出范围在合理的范围内
    if (output < 0)
		{
        output = 0;
    }
		else if (output > 100)
		{
        output = 100;
    }
		if (FAN_integral > 150)
		{
			FAN_integral = 150;
		}
		if (FAN_integral < -150)
		{
			FAN_integral = -150;
		}
    return output;
}