#ifndef _DHT11_H
#define _DHT11_H


#include "stm32f10x.h"

#define DHT11_Data_PORT     GPIOA   
#define DHT11_Data_PIN 			GPIO_Pin_5
#define DHT1_PORT_RCC		RCC_APB2Periph_GPIOA



void DHT_GPIO_SET_OUTPUT(void);
void DHT_GPIO_SET_INTPUT(void);
uint8_t DHT_Read_Byte(void);
uint8_t DHT_Read(void);
extern uint8_t Data[5];

#endif
