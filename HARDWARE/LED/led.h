#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
						  
////////////////////////////////////////////////////////////////////////////////// 	

//LED�˿ڶ���		PA6 PA7Ϊ���ư�LED PC2 PC3Ϊ��չ��LED
#define LED0 PAout(6)			// ���ư�LED
#define LED1 PAout(7)			// ���ư�LED
#define	LED2 PBout(8)			// ��չ��LED
#define LED3 PBout(9)			// ��չ��LED

void LED_Init(void);  //��ʼ��
#endif
