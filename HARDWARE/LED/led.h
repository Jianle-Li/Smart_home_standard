#ifndef _lED_H
#define _lED_H

#include "stm32f10x.h"

/*  LEDʱ�Ӷ˿ڡ����Ŷ��� */
#define LED1_PORT 		GPIOB   
#define LED1_PIN 			GPIO_Pin_10
#define LED1_PORT_RCC		RCC_APB2Periph_GPIOB

void LED_Init(void);
extern uint8_t Led_Status;

#endif
