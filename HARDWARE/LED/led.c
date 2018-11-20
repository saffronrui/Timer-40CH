#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD ,ENABLE); //ʹ��GPIOD��ʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;			//	PA6 PA7Ϊ���ư�LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //�������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //����GPIO
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7); //GPIOF �ߵ�ƽ

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3;			//	PC2 PC3Ϊ��չ��LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //�������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //����GPIO
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3); //GPIOF �ߵ�ƽ
	
	//PA0-PA5Ϊ����ͨ��
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;			//	PA6 PA7Ϊ���ư�LED
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  //GPIOD��ʼ�����ã� PD0-PD7Ϊ8·����ͨ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOD, &GPIO_InitStructure);											//��ʼ��GPIOD

}
