#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//						  
////////////////////////////////////////////////////////////////////////////////// 	

//初始化PF9和PF10为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE,       ENABLE); //使能GPIOD的时钟
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;			//	PB8 PB9为扩展板LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉输出
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //高速GPIO
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9); //GPIOB 高电平
	
	//PA0-PA5为输入通道
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5 |GPIO_Pin_6|GPIO_Pin_7;			
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  //GPIOD初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOD, &GPIO_InitStructure);											//初始化GPIOD

  //GPIOE初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOE, &GPIO_InitStructure);											//初始化GPIOD

}
