#ifndef _FAN_H
#define _FAN_H

#include "stm32f10x.h"

///*  LEDʱ�Ӷ˿ڡ����Ŷ��� */
//#define LED1_PORT 		GPIOB   
//#define LED1_PIN 			GPIO_Pin_10
//#define LED1_PORT_RCC		RCC_APB2Periph_GPIOB

void FAN_Init(void);
extern uint8_t FAN_Status;

void FAN_SetCompare3(uint16_t Compare);
uint16_t FAN_GetCapture3(void);

#endif