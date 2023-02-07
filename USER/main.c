#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "esp8266.h"
#include "onenet.h"
#include "beep.h"
#include "dht11.h"
 
/************************************************
 ALIENTEK��ӢSTM32������ʵ��13
 TFTLCD��ʾʵ��  
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

 int main(void)
 {	 	    
	u8 temperature;  	    
	u8 humidity;  
	 
	char PUB_BUF[256];    		//�ϴ����ݵ�BUF
	const char *devSubTopic[] = {"/timedisplay/sub"};
 	const char devPubTopic[] = "/timedisplay/pub";
	unsigned short timeCount = 0;	//���ͼ������
	unsigned char *dataPtr = NULL;
	
	 
	delay_init();	    	    //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	Usart1_Init(115200);	 	//debug���ڴ�ӡ
	Usart3_Init(115200);		//esp8266����
 	LED_Init();			    	  //LED�˿ڳ�ʼ��
	LCD_Init();
	BEEP_Init();
 
	 
	

	 
	LCD_Clear(WHITE);			
	POINT_COLOR=RED;	  
	 
	LCD_ShowString(30,40,210,24,24,"STM32 + ESP8266"); 
	LCD_ShowString(30,90,200,16,16,"2021/10/9");
//	LCD_ShowString(30,110,200,16,16,"21:14:00");
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n"); 
		
	ESP8266_Init();	
	while(OneNet_DevLink())			//����OneNET
		delay_ms(500);
	BEEP = 1;//���н���ɹ�
	
	while(DHT11_Init())	//DHT11��ʼ��	
	{
		LCD_ShowString(30,110,200,16,16,"DHT11 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}								   
	LCD_ShowString(30,110,200,16,16,"DHT11 OK");
	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
 	LCD_ShowString(30,130,200,16,16,"Temp:  C");	 
 	LCD_ShowString(30,150,200,16,16,"Humi:  %");
	delay_ms(250);   
	BEEP = 0;	
	OneNet_Subscribe(devSubTopic, 1);
  
	while(1) 
	{	
		if(timeCount % 40 == 0)				//1s��תһ��
		{
			LED0=!LED0;			

			DHT11_Read_Data(&temperature,&humidity);	//��ȡ��ʪ��ֵ					    
			LCD_ShowNum(30+40,130,temperature,2,16);	//��ʾ�¶�	   		   
			LCD_ShowNum(30+40,150,humidity,2,16);		//��ʾʪ��
			if (LED1 == 0)	
				LCD_ShowString(30,170,200,16,16,"LED1: ON ");
			else
				LCD_ShowString(30,170,200,16,16,"LED1: OFF");
			printf("temp: %d      humi: %d\r\n", temperature, humidity);	//���ڴ�ӡ
		}
		if(++timeCount >= 200)									//���ͼ��5s
		{
			UsartPrintf(USART_DEBUG, " OneNet_Publish\r\n");    //���̵���һ���ַ�
			sprintf(PUB_BUF, "{\"temp\":%d,  \"humi\":%d}", temperature, humidity);
			OneNet_Publish(devPubTopic, PUB_BUF);
			
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(3);   //15ms
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
		delay_ms(10);										//10ms
	} 
}
	
