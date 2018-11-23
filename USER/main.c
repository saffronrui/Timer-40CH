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

uint8 ON_TEXT_ID[40]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
												 11,12,13,14,15,16,17,18,19,20,
												 21,22,23,24,25,26,27,28,29,30,
												 31,32,33,34,35,36,37,38,39,40};

uint8 OFF_TEXT_ID[40] = {41,42,43,44,45,46,47,48,49,50,
												 51,52,53,54,55,56,57,58,59,60,
												 61,62,63,64,65,66,67,68,69,70,
												 71,72,73,74,75,76,77,78,79,80};

IO_INFO	Port_Information[40];

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
	
	delay_init(168); 		  //时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);    //串口初始化
	LED_Init();           //LED初始化	
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

//**********************************************//
//端口信息初始化
//输入： void
//返回： void
//**********************************************//
void time_info_init(void)										
{
		int8_t	temp;

		for( temp = 0; temp < 40; temp++ ){
				 
			Port_Information[temp].io_last_sta    = false;
			Port_Information[temp].io_sta			    = false;
			Port_Information[temp].on_time_stamp  = 0.0;
			Port_Information[temp].off_time_stamp = 0.0;
			Port_Information[temp].on_text_ID  = ON_TEXT_ID[temp];					//屏幕导通ID初始化
			Port_Information[temp].off_text_ID = OFF_TEXT_ID[temp];					//屏幕关断ID初始化
		}
}

/*
//	GPIO 读取函数
//	gpio_sta_read() function
//
*/
void gpio_sta_read(void)
{
	uint8_t		i;
	uint16_t	gpio_data_E, gpio_data_A, gpio_data_D, gpio_data;
	uint16_t	temp;
	uchar ch_tim_buf[12] = {0} ;
	
	gpio_data_A = GPIO_ReadInputData(GPIOA);	
	gpio_data_D = GPIO_ReadInputData(GPIOD);
	gpio_data_E = GPIO_ReadInputData(GPIOE);	
	
	Port_Information[0].io_sta =  (gpio_data_D >> 7)  & 0x0001;				//通道0输入端口
	Port_Information[1].io_sta =  (gpio_data_D >> 5)  & 0x0001;				//通道1输入端口
	Port_Information[2].io_sta =  (gpio_data_D >> 3)  & 0x0001;				//通道2输入端口
	Port_Information[3].io_sta =  (gpio_data_D >> 1)  & 0x0001;				//通道3输入端口
	Port_Information[4].io_sta =  (gpio_data_D >> 15) & 0x0001;				//通道4输入端口
	Port_Information[5].io_sta =  (gpio_data_D >> 13) & 0x0001;				//通道5输入端口
	Port_Information[6].io_sta =  (gpio_data_D >> 11) & 0x0001;				//通道6输入端口
	Port_Information[7].io_sta =  (gpio_data_D >> 9)  & 0x0001;				//通道7输入端口
	Port_Information[8].io_sta =  (gpio_data_E >> 7)  & 0x0001;				//通道8输入端口
	Port_Information[9].io_sta =  (gpio_data_E >> 4)  & 0x0001;				//通道9输入端口
	Port_Information[10].io_sta = (gpio_data_E >> 3)  & 0x0001;				//通道10输入端口
	Port_Information[11].io_sta = (gpio_data_A >> 6)  & 0x0001;				//通道11输入端口
	Port_Information[12].io_sta = (gpio_data_A >> 5)  & 0x0001;				//通道12输入端口	
	Port_Information[13].io_sta = (gpio_data_A)  			& 0x0001;				//通道13输入端口	
	Port_Information[14].io_sta = (gpio_data_E >> 14) & 0x0001;			  //通道14输入端口
	Port_Information[15].io_sta = (gpio_data_E >> 12) & 0x0001;			  //通道15输入端口
	Port_Information[16].io_sta = (gpio_data_E >> 10) & 0x0001;				//通道16输入端口	
	Port_Information[17].io_sta = (gpio_data_D >> 6)  & 0x0001;				//通道17输入端口
	Port_Information[18].io_sta = (gpio_data_D >> 4)  & 0x0001;				//通道18输入端口	
	Port_Information[19].io_sta = (gpio_data_D >> 2)  & 0x0001;				//通道19输入端口
	Port_Information[20].io_sta = (gpio_data_D)  			& 0x0001;				//通道20输入端口
	Port_Information[21].io_sta = (gpio_data_D >> 14) & 0x0001;				//通道21输入端口
	Port_Information[22].io_sta = (gpio_data_D >> 12) & 0x0001;				//通道22输入端口
	Port_Information[23].io_sta = (gpio_data_D >> 10) & 0x0001;				//通道23输入端口	
	Port_Information[24].io_sta = (gpio_data_D >> 8)  & 0x0001;				//通道24输入端口	
	Port_Information[25].io_sta = (gpio_data_E >> 6)  & 0x0001;				//通道25输入端口
	Port_Information[26].io_sta = (gpio_data_E >> 5)  & 0x0001;				//通道26输入端口
	Port_Information[27].io_sta = (gpio_data_E)       & 0x0001;				//通道27输入端口
	Port_Information[28].io_sta = (gpio_data_A >> 7)  & 0x0001;				//通道28输入端口
	Port_Information[29].io_sta = (gpio_data_A >> 2)  & 0x0001;				//通道29输入端口
	Port_Information[30].io_sta = (gpio_data_A >> 1)  & 0x0001;				//通道30输入端口
	Port_Information[31].io_sta = (gpio_data_E >> 13) & 0x0001;				//通道31输入端口	
	Port_Information[32].io_sta = (gpio_data_E >> 9)  & 0x0001;				//通道32输入端口
	Port_Information[33].io_sta = (gpio_data_E >> 2)  & 0x0001;				//通道33输入端口
	Port_Information[34].io_sta = (gpio_data_E >> 1)  & 0x0001;				//通道34输入端口
	Port_Information[35].io_sta = (gpio_data_A >> 4)  & 0x0001;				//通道35输入端口	
	Port_Information[36].io_sta = (gpio_data_A >> 3)  & 0x0001;				//通道36输入端口	
	Port_Information[37].io_sta = (gpio_data_E >> 15) & 0x0001;				//通道37输入端口
	Port_Information[38].io_sta = (gpio_data_E >> 11) & 0x0001;				//通道38输入端口	
	Port_Information[39].io_sta = (gpio_data_E >> 8)  & 0x0001;				//通道39输入端口	

/*
	
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
			
	}	
*/

}

//*********************************************//
//touchpad CMD_handler,触摸屏命令处理任务//
//*********************************************//
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
//		UpdateUI( &ch_time );
		Fresh_GUI(Port_Information, 40);
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



