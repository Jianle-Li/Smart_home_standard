/*
1.��Ŀ���ƣ������콢��LD3320����ʶ��ģ��STM32F103C8T6���Գ���
2.��ʾģ�飺�������
3.����APP����
4.������λ������
5.��Ŀ��ɣ�LD3320����ʶ��ģ��
6.��Ŀ���ܣ�ʶ�����úõĿ����ͨ�����ڷ���ʶ����Ϣ��
7.��Ҫԭ������ο�LD3320�����ֲ�
8.�����ַ��https://lssz.tmall.com ���Ա��������������콢�ꡱ
10.��Ȩ�����������콢�����г��������������Ȩ�����뱾���Ʒ���׳��ۣ��벻Ҫ����������׷���䷨�����Σ�
���߶���:
	VCC��5V�ν���һ                                                     
	VCC��3.3V���磬5V��5V����
	GND--GND
	SCK--PB7
	MI--PB6
	MO--PB5
	CS--PB4
	RST--PB3
	IRQ--PB8
	WR---GND
*/
#include "stm32f10x.h"
#include "delay.h"
#include "Reg_RW.h"
#include "LDChip.h"
#include "usart.h"

#include "sys.h"
#include "led.h"
#include "dht11.h"
#include "oled.h"
#include "ad.h"
#include "esp8266.h"
#include "onenet.h"
#include <string.h>

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"

void User_Modification(u8 dat);

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;

uint16_t ADValue;
uint8_t Led_Status;
int main(void)
{	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();
	LD3320_Init();
	EXTIX_Init();
	
	LED_Init();
	
	LD_Reset();
	
	uart1_init(115200);
	uart2_init(115200);
	nAsrStatus = LD_ASR_NONE;		//	��ʼ״̬��û������ASR
	SCS=0;
	Led_Status = 0;
	unsigned short timeCount = 0;	//���ͼ������
	unsigned char *dataPtr = NULL;

	
	OLED_Init();
	AD_Init();
	ESP8266_Init();
	
	//Connect MQTT
	UsartPrintf(USART_DEBUG, "Connect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		delay_ms(500);
	UsartPrintf(USART_DEBUG, "Connect MQTT Server Success\r\n");
	
	delay_ms(100);
	while(OneNet_DevLink())			//����OneNET
		delay_ms(1000);
	OneNET_Subscribe();

	printf("���г���\r\n");
	OLED_ShowString(1,1,"Temperature:00.0");
	OLED_ShowString(2,1,"Humidity:");
	OLED_ShowString(3, 1, "Light:00.00%");
	while(1)
	{
		
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:	
					 break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	//����һ��ASRʶ�����̣�ASR��ʼ����ASR��ӹؼ��������ASR����
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: //һ��ASRʶ�����̽�����ȥȡASRʶ����
			{
				nAsrRes = LD_GetResult();		//��ȡ���											
				User_Modification(nAsrRes);
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		} 
		//Display light intensity
		ADValue = AD_GetValue();//Obtaining light intensity
		OLED_ShowNum(3, 7,100 - (uint16_t)(((float)(ADValue - 100) / 2900) * 100), 2);//The units place is displayed as a percentage
		OLED_ShowNum(3, 10,100 - (uint16_t)(((float)(ADValue - 100) / 2900) * 10000) %100, 2);//Displays decimal places in percentage form
		
		//Display the temperature and humidity on the screen
		if(DHT_Read())
			{
				OLED_ShowNum(1,13,Data[2],2);
				OLED_ShowNum(1,16,Data[3],1);
				OLED_ShowNum(2,10,Data[0],2);
			}
			delay_ms(1000);
			
//		UsartPrintf(USART_DEBUG, "temp %d ,humi %d\r\n",Data[2],Data[0]);
		if(++timeCount >= 10)									//���ͼ��5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OneNet_SendData();									//��������
			
				timeCount = 0;
				ESP8266_Clear();
			}
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
			delay_ms(10);
		}
}

/***********************************************************
* ��    �ƣ��û�ִ�к��� 
* ��    �ܣ�ʶ��ɹ���ִ�ж������ڴ˽����޸� 
* ��ڲ����� �� 
* ���ڲ�������
* ˵    ���� 					 
**********************************************************/

void User_Modification(u8 dat)
{
	if(dat ==0)
	{
		flag=1;
		printf("�յ�\r\n");
	}
	else if(flag)
	{
		flag=0;
		switch(nAsrRes)		   //�Խ��ִ����ز���,�ͻ��޸�
		{
			case CODE_DMCS:		//���������ԡ�
					printf("���Ѵ�\r\n"); //text.....
					GPIO_ResetBits(LED1_PORT,LED1_PIN);//����LED1
					Led_Status = 1;
												break;
			case CODE_CSWB:			//���������ϡ�
					printf("���ѹر�\r\n"); //text.....
					GPIO_SetBits(LED1_PORT,LED1_PIN);//����LED1
					Led_Status = 0;

												break;
			
			case CODE_1KL1:	 //���������
					printf("\"����\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_1KL2:		//����Ϻ���
		
					printf("\"�Ϻ�\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_1KL3:	 //������ơ�
					printf("\"����\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_1KL4:		//����صơ�				
					printf("\"�ص�\"ʶ��ɹ�\r\n"); //text.....
												break;
			
			case CODE_2KL1:	 //���....��
					printf("\"����\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_2KL2:	 //���....��
					printf("\"����\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_2KL3:	 //���....��
					printf("\"����ת\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_2KL4:	 //���....��/
					printf("\"����ת\"ʶ��ɹ�\r\n"); //text.....
															break;
						
			case CODE_3KL1:	 //���....��
					printf("\"�򿪿յ�\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_3KL2:	 //���....��
					printf("\"�رտյ�\"ʶ��ɹ�\r\n"); //text.....
												break;
			case CODE_5KL1:	 //���....��
					printf("\"����\"ʶ��ɹ�"); //text.....
												break;
//		case CODE_3KL4:	 //���....��
//				printf("\"�������\"ʶ��ɹ�"); //text.....
//											break;
//					
//					case CODE_4KL1:	 //���....��
//							printf("O"); //text.....
//														break;
//					case CODE_4KL2:	 //���....��
//							printf("P"); //text.....
//														break;
//					case CODE_4KL3:	 //���....��
//							printf("Q"); //text.....
//														break;
//					case CODE_4KL4:	 //���....��
//							printf("R"); //text.....
//														break;
		
			default:break;
		}
	}
	else 	
	{
		printf("��˵��һ������\r\n"); //text.....
	}
	
}
