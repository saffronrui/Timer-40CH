#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//LED�˿ڶ���		PA6 PA7Ϊ���ư�LED PC2 PC3Ϊ��չ��LED
#define LED0 PAout(6)			// ���ư�LED
#define LED1 PAout(7)			// ���ư�LED
#define	LED2 PCout(2)			// ��չ��LED
#define LED3 PCout(3)			// ��չ��LED

void LED_Init(void);  //��ʼ��
#endif
