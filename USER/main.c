/*
1.项目名称：绿深旗舰店LD3320语音识别模块STM32F103C8T6测试程序
2.显示模块：串口输出
3.配套APP：无
4.配套上位机：无
5.项目组成：LD3320语音识别模块
6.项目功能：识别设置好的口令，并通过串口返回识别信息。
7.主要原理：具体参考LD3320数据手册
8.购买地址：https://lssz.tmall.com 或淘宝上搜索“绿深旗舰店”
10.版权声明：绿深旗舰店所有程序都申请软件著作权。均与本店产品配套出售，请不要传播，以免追究其法律责任！
接线定义:
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

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	delay_init();
	LD3320_Init();
	EXTIX_Init();
	
	LED_Init();
	
	LD_Reset();
	
	uart1_init(115200);
	uart2_init(115200);
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	SCS=0;
	Led_Status = 0;
	unsigned short timeCount = 0;	//发送间隔变量
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
	while(OneNet_DevLink())			//接入OneNET
		delay_ms(1000);
	OneNET_Subscribe();

	printf("运行程序\r\n");
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
		if(++timeCount >= 10)									//发送间隔5s
			{
				UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
				OneNet_SendData();									//发送数据
			
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
			case CODE_DMCS:		//命令“代码测试”
					printf("灯已打开\r\n"); //text.....
					GPIO_ResetBits(LED1_PORT,LED1_PIN);//点亮LED1
					Led_Status = 1;
												break;
			case CODE_CSWB:			//命令“测试完毕”
					printf("灯已关闭\r\n"); //text.....
					GPIO_SetBits(LED1_PORT,LED1_PIN);//点亮LED1
					Led_Status = 0;

												break;
			
			case CODE_1KL1:	 //命令“北京”
					printf("\"北京\"识别成功\r\n"); //text.....
												break;
			case CODE_1KL2:		//命令“上海”
		
					printf("\"上海\"识别成功\r\n"); //text.....
												break;
			case CODE_1KL3:	 //命令“开灯”
					printf("\"开灯\"识别成功\r\n"); //text.....
												break;
			case CODE_1KL4:		//命令“关灯”				
					printf("\"关灯\"识别成功\r\n"); //text.....
												break;
			
			case CODE_2KL1:	 //命令“....”
					printf("\"广州\"识别成功\r\n"); //text.....
												break;
			case CODE_2KL2:	 //命令“....”
					printf("\"深圳\"识别成功\r\n"); //text.....
												break;
			case CODE_2KL3:	 //命令“....”
					printf("\"向左转\"识别成功\r\n"); //text.....
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
//					case CODE_4KL3:	 //命令“....”
//							printf("Q"); //text.....
//														break;
//					case CODE_4KL4:	 //命令“....”
//							printf("R"); //text.....
//														break;
		
			default:break;
		}
	}
	else 	
	{
		printf("请说出一级口令\r\n"); //text.....
	}
	
}
