#include "stm32f10x.h"
#include "delay.h"

#include "dht11.h"

uint8_t Data[5]={0x00,0x00,0x00,0x00,0x00,};

void DHT_GPIO_SET_OUTPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DHT1_PORT_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = DHT11_Data_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_Data_PORT,&GPIO_InitStructure);
}

void DHT_GPIO_SET_INPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DHT1_PORT_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = DHT11_Data_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_Data_PORT,&GPIO_InitStructure);
}

uint8_t DHT_Read_Byte(void)  //从DHT11读取一位（8字节）信号
{
	 uint8_t ReadData=0;  //ReadData用于存放8bit数据，即8个单次读取的1bit数据的组合
	 uint8_t temp;      //临时存放信号电平（0或1）
	 uint8_t retry=0;   //retry用于防止卡死
	 uint8_t i;    
	 for(i=0; i<8; i++)   //一次温湿度信号读取八位
	 {
			while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==0 && retry<100)  
			//等待直到DHT11输出高电平：当PA5=1，上升沿，表示开始接受数据，可以判断0 or 1，跳出循环，执行后续判断（若PA5=0，将一直循环等待）
			{
					delay_us(1);
				  retry++;             //retry防止PA5读取不到数据卡死在这一步，当经历100us后retry自增到100，跳出循环。
			}
			retry=0;
			
			delay_us(40);    //延时30us
			//根据时序图，DHT传回高电平信号维持26us~28us表示0，	维持70us表示1
		  //延时30us后，如果IO读取到仍是高电平，说明采集到1；如果IO读取到低电平，说明采集到0
			//读取电平信号暂存temp内，随后会压入ReadData中
			if(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==1)   temp=1;
			 else   temp=0;

			 while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==1 && retry<100)
			//等待直到DHT11输出低电平，表示退出。本轮1bit信号接收完毕。
			 {
				 delay_us(1);
				 retry++;
			 }
			 retry=0;
			 
			 ReadData<<=1;    //ReadData内信号先全部左移一位，空出末尾位置
			 ReadData |= temp;        //将temp写入ReadData
	 }

		return ReadData;
}

/*
     uint8_t DHT_Read(void)表达完整时序
*/
uint8_t DHT_Read(void)
{
	 uint8_t retry=0;
	 uint8_t i;
		
	 DHT_GPIO_SET_OUTPUT();    //IO设置为输出模式。在传输的最开始，MCU要向DHT11发送信号
	 GPIO_ResetBits(DHT11_Data_PORT,DHT11_Data_PIN);   //IO->DHT11:先拉低电平18ms（应时序要求）
	 delay_ms(18);
	 GPIO_SetBits(DHT11_Data_PORT,DHT11_Data_PIN);   //IO->DHT11:随后拉高电平20us
	 delay_us(20);
	
	//MCU通过IO向DHT11发送请求完毕。接下来DHT11向IO发送响应,IO转为输入模式。在这之后就开始信号的转译读取。
	DHT_GPIO_SET_INPUT();
	delay_us(20);
	if(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==0) //DHT11发回低电平响应（读取到低电平，说明DHT11有响应）
	{
		//接下来，DHT11拉低电平一段时间后拉高电平一段时间
		while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==0 && retry<100)  
		{
		   delay_us(1);
			 retry++;
		}
		retry=0;
		while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==1 && retry<100) 
		{
		   delay_us(1);
			 retry++;
		}
		retry=0;
		
		//一共传输40位，一次DHT_Read_Byte返回8位，共读取5次。存储在Data[]中。（Data[]定义为全局）
		for(i=0; i<5; i++)
		{
			 Data[i] = DHT_Read_Byte();  //每次读取一字节（8位）
		}
		delay_us(50);
		//说明：Data[0]湿度， Data[2]温度。Data[1]和Data[3]分别为0和2的小数位。Data[4]用于校验。
	}
	
	 uint32_t sum=Data[0]+Data[1]+Data[2]+Data[3];  //校验
	 if((sum)==Data[4])    return 1;  
	   else   return 0;
	
}

