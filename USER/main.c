/*
2.显示模块：串口输出
3.配套APP：无
4.配套上位机：无
5.项目组成：LD3320语音识别模块
6.项目功能：识别设置好的口令，并通过串口返回识别信息。
7.主要原理：具体参考LD3320数据手册
	VCC与5V任接其一                                                     
	VCC接3.3V供电，5V接5V供电
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
//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
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
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	SCS=0;
	LED_Status = 0;
	LED_PID_Status = 0;
	FAN_Status = 0;
	unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级

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
	while(OneNet_DevLink())			//接入OneNET
		delay_ms(1000);
	OneNET_Subscribe();

	printf("运行程序\r\n");
	
	OLED_ShowString(1,1,"Temperature:00.0");
	OLED_ShowString(2,1,"Humidity:");
	OLED_ShowString(3, 1, "Light:00.00%");

//***************************************************************
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
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
		if(++timeCount >= 10)									//发送间隔5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OLED_ShowString(4,1,"                ");
				OLED_ShowString(4,1,"OneNet_SendDat");
				OneNet_SendData();									//发送数据
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
//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //创建TASK2任务
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 

		//创建TASK3任务
    xTaskCreate((TaskFunction_t )task3_task,     
                (const char*    )"task3_task",   
                (uint16_t       )TASK3_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK3_TASK_PRIO,
                (TaskHandle_t*  )&Task3Task_Handler); 
		//创建TASK4任务
    xTaskCreate((TaskFunction_t )task4_task,     
                (const char*    )"task4_task",   
                (uint16_t       )TASK4_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK4_TASK_PRIO,
                (TaskHandle_t*  )&Task4Task_Handler); 
		//创建TASK5任务
    xTaskCreate((TaskFunction_t )task5_task,     
                (const char*    )"task5_task",   
                (uint16_t       )TASK5_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK5_TASK_PRIO,
                (TaskHandle_t*  )&Task5Task_Handler);

    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
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
	unsigned short timeCount = 0;	//发送间隔变量
	while(1)
	{
		//Send data to OneNet
		if(++timeCount >= 10)									//发送间隔5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OLED_ShowString(4,1,"                ");
				OLED_ShowString(4,1,"OneNet_SendDat");
				OneNet_SendData();									//发送数据
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
				if (RunASR()==0)	//启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: //一次ASR识别流程结束，去取ASR识别结果
			{
				nAsrRes = LD_GetResult();		//获取结果											
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
* 名    称：用户执行函数 
* 功    能：识别成功后，执行动作可在此进行修改 
* 入口参数： 无 
* 出口参数：无
* 说    明： 					 
**********************************************************/

void User_Modification(u8 dat)
{
	if(dat ==0)
	{
		flag=1;
		printf("收到\r\n");
	}
	else if(flag)
	{
		flag=0;
		switch(nAsrRes)		   //对结果执行相关操作,客户修改
		{
			case CODE_DMCS:		//命令“开灯”
					printf("灯已打开\r\n"); //text.....
//				GPIO_ResetBits(LED1_PORT,LED1_PIN);//点亮LED1
					LED_SetCompare2(100);
					LED_Status = 1;
												break;
			case CODE_CSWB:			//命令“关灯”
					printf("灯已关闭\r\n"); //text.....
//				GPIO_SetBits(LED1_PORT,LED1_PIN);//点亮LED1
					LED_SetCompare2(0);
					LED_PID_Status = 0;
					LED_Status = 0;
												break;
			
			case CODE_1KL1:	 //命令“亮度加”
				if(LED_GetCapture2() == 100)
				{
					printf("亮度已为最大\r\n"); 
				}
				else
				{
					LED_SetCompare2(LED_GetCapture2() + 25);
					LED_Status = 1;
					printf("亮度已增加\r\n");
				}					//text.....
												break;
				
			case CODE_1KL2:		//命令“亮度减”
				if(LED_GetCapture2() == 0)
				{
					LED_Status = 0;
					printf("亮度已为最小\r\n"); 
				}
				else
				{
					LED_SetCompare2(LED_GetCapture2() - 25);
					printf("亮度已减小\r\n");
				} 				//text.....
												break;
				
			case CODE_1KL3:	 //命令“开风扇”
					printf("风扇已打开\r\n"); //text.....
					FAN_SetCompare3(100);
					FAN_Status = 1;
												break;
			
			case CODE_1KL4:		//命令“关风扇”				
					printf("风扇已关闭\r\n"); //text.....
					FAN_SetCompare3(0);
					FAN_Status = 0;
			    FAN_PID_Status = 0;
												break;
			
			case CODE_2KL1:	 //命令“开启调光”
					printf("自动调光已开启\r\n"); //text.....
					LED_PID_Status = 1;
					LED_Status = 1;
												break;
			
			case CODE_2KL2:	 //命令“关闭调光”
					printf("自动调光已关闭\r\n"); //text.....
					LED_PID_Status = 0;
					LED_Status = 0;
												break;
						
			case CODE_2KL3:	 //命令“调节温度”
					printf("调节温度已开启\r\n"); //text.....
					FAN_PID_Status = 1;
					FAN_Status = 1;
												break;
			
			
			
			case CODE_2KL4:	 //命令“....”/
					printf("\"向右转\"识别成功\r\n"); //text.....
															break;
						
			case CODE_3KL1:	 //命令“....”
					printf("\"打开空调\"识别成功\r\n"); //text.....
												break;
			case CODE_3KL2:	 //命令“....”
					printf("\"关闭空调\"识别成功\r\n"); //text.....
												break;
			case CODE_5KL1:	 //命令“....”
					printf("\"后退\"识别成功"); //text.....
												break;
//		case CODE_3KL4:	 //命令“....”
//				printf("\"代码测试\"识别成功"); //text.....
//											break;
//					
//					case CODE_4KL1:	 //命令“....”
//							printf("O"); //text.....
//														break;
//					case CODE_4KL2:	 //命令“....”
//							printf("P"); //text.....
//														break;
			default:break;
		}
	}
	else 	
	{
		printf("请说出一级口令\r\n"); //text.....
	}
}
