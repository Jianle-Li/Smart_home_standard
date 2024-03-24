#ifndef _I2C_GPIO_H
#define _I2C_GPIO_H

#include "stm32f10x.h"
#include "delay.h"

#define SDA		GPIO_Pin_7		//根据自己的引脚进行修改
#define SCL		GPIO_Pin_6		//根据自己的引脚进行修改
#define I2C_port	GPIOA		//根据自己的引脚进行修改
	
#define SDA_H	GPIO_SetBits(I2C_port,SDA)	//SDA高电平
#define SCL_H	GPIO_SetBits(I2C_port,SCL)	//SCL高电平
#define SDA_L	GPIO_ResetBits(I2C_port,SDA)//SDA低电平
#define SCL_L	GPIO_ResetBits(I2C_port,SCL)//SCL低电平	

void I2C_GPIO_Config(void);	//GPIO模拟I2C的引脚初始化					
void I2C_Start(void);		//起始信号
void I2C_Stop(void);		//停止信号
uint8_t I2C_Wait_Ack(void);	//接收应答信号
void I2C_Ack(void);			//发送应答信号
void I2C_nAck(void);		//发送非应答信号
void I2C_Send_Byte(uint8_t Dat);	//发送一个字节
uint8_t I2C_Read_Byte(unsigned char Ack);	//读一个字节
uint8_t I2C_Write(uint8_t Add,uint8_t Reg,uint8_t Dat);		//I2C写一个数据
uint8_t I2C_Write_Len(uint8_t Add,uint8_t Reg,uint8_t Len,uint8_t *Buf);	//I2C写多个数据
uint8_t I2C_Read(uint8_t Add,uint8_t Reg);					//I2C读一个数据
uint8_t I2C_Read_Len(uint8_t Add,uint8_t Reg,uint8_t Len,uint8_t *Buf);		//I2C读多个数据

#endif	

