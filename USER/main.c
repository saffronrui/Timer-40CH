#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
#include "hmi_app.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "hmi_user_uart.h"

int16_t test_value;
//PTIME_INFO	*ch_time; 
TIME_INFO		ch_time;
long task2_num=0;

uint8 cmd_buffer[CMD_MAX_SIZE];							 //指令缓存

void time_info_init(void);

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define TASK1_TASK_PRIO		4
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务控制块
OS_TCB Task1_TaskTCB;
//任务堆栈	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);

//任务优先级
#define TASK100MS_TASK_PRIO		6
//任务堆栈大小	
#define TASK100MS_STK_SIZE 		128
//任务控制块
OS_TCB Task100ms_TaskTCB;
//任务堆栈	
CPU_STK TASK100MS_TASK_STK[TASK100MS_STK_SIZE];
//任务函数
void task100ms_task(void *p_arg);

//任务优先级
#define TASK1000MS_TASK_PRIO		7
//任务堆栈大小	
#define TASK1000MS_STK_SIZE 		128
//任务控制块
OS_TCB Task1000ms_TaskTCB;
//任务堆栈	
CPU_STK TASK1000MS_TASK_STK[TASK100MS_STK_SIZE];
//任务函数
void task1000ms_task(void *p_arg);

//任务优先级
#define TASK2_TASK_PRIO		9
//任务堆栈大小	
#define TASK2_STK_SIZE 		128
//任务控制块
OS_TCB Task2_TaskTCB;
//任务堆栈	
CPU_STK TASK2_TASK_STK[TASK2_STK_SIZE];
//任务函数
void task2_task(void *p_arg);


//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init(168);  //时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);   //串口初始化
	LED_Init();         //LED初始化	
	time_info_init();
	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}


//开始任务任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//进入临界区
	//创建TASK1任务
	OSTaskCreate((OS_TCB 	* )&Task1_TaskTCB,		
				 (CPU_CHAR	* )"Task1 task", 		
                 (OS_TASK_PTR )task1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1_TASK_PRIO,     
                 (CPU_STK   * )&TASK1_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK1_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建TASK2任务
	OSTaskCreate((OS_TCB 	* )&Task2_TaskTCB,		
				 (CPU_CHAR	* )"task2 task", 		
                 (OS_TASK_PTR )task2_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK2_TASK_PRIO,     	
                 (CPU_STK   * )&TASK2_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK2_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK2_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);			 
								 
		//创建100ms计时任务
	OSTaskCreate((OS_TCB 	* )&Task100ms_TaskTCB,		
				 (CPU_CHAR	* )"task100ms task", 		
                 (OS_TASK_PTR )task100ms_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK100MS_TASK_PRIO,     	
                 (CPU_STK   * )&TASK100MS_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK100MS_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK100MS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
								 
		//创建1000ms计时任务
	OSTaskCreate((OS_TCB 	* )&Task1000ms_TaskTCB,		
				 (CPU_CHAR	* )"task1000ms task", 		
                 (OS_TASK_PTR )task1000ms_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1000MS_TASK_PRIO,     	
                 (CPU_STK   * )&TASK1000MS_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK1000MS_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1000MS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
	
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}


void time_info_init(void)
{
		int8_t	temp;
	
//		ch_time -> cnt_1000ms = 0;
//		ch_time -> cnt_100ms	 = 0;
//		for( temp = 0; temp < CH_MAX;	temp++ )
//		{
//				ch_time -> ch_sta[temp] = CH_OFF;
//				ch_time -> ch_on_time[temp] = 0.0;
//		}
}

/*
//	GPIO 读取函数
//	gpio_sta_read() function
//
*/
void gpio_sta_read(void)
{
	uint8_t		i;
	uint16_t	gpiod_data, gpioe_data, gpioc_data;
	uint16_t	temp;
	uchar ch_tim_buf[12] = {0} ;
	
	ch_time.ch_sta[0]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
	ch_time.ch_sta[1]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
	ch_time.ch_sta[2]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
	ch_time.ch_sta[3]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
	ch_time.ch_sta[4]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4);
	ch_time.ch_sta[5]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
	ch_time.ch_sta[6]  =  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
	ch_time.ch_sta[7]  =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0);
	ch_time.ch_sta[8]  =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1);
	ch_time.ch_sta[9]  =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2);
	ch_time.ch_sta[10] =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);
	ch_time.ch_sta[11] =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4);
	ch_time.ch_sta[12] =  GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_5);
//	gpioe_data = GPIO_ReadInputData(GPIOE);
//	gpioc_data = GPIO_ReadInputData(GPIOC) & 0x0f;
	
	
	
	for( i = 0; i < 10; i++ )								
	{	
			temp = (gpiod_data >> i) & 0x01;
			if( (temp == 0) && (ch_time.ch_sta[i]) )
					ch_time.ch_on_time[i] = (ch_time.cnt_1000ms );
			if( (temp > 0) && (!ch_time.ch_sta[i]) )
			{
					GUI_RectangleFill(200+ch_time.ch_on_time[i]*9.17, 109+i*30, 220+ch_time.cnt_1000ms*9.17,125+i*30);
					sprintf(ch_tim_buf, "%.01f", (ch_time.cnt_1000ms*10 + ch_time.cnt_100ms) / 10.0);
					SetFcolor(65504); 	//设置字体颜色，通过设置前景色实现
					DisText(150+ch_time.ch_on_time[i]*9.17,109+i*30,0x00,0x02,ch_tim_buf );
					SetFcolor(63488);		//恢复红色前景色
			}
			ch_time.ch_sta[i] = temp;
			
			
//			if( ((gpiod_data >> i) & 0x01) && (!ch_time.ch_sta[i]) )
//			{
//					ch_time.ch_on_time[i] = (ch_time.cnt_1000ms + ch_time.cnt_100ms);
//					ch_time.ch_sta[i] = CH_ON;
//			}
	}	

}

//task1任务函数
void task1_task(void *p_arg)
{
	u8 task1_num=0;
	u8 size = 0;
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		LED0 = ~LED0;
		LED2 = ~LED2;
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE);
		if(  size > 0 )												//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);				//指令处理
		}
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
		
	}
}

//task2任务函数
void task2_task(void *p_arg)
{

	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		LED1 = ~LED1;
		LED3 = ~LED3;
		gpio_sta_read();
		UpdateUI( &ch_time );
//		SetTextValueInt32( 0, 65, ch_time.cnt_1000ms );
		OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
	}
}


void task100ms_task(void *p_arg)			// 0.1s计时
{
		OS_ERR err;
		CPU_SR_ALLOC();
		p_arg = p_arg;	
		
		while(1)
		{
				ch_time.cnt_100ms++;
				OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);	 //延时100ms
		}
}

void task1000ms_task(void *p_arg)			//	1s计时
{
		OS_ERR err;
		CPU_SR_ALLOC();
		p_arg = p_arg;	
		
		while(1)
		{
				ch_time.cnt_1000ms ++;
				test_value ++;
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);	 //延时1000ms
		}
}



