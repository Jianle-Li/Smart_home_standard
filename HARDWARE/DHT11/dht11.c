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

uint8_t DHT_Read_Byte(void)  //��DHT11��ȡһλ��8�ֽڣ��ź�
{
	 uint8_t ReadData=0;  //ReadData���ڴ��8bit���ݣ���8�����ζ�ȡ��1bit���ݵ����
	 uint8_t temp;      //��ʱ����źŵ�ƽ��0��1��
	 uint8_t retry=0;   //retry���ڷ�ֹ����
	 uint8_t i;    
	 for(i=0; i<8; i++)   //һ����ʪ���źŶ�ȡ��λ
	 {
			while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==0 && retry<100)  
			//�ȴ�ֱ��DHT11����ߵ�ƽ����PA5=1�������أ���ʾ��ʼ�������ݣ������ж�0 or 1������ѭ����ִ�к����жϣ���PA5=0����һֱѭ���ȴ���
			{
					delay_us(1);
				  retry++;             //retry��ֹPA5��ȡ�������ݿ�������һ����������100us��retry������100������ѭ����
			}
			retry=0;
			
			delay_us(40);    //��ʱ30us
			//����ʱ��ͼ��DHT���ظߵ�ƽ�ź�ά��26us~28us��ʾ0��	ά��70us��ʾ1
		  //��ʱ30us�����IO��ȡ�����Ǹߵ�ƽ��˵���ɼ���1�����IO��ȡ���͵�ƽ��˵���ɼ���0
			//��ȡ��ƽ�ź��ݴ�temp�ڣ�����ѹ��ReadData��
			if(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==1)   temp=1;
			 else   temp=0;

			 while(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==1 && retry<100)
			//�ȴ�ֱ��DHT11����͵�ƽ����ʾ�˳�������1bit�źŽ�����ϡ�
			 {
				 delay_us(1);
				 retry++;
			 }
			 retry=0;
			 
			 ReadData<<=1;    //ReadData���ź���ȫ������һλ���ճ�ĩβλ��
			 ReadData |= temp;        //��tempд��ReadData
	 }

		return ReadData;
}

/*
     uint8_t DHT_Read(void)�������ʱ��
*/
uint8_t DHT_Read(void)
{
	 uint8_t retry=0;
	 uint8_t i;
		
	 DHT_GPIO_SET_OUTPUT();    //IO����Ϊ���ģʽ���ڴ�����ʼ��MCUҪ��DHT11�����ź�
	 GPIO_ResetBits(DHT11_Data_PORT,DHT11_Data_PIN);   //IO->DHT11:�����͵�ƽ18ms��Ӧʱ��Ҫ��
	 delay_ms(18);
	 GPIO_SetBits(DHT11_Data_PORT,DHT11_Data_PIN);   //IO->DHT11:������ߵ�ƽ20us
	 delay_us(20);
	
	//MCUͨ��IO��DHT11����������ϡ�������DHT11��IO������Ӧ,IOתΪ����ģʽ������֮��Ϳ�ʼ�źŵ�ת���ȡ��
	DHT_GPIO_SET_INPUT();
	delay_us(20);
	if(GPIO_ReadInputDataBit(DHT11_Data_PORT,DHT11_Data_PIN)==0) //DHT11���ص͵�ƽ��Ӧ����ȡ���͵�ƽ��˵��DHT11����Ӧ��
	{
		//��������DHT11���͵�ƽһ��ʱ������ߵ�ƽһ��ʱ��
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
		
		//һ������40λ��һ��DHT_Read_Byte����8λ������ȡ5�Ρ��洢��Data[]�С���Data[]����Ϊȫ�֣�
		for(i=0; i<5; i++)
		{
			 Data[i] = DHT_Read_Byte();  //ÿ�ζ�ȡһ�ֽڣ�8λ��
		}
		delay_us(50);
		//˵����Data[0]ʪ�ȣ� Data[2]�¶ȡ�Data[1]��Data[3]�ֱ�Ϊ0��2��С��λ��Data[4]����У�顣
	}
	
	 uint32_t sum=Data[0]+Data[1]+Data[2]+Data[3];  //У��
	 if((sum)==Data[4])    return 1;  
	   else   return 0;
	
}

