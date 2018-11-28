#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//						  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE,       ENABLE); //ʹ��GPIOD��ʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;			//	PB8 PB9Ϊ��չ��LED
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //�������
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz; //����GPIO
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9); //GPIOB �ߵ�ƽ
	
	//PA0-PA5Ϊ����ͨ��
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5 |GPIO_Pin_6|GPIO_Pin_7;			
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  //GPIOD��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOD, &GPIO_InitStructure);											//��ʼ��GPIOD

  //GPIOE��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;								//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;					  //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;				  
	GPIO_Init(GPIOE, &GPIO_InitStructure);											//��ʼ��GPIOD

}
