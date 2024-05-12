/*
2.��ʾģ�飺�������
3.����APP����
4.������λ������
5.��Ŀ��ɣ�LD3320����ʶ��ģ��
6.��Ŀ���ܣ�ʶ�����úõĿ����ͨ�����ڷ���ʶ����Ϣ��
7.��Ҫԭ������ο�LD3320�����ֲ�
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
//Microcontroller head file
#include "stm32f10x.h"
#include "sys.h"

//Hardware drivers
#include "delay.h"
#include "usart.h"
#include "ad.h"

//Peripheral modules
#include "Reg_RW.h"
#include "LDChip.h"
#include "esp8266.h"
#include "led.h"
#include "fan.h"
#include "dht11.h"
#include "oled.h"
#include "onenet.h"

//C library
#include <string.h>

//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
//*****************************************************
//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

#define TASK1_TASK_PRIO		6
#define TASK1_STK_SIZE 		50  
TaskHandle_t Task1Task_Handler;
void task1_task(void *pvParameters);

#define TASK2_TASK_PRIO		7
#define TASK2_STK_SIZE 		50  
TaskHandle_t Task2Task_Handler;
void task2_task(void *pvParameters);

#define TASK3_TASK_PRIO		5
#define TASK3_STK_SIZE 		100  
TaskHandle_t Task3Task_Handler;
void task3_task(void *pvParameters);

#define TASK4_TASK_PRIO		2
#define TASK4_STK_SIZE 		200  
TaskHandle_t Task4Task_Handler;
void task4_task(void *pvParameters);

#define TASK5_TASK_PRIO		3
#define TASK5_STK_SIZE 		200  
TaskHandle_t Task5Task_Handler;
void task5_task(void *pvParameters);

//*****************************************************
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n" //

void User_Modification(u8 dat);
void ASR_Recognition(void);

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;

uint16_t Light_Value = 0;//The value of the light sensor
uint16_t Brig_tar = 0;
uint16_t Temp_tar = 0;
uint8_t LED_Status;//LED status flags
uint8_t LED_PID_Status;//LED_PID status flags
uint8_t FAN_Status;//FAN status flags
uint8_t FAN_PID_Status;//FAN_PID status flags
uint8_t Data[5]={0x00,0x00,0x00,0x00,0x00,};

int main(void)
{	
	nAsrStatus = LD_ASR_NONE;		//	��ʼ״̬��û������ASR
	SCS=0;
	LED_Status = 0;
	LED_PID_Status = 0;
	FAN_Status = 0;
	unsigned short timeCount = 0;	//���ͼ������
	unsigned char *dataPtr = NULL;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�

	delay_init();
	OLED_Init();
	
	//Determine whether it is a watchdog reset
		if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		OLED_ShowString(4, 1, "IWDGRST");
		delay_ms(500);
		OLED_ShowString(4, 1, "       ");
		delay_ms(100);
		
		RCC_ClearFlag();
	}
	
	LD3320_Init();
	EXTIX_Init();
	
	LED_Init();
	FAN_Init();
	LD_Reset();
	
	AD_Init();

	uart1_init(115200);
	uart2_init(115200);

	
	
	//Configure a stand-alone watchdog
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(2343);					//15000ms
	IWDG_ReloadCounter();
	IWDG_Enable();
	
	ESP8266_Init();
	
	//Connect MQTT
	UsartPrintf(USART_DEBUG, "Connect MQTT Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))//Establish a TCP connection
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

//***************************************************************
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
//***************************************************************
/*
	while(1)
	{
		//Resets the standalone watchdog value
		IWDG_ReloadCounter();
		
		//Speech recognition detection
		ASR_Recognition(); 
		
		//Display light intensity
		Light_Value = AD_GetValue();//Obtaining light intensity
		OLED_ShowNum(3, 7,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 100), 2);//The units place is displayed as a percentage
		OLED_ShowNum(3, 10,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 10000) %100, 2);//Displays decimal places in percentage form
		
		//Display the temperature and humidity on the screen
		if(DHT_Read())
			{
				OLED_ShowNum(1,13,Data[2],2);
				OLED_ShowNum(1,16,Data[3],1);
				OLED_ShowNum(2,10,Data[0],2);
			}
			delay_ms(1000);
			
//		UsartPrintf(USART_DEBUG, "temp %d ,humi %d\r\n",Data[2],Data[0]);

		//Send data to OneNet
		if(++timeCount >= 10)									//���ͼ��5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OLED_ShowString(4,1,"                ");
				OLED_ShowString(4,1,"OneNet_SendDat");
				OneNet_SendData();									//��������
				timeCount = 0;
				ESP8266_Clear();
			}

		//Get OneNet data	
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
			delay_ms(10);
		
		//LED auto-dimming
		if(LED_PID_Status == 1)
			{
				uint16_t n = 60;
				uint16_t output;
				Light_Value = AD_GetValue();//Obtaining light intensity
				output = (uint16_t)LED_PID_Controller(n,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 100));
				LED_SetCompare2(output);
				printf("LED_PID%d\r\n",output);
				delay_ms(500);
			}
	}
	*/
}
//******************************************************
//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //����TASK2����
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 

		//����TASK3����
    xTaskCreate((TaskFunction_t )task3_task,     
                (const char*    )"task3_task",   
                (uint16_t       )TASK3_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK3_TASK_PRIO,
                (TaskHandle_t*  )&Task3Task_Handler); 
		//����TASK4����
    xTaskCreate((TaskFunction_t )task4_task,     
                (const char*    )"task4_task",   
                (uint16_t       )TASK4_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK4_TASK_PRIO,
                (TaskHandle_t*  )&Task4Task_Handler); 
		//����TASK5����
    xTaskCreate((TaskFunction_t )task5_task,     
                (const char*    )"task5_task",   
                (uint16_t       )TASK5_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK5_TASK_PRIO,
                (TaskHandle_t*  )&Task5Task_Handler);

    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}
//*********************************************************
void task1_task(void* pvParameters)
{
	while(1)
	{
		//Resets the standalone watchdog value
		IWDG_ReloadCounter();
		vTaskDelay(1000);
		printf("task1\r\n");
	}
}

void task2_task(void* pvParameters)
{
	while(1)
	{
		//Display light intensity
		Light_Value = AD_GetValue();//Obtaining light intensity
		OLED_ShowNum(3, 7,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 100), 2);//The units place is displayed as a percentage
		OLED_ShowNum(3, 10,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 10000) %100, 2);//Displays decimal places in percentage form
		vTaskDelay(1000);
		printf("task2\r\n");
		
		//Display the temperature and humidity on the screen
		if(DHT_Read())
			{
				OLED_ShowNum(1,13,Data[2],2);
				OLED_ShowNum(1,16,Data[3],1);
				OLED_ShowNum(2,10,Data[0],2);
			}
			printf("task3\r\n");
			vTaskDelay(1000);
			
		//LED auto-dimming
		if(LED_PID_Status == 1)
			{
				uint16_t LED_output;
				Light_Value = AD_GetValue();//Obtaining light intensity
				LED_output = (uint16_t)LED_PID_Controller(Brig_tar,100 - (uint16_t)(((float)(Light_Value - 100) / 2900) * 100));
				LED_SetCompare2(LED_output);
				printf("LED_PID: %d\r\n",LED_output);
			}
			printf("task6\r\n");
			vTaskDelay(1000);
			
		//FAN auto-dimming
		if(FAN_PID_Status == 1)
			{
				uint16_t FAN_output;
				FAN_output = (uint16_t)FAN_PID_Controller(Data[2],Temp_tar);
				FAN_SetCompare3(FAN_output);
				printf("FAN_PID: %d\r\n",FAN_output);
			}
			printf("task7\r\n");
			vTaskDelay(1000);
	}
}

void task3_task(void* pvParameters)
{
	while(1)
	{
		//Speech recognition detection
		ASR_Recognition();
		printf("task3\r\n");
		vTaskDelay(500);
	}
}

void task4_task(void* pvParameters)
{
	unsigned short timeCount = 0;	//���ͼ������
	while(1)
	{
		//Send data to OneNet
		if(++timeCount >= 10)									//���ͼ��5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OLED_ShowString(4,1,"                ");
				OLED_ShowString(4,1,"OneNet_SendDat");
				OneNet_SendData();									//��������
				timeCount = 0;
				ESP8266_Clear();
			}
			printf("task4\r\n");
			vTaskDelay(1000);
	}

}

void task5_task(void* pvParameters)
{
	unsigned char *dataPtr = NULL;
	while(1)
	{
		//Get OneNet data	
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
		OneNet_RevPro(dataPtr);
		delay_ms(10);
		printf("task5\r\n");
		vTaskDelay(1000);
	}
}

//******************************************************










//Speech recognition detection
void ASR_Recognition(void)
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
			case CODE_DMCS:		//������ơ�
					printf("���Ѵ�\r\n"); //text.....
//				GPIO_ResetBits(LED1_PORT,LED1_PIN);//����LED1
					LED_SetCompare2(100);
					LED_Status = 1;
												break;
			case CODE_CSWB:			//����صơ�
					printf("���ѹر�\r\n"); //text.....
//				GPIO_SetBits(LED1_PORT,LED1_PIN);//����LED1
					LED_SetCompare2(0);
					LED_PID_Status = 0;
					LED_Status = 0;
												break;
			
			case CODE_1KL1:	 //������ȼӡ�
				if(LED_GetCapture2() == 100)
				{
					printf("������Ϊ���\r\n"); 
				}
				else
				{
					LED_SetCompare2(LED_GetCapture2() + 25);
					LED_Status = 1;
					printf("����������\r\n");
				}					//text.....
												break;
				
			case CODE_1KL2:		//������ȼ���
				if(LED_GetCapture2() == 0)
				{
					LED_Status = 0;
					printf("������Ϊ��С\r\n"); 
				}
				else
				{
					LED_SetCompare2(LED_GetCapture2() - 25);
					printf("�����Ѽ�С\r\n");
				} 				//text.....
												break;
				
			case CODE_1KL3:	 //��������ȡ�
					printf("�����Ѵ�\r\n"); //text.....
					FAN_SetCompare3(100);
					FAN_Status = 1;
												break;
			
			case CODE_1KL4:		//����ط��ȡ�				
					printf("�����ѹر�\r\n"); //text.....
					FAN_SetCompare3(0);
					FAN_Status = 0;
			    FAN_PID_Status = 0;
												break;
			
			case CODE_2KL1:	 //����������⡱
					printf("�Զ������ѿ���\r\n"); //text.....
					LED_PID_Status = 1;
					LED_Status = 1;
												break;
			
			case CODE_2KL2:	 //����رյ��⡱
					printf("�Զ������ѹر�\r\n"); //text.....
					LED_PID_Status = 0;
					LED_Status = 0;
												break;
						
			case CODE_2KL3:	 //��������¶ȡ�
					printf("�����¶��ѿ���\r\n"); //text.....
					FAN_PID_Status = 1;
					FAN_Status = 1;
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
			default:break;
		}
	}
	else 	
	{
		printf("��˵��һ������\r\n"); //text.....
	}
}
