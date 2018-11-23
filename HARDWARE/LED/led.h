#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
						  
////////////////////////////////////////////////////////////////////////////////// 	

//LED端口定义		PA6 PA7为控制板LED PC2 PC3为扩展板LED
#define LED0 PAout(6)			// 控制板LED
#define LED1 PAout(7)			// 控制板LED
#define	LED2 PBout(8)			// 扩展板LED
#define LED3 PBout(9)			// 扩展板LED

void LED_Init(void);  //初始化
#endif
