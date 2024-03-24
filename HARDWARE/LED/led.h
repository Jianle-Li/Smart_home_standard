#ifndef _lED_H
#define _lED_H

#include "stm32f10x.h"

/*  LED时钟端口、引脚定义 */
#define LED1_PORT 		GPIOB   
#define LED1_PIN 			GPIO_Pin_10
#define LED1_PORT_RCC		RCC_APB2Periph_GPIOB

#define LED2_PORT 			GPIOE   
#define LED2_PIN 			GPIO_Pin_5
#define LED2_PORT_RCC		RCC_APB2Periph_GPIOE	


void LED_Init(void);


#endif
