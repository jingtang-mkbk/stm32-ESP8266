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
 ALIENTEK精英STM32开发板实验13
 TFTLCD显示实验  
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

 int main(void)
 {	 	    
	u8 temperature;  	    
	u8 humidity;  
	 
	char PUB_BUF[256];    		//上传数据的BUF
	const char *devSubTopic[] = {"/timedisplay/sub"};
 	const char devPubTopic[] = "/timedisplay/pub";
	unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
	
	 
	delay_init();	    	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	Usart1_Init(115200);	 	//debug串口打印
	Usart3_Init(115200);		//esp8266串口
 	LED_Init();			    	  //LED端口初始化
	LCD_Init();
	BEEP_Init();
 
	 
	

	 
	LCD_Clear(WHITE);			
	POINT_COLOR=RED;	  
	 
	LCD_ShowString(30,40,210,24,24,"STM32 + ESP8266"); 
	LCD_ShowString(30,90,200,16,16,"2021/10/9");
//	LCD_ShowString(30,110,200,16,16,"21:14:00");
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n"); 
		
	ESP8266_Init();	
	while(OneNet_DevLink())			//接入OneNET
		delay_ms(500);
	BEEP = 1;//鸣叫接入成功
	
	while(DHT11_Init())	//DHT11初始化	
	{
		LCD_ShowString(30,110,200,16,16,"DHT11 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}								   
	LCD_ShowString(30,110,200,16,16,"DHT11 OK");
	POINT_COLOR=BLUE;//设置字体为蓝色 
 	LCD_ShowString(30,130,200,16,16,"Temp:  C");	 
 	LCD_ShowString(30,150,200,16,16,"Humi:  %");
	delay_ms(250);   
	BEEP = 0;	
	OneNet_Subscribe(devSubTopic, 1);
  
	while(1) 
	{	
		if(timeCount % 40 == 0)				//1s反转一次
		{
			LED0=!LED0;			

			DHT11_Read_Data(&temperature,&humidity);	//读取温湿度值					    
			LCD_ShowNum(30+40,130,temperature,2,16);	//显示温度	   		   
			LCD_ShowNum(30+40,150,humidity,2,16);		//显示湿度
			if (LED1 == 0)	
				LCD_ShowString(30,170,200,16,16,"LED1: ON ");
			else
				LCD_ShowString(30,170,200,16,16,"LED1: OFF");
			printf("temp: %d      humi: %d\r\n", temperature, humidity);	//串口打印
		}
		if(++timeCount >= 200)									//发送间隔5s
		{
			UsartPrintf(USART_DEBUG, " OneNet_Publish\r\n");    //会吞掉第一个字符
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
	
