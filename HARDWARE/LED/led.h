#ifndef _lED_H
#define _lED_H

#include "stm32f10x.h"

///*  LED时钟端口、引脚定义 */
//#define LED1_PORT 		GPIOB   
//#define LED1_PIN 			GPIO_Pin_10
//#define LED1_PORT_RCC		RCC_APB2Periph_GPIOB

void LED_Init(void);
void LED_SetCompare2(uint16_t Compare);
uint16_t LED_GetCapture2(void);
float LED_PID_Controller(float setpoint, float current_value);


#endif
