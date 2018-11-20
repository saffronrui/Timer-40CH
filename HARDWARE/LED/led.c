#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//初始化PF9和PF10为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD ,ENABLE); //使能GPIOD的时钟
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;			//	PA6 PA7为控制板LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //高速GPIO
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7); //GPIOF 高电平

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3;			//	PC2 PC3为扩展板LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //高速GPIO
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3); //GPIOF 高电平
	
	//PA0-PA5为输入通道
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;			//	PA6 PA7为控制板LED
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  //GPIOD初始化设置， PD0-PD7为8路输入通道
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOD, &GPIO_InitStructure);											//初始化GPIOD

}
