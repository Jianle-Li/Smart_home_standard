#ifndef _I2C_GPIO_H
#define _I2C_GPIO_H

#include "stm32f10x.h"
#include "delay.h"

#define SDA		GPIO_Pin_7		//�����Լ������Ž����޸�
#define SCL		GPIO_Pin_6		//�����Լ������Ž����޸�
#define I2C_port	GPIOA		//�����Լ������Ž����޸�
	
#define SDA_H	GPIO_SetBits(I2C_port,SDA)	//SDA�ߵ�ƽ
#define SCL_H	GPIO_SetBits(I2C_port,SCL)	//SCL�ߵ�ƽ
#define SDA_L	GPIO_ResetBits(I2C_port,SDA)//SDA�͵�ƽ
#define SCL_L	GPIO_ResetBits(I2C_port,SCL)//SCL�͵�ƽ	

void I2C_GPIO_Config(void);	//GPIOģ��I2C�����ų�ʼ��					
void I2C_Start(void);		//��ʼ�ź�
void I2C_Stop(void);		//ֹͣ�ź�
uint8_t I2C_Wait_Ack(void);	//����Ӧ���ź�
void I2C_Ack(void);			//����Ӧ���ź�
void I2C_nAck(void);		//���ͷ�Ӧ���ź�
void I2C_Send_Byte(uint8_t Dat);	//����һ���ֽ�
uint8_t I2C_Read_Byte(unsigned char Ack);	//��һ���ֽ�
uint8_t I2C_Write(uint8_t Add,uint8_t Reg,uint8_t Dat);		//I2Cдһ������
uint8_t I2C_Write_Len(uint8_t Add,uint8_t Reg,uint8_t Len,uint8_t *Buf);	//I2Cд�������
uint8_t I2C_Read(uint8_t Add,uint8_t Reg);					//I2C��һ������
uint8_t I2C_Read_Len(uint8_t Add,uint8_t Reg,uint8_t Len,uint8_t *Buf);		//I2C���������

#endif	

