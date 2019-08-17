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

#define	CH_NUM	30

TIME_INFO		ch_time;
IO_INFO	Port_Information[CH_NUM];

uint8_t	CMD_VAL;

long task2_num=0;

uint8 cmd_buffer[CMD_MAX_SIZE];							 //指令缓存

void time_info_init(void);					//时间基准初始化函数
void	wait_start_signal(void);			//等待开始信号函数，阻塞函数，只有输出为高电平才能执行此句，否则一直在此处循环

void wait_start_signal(void)				//等待开始信号函数
{
		while(!((GPIO_ReadInputData(GPIOD) >> 9) & 0x0001));
}


//*****************************************************//
//任务优先级排序：
//start_task >> 100ms_task >> 100ms_task >> task1(touch cmd task) >> task2(gui_refresh task)
//******************************************************//

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
#define TASK1_TASK_PRIO		8
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务控制块
OS_TCB Task1_TaskTCB;
//任务堆栈	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);

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
	time_info_init();			//结构体参数初始化
	delay_ms(100);				//延时100ms等待触屏初始化
	SetBuzzer(0x3A);			//上电提醒
	
//	wait_start_signal();	//等待时间基准信号,通道8输入信号为开始信号	
	
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

		CMD_VAL = 0;																// 清除命令状态字

		ch_time.cnt_1000ms = 0;											// 时间基准清零
		ch_time.cnt_100ms = 0;											// 时间基准清零
	
		for( temp = 0; temp < CH_NUM; temp++ ){					// PAGE0/PAGE1通用数据初始化
				 
			Port_Information[temp].io_last_sta    = false;
			Port_Information[temp].io_sta			    = false;
			Port_Information[temp].on_time_stamp  = 0.0;
			Port_Information[temp].off_time_stamp = 0.0;
		}
		
		Port_Information[0].page_ID 		= 0;				//PORT0 ID-Number Init, PAGE0
		Port_Information[0].name_ID 		= 6;
		Port_Information[0].on_text_ID 	= 60;
		Port_Information[0].off_text_ID = 61;
		
		Port_Information[1].page_ID 		= 0;				//PORT1 ID-Number Init, PAGE0
		Port_Information[1].name_ID 		= 9;
		Port_Information[1].on_text_ID 	= 62;
		Port_Information[1].off_text_ID = 63;
		
		Port_Information[2].page_ID 		= 0;				//PORT2 ID-Number Init, PAGE0
		Port_Information[2].name_ID 		= 10;
		Port_Information[2].on_text_ID 	= 64;
		Port_Information[2].off_text_ID = 65;
		
		Port_Information[3].page_ID 		= 0;				//PORT3 ID-Number Init, PAGE0
		Port_Information[3].name_ID 		= 11;
		Port_Information[3].on_text_ID 	= 66;
		Port_Information[3].off_text_ID = 67;

		Port_Information[4].page_ID 		= 0;				//PORT4 ID-Number Init, PAGE0
		Port_Information[4].name_ID 		= 12;
		Port_Information[4].on_text_ID 	= 68;
		Port_Information[4].off_text_ID = 69;
		
		Port_Information[5].page_ID 		= 0;				//PORT5 ID-Number Init, PAGE0
		Port_Information[5].name_ID 		= 13;
		Port_Information[5].on_text_ID 	= 70;
		Port_Information[5].off_text_ID = 71;
		
		Port_Information[6].page_ID 		= 0;				//PORT6 ID-Number Init, PAGE0
		Port_Information[6].name_ID 		= 14;
		Port_Information[6].on_text_ID 	= 72;
		Port_Information[6].off_text_ID = 73;
		
		Port_Information[7].page_ID 		= 0;				//PORT7 ID-Number Init, PAGE0
		Port_Information[7].name_ID 		= 15;
		Port_Information[7].on_text_ID 	= 74;
		Port_Information[7].off_text_ID = 75;
		
		Port_Information[8].page_ID 		= 0;				//PORT8 ID-Number Init, PAGE0
		Port_Information[8].name_ID 		= 16;
		Port_Information[8].on_text_ID 	= 76;
		Port_Information[8].off_text_ID = 77;
		
		Port_Information[9].page_ID 		= 0;				//PORT9 ID-Number Init, PAGE0
		Port_Information[9].name_ID 		= 17;
		Port_Information[9].on_text_ID 	= 78;
		Port_Information[9].off_text_ID = 79;
		
		Port_Information[10].page_ID 			= 0;				//PORT10 ID-Number Init, PAGE0
		Port_Information[10].name_ID 			= 18;
		Port_Information[10].on_text_ID 	= 80;
		Port_Information[10].off_text_ID 	= 81;
		
		Port_Information[11].page_ID 			= 0;				//PORT11 ID-Number Init, PAGE0
		Port_Information[11].name_ID 			= 19;
		Port_Information[11].on_text_ID 	= 82;
		Port_Information[11].off_text_ID 	= 83;

		Port_Information[12].page_ID 			= 0;				//PORT12 ID-Number Init, PAGE0
		Port_Information[12].name_ID 			= 20;
		Port_Information[12].on_text_ID 	= 84;
		Port_Information[12].off_text_ID 	= 85;
		
		Port_Information[13].page_ID 			= 0;				//PORT13 ID-Number Init, PAGE0
		Port_Information[13].name_ID 			= 21;
		Port_Information[13].on_text_ID 	= 86;
		Port_Information[13].off_text_ID 	= 87;
		
		Port_Information[14].page_ID 			= 0;				//PORT14 ID-Number Init, PAGE0
		Port_Information[14].name_ID 			= 22;
		Port_Information[14].on_text_ID 	= 88;
		Port_Information[14].off_text_ID 	= 89;
		
		Port_Information[15].page_ID 			= 0;				//PORT15 ID-Number Init, PAGE0
		Port_Information[15].name_ID 			= 23;
		Port_Information[15].on_text_ID 	= 90;
		Port_Information[15].off_text_ID 	= 91;

		Port_Information[16].page_ID 			= 0;				//PORT16 ID-Number Init, PAGE0
		Port_Information[16].name_ID 			= 24;
		Port_Information[16].on_text_ID 	= 92;
		Port_Information[16].off_text_ID 	= 93;
		
		Port_Information[17].page_ID 			= 0;				//PORT17 ID-Number Init, PAGE0
		Port_Information[17].name_ID 			= 25;
		Port_Information[17].on_text_ID 	= 94;
		Port_Information[17].off_text_ID 	= 95;
		
		Port_Information[18].page_ID 			= 0;				//PORT18 ID-Number Init, PAGE0
		Port_Information[18].name_ID 			= 26;
		Port_Information[18].on_text_ID 	= 96;
		Port_Information[18].off_text_ID 	= 97;

		Port_Information[19].page_ID 			= 0;				//PORT19 ID-Number Init, PAGE0
		Port_Information[19].name_ID 			= 27;
		Port_Information[19].on_text_ID 	= 98;
		Port_Information[19].off_text_ID 	= 99;
		
		Port_Information[20].page_ID 			= 0;				//PORT20 ID-Number Init, PAGE0
		Port_Information[20].name_ID 			= 28;
		Port_Information[20].on_text_ID 	= 100;
		Port_Information[20].off_text_ID 	= 101;
		
		Port_Information[21].page_ID 			= 0;				//PORT21 ID-Number Init, PAGE0
		Port_Information[21].name_ID 			= 29;
		Port_Information[21].on_text_ID 	= 102;
		Port_Information[21].off_text_ID 	= 103;
		
		Port_Information[22].page_ID 			= 1;				//PORT22 ID-Number Init, PAGE1
		Port_Information[22].name_ID 			= 6;
		Port_Information[22].on_text_ID 	= 60;
		Port_Information[22].off_text_ID 	= 61;
		
		Port_Information[23].page_ID 			= 1;				//PORT23 ID-Number Init, PAGE1
		Port_Information[23].name_ID 			= 9;
		Port_Information[23].on_text_ID 	= 62;
		Port_Information[23].off_text_ID 	= 63;
		
		Port_Information[24].page_ID 			= 1;				//PORT24 ID-Number Init, PAGE1
		Port_Information[24].name_ID 			= 10;
		Port_Information[24].on_text_ID 	= 64;
		Port_Information[24].off_text_ID 	= 65;
		
		Port_Information[25].page_ID 			= 1;				//PORT25 ID-Number Init, PAGE1
		Port_Information[25].name_ID 			= 11;
		Port_Information[25].on_text_ID 	= 66;
		Port_Information[25].off_text_ID 	= 67;
		
//		Port_Information[26].page_ID 			= 1;				//PORT26 ID-Number Init, PAGE1
//		Port_Information[26].name_ID 			= 12;
//		Port_Information[26].on_text_ID 	= 68;
//		Port_Information[26].off_text_ID 	= 69;
//		
//		Port_Information[27].page_ID 			= 1;				//PORT27 ID-Number Init, PAGE1
//		Port_Information[27].name_ID 			= 3;
//		Port_Information[27].on_text_ID 	= 70;
//		Port_Information[27].off_text_ID 	= 71;
//		
//		Port_Information[28].page_ID 			= 1;				//PORT28 ID-Number Init, PAGE1
//		Port_Information[28].name_ID 			= 4;
//		Port_Information[28].on_text_ID 	= 72;
//		Port_Information[28].off_text_ID 	= 73;
//		
//		Port_Information[29].page_ID 			= 1;				//PORT29 ID-Number Init, PAGE1
//		Port_Information[29].name_ID 			= 5;
//		Port_Information[29].on_text_ID 	= 74;
//		Port_Information[29].off_text_ID 	= 75;

//		Port_Information[30].page_ID 			= 1;				//PORT30 ID-Number Init, PAGE1
//		Port_Information[30].name_ID 			= 13;
//		Port_Information[30].on_text_ID 	= 76;
//		Port_Information[30].off_text_ID 	= 77;
		
		Port_Information[26].page_ID 			= 1;				//PORT26 ID-Number Init, PAGE1
		Port_Information[26].name_ID 			= 14;
		Port_Information[26].on_text_ID 	= 78;
		Port_Information[26].off_text_ID 	= 79;
		
		Port_Information[27].page_ID 			= 1;				//PORT27 ID-Number Init, PAGE1
		Port_Information[27].name_ID 			= 16;
		Port_Information[27].on_text_ID 	= 80;
		Port_Information[27].off_text_ID 	= 81;
		
		Port_Information[28].page_ID 			= 1;				//PORT28 ID-Number Init, PAGE1
		Port_Information[28].name_ID 			= 17;
		Port_Information[28].on_text_ID 	= 82;
		Port_Information[28].off_text_ID 	= 83;
		
		Port_Information[29].page_ID 			= 1;				//PORT29 ID-Number Init, PAGE1
		Port_Information[29].name_ID 			= 18;
		Port_Information[29].on_text_ID 	= 84;
		Port_Information[29].off_text_ID 	= 85;
		
//		Port_Information[35].page_ID 			= 1;				//PORT35 ID-Number Init, PAGE1
//		Port_Information[35].name_ID 			= 19;
//		Port_Information[35].on_text_ID 	= 86;
//		Port_Information[35].off_text_ID 	= 87;
//		
//		Port_Information[36].page_ID 			= 1;				//PORT36 ID-Number Init, PAGE1
//		Port_Information[36].name_ID 			= 20;
//		Port_Information[36].on_text_ID 	= 88;
//		Port_Information[36].off_text_ID 	= 89;
//		
//		Port_Information[37].page_ID 			= 1;				//PORT37 ID-Number Init, PAGE1
//		Port_Information[37].name_ID 			= 21;
//		Port_Information[37].on_text_ID 	= 90;
//		Port_Information[37].off_text_ID 	= 91;

//		Port_Information[38].page_ID 			= 1;				//PORT38 ID-Number Init, PAGE1
//		Port_Information[38].name_ID 			= 22;
//		Port_Information[38].on_text_ID 	= 92;
//		Port_Information[38].off_text_ID 	= 93;
//		
//		Port_Information[39].page_ID 			= 1;				//PORT39 ID-Number Init, PAGE1
//		Port_Information[39].name_ID 			= 23;
//		Port_Information[39].on_text_ID 	= 94;
//		Port_Information[39].off_text_ID 	= 95;
}

/*
//	GPIO 读取函数
//	gpio_sta_read() function
//
*/
void gpio_sta_read(void)
{
	uint16_t	gpio_data_E, gpio_data_A, gpio_data_D;
	
	gpio_data_A = GPIO_ReadInputData(GPIOA);				//读取 GPIOA 8位
	gpio_data_D = GPIO_ReadInputData(GPIOD);				//读取 GPIOD 16位
	gpio_data_E = GPIO_ReadInputData(GPIOE);				//读取 GPIOE 16位

//输入通道需要重新排列	
//	Port_Information[0].io_sta  =  (gpio_data_D >> 7) & 0x0001;				//通道0输入端口
//	Port_Information[1].io_sta  =  (gpio_data_D >> 5) & 0x0001;				//通道1输入端口
//	Port_Information[2].io_sta  =  (gpio_data_D >> 3) & 0x0001;				//通道2输入端口
//	Port_Information[3].io_sta  =  (gpio_data_D >> 1) & 0x0001;				//通道3输入端口
	Port_Information[0].io_sta  =  (gpio_data_D >> 15)& 0x0001;				//通道4输入端口
	Port_Information[1].io_sta  =  (gpio_data_D >> 13)& 0x0001;				//通道5输入端口
	Port_Information[2].io_sta  =  (gpio_data_D >> 11)& 0x0001;				//通道6输入端口
	Port_Information[3].io_sta  =  (gpio_data_D >> 9) & 0x0001;				//通道7输入端口
	Port_Information[4].io_sta  =  (gpio_data_E >> 7) & 0x0001;				//通道8输入端口
	Port_Information[5].io_sta  =  (gpio_data_E >> 4) & 0x0001;				//通道9输入端口
	Port_Information[6].io_sta = (gpio_data_E >> 3)  & 0x0001;				//通道10输入端口
	Port_Information[7].io_sta = (gpio_data_A >> 6)  & 0x0001;				//通道11输入端口
	Port_Information[8].io_sta = (gpio_data_A >> 5)  & 0x0001;				//通道12输入端口	
	Port_Information[9].io_sta = (gpio_data_A)  			& 0x0001;				//通道13输入端口	
	Port_Information[10].io_sta = (gpio_data_E >> 14) & 0x0001;			  //通道14输入端口
	Port_Information[11].io_sta = (gpio_data_E >> 12) & 0x0001;			  //通道15输入端口
	Port_Information[12].io_sta = (gpio_data_E >> 10) & 0x0001;				//通道16输入端口	
//	Port_Information[17].io_sta = (gpio_data_D >> 6)  & 0x0001;				//通道17输入端口
//	Port_Information[18].io_sta = (gpio_data_D >> 4)  & 0x0001;				//通道18输入端口	
//	Port_Information[19].io_sta = (gpio_data_D >> 2)  & 0x0001;				//通道19输入端口
//	Port_Information[20].io_sta = (gpio_data_D)  			& 0x0001;				//通道20输入端口
	Port_Information[13].io_sta = (gpio_data_D >> 14) & 0x0001;				//通道21输入端口
	Port_Information[14].io_sta = (gpio_data_D >> 12) & 0x0001;				//通道22输入端口
	Port_Information[15].io_sta = (gpio_data_D >> 10) & 0x0001;				//通道23输入端口	
	Port_Information[16].io_sta = (gpio_data_D >> 8)  & 0x0001;				//通道24输入端口	
	Port_Information[17].io_sta = (gpio_data_E >> 6)  & 0x0001;				//通道25输入端口
	Port_Information[18].io_sta = (gpio_data_E >> 5)  & 0x0001;				//通道26输入端口
	Port_Information[19].io_sta = (gpio_data_E)       & 0x0001;				//通道27输入端口
	Port_Information[20].io_sta = (gpio_data_A >> 7)  & 0x0001;				//通道28输入端口
	Port_Information[21].io_sta = (gpio_data_A >> 2)  & 0x0001;				//通道29输入端口
	Port_Information[22].io_sta = (gpio_data_A >> 1)  & 0x0001;				//通道30输入端口
	Port_Information[23].io_sta = (gpio_data_E >> 13) & 0x0001;				//通道31输入端口	
	Port_Information[24].io_sta = (gpio_data_E >> 9)  & 0x0001;				//通道32输入端口
	Port_Information[25].io_sta = (gpio_data_E >> 2)  & 0x0001;				//通道33输入端口
	Port_Information[26].io_sta = (gpio_data_E >> 1)  & 0x0001;				//通道34输入端口
	Port_Information[27].io_sta = (gpio_data_A >> 4)  & 0x0001;				//通道35输入端口	
	Port_Information[28].io_sta = (gpio_data_A >> 3)  & 0x0001;				//通道36输入端口	
	Port_Information[29].io_sta = (gpio_data_E >> 15) & 0x0001;				//通道37输入端口
//	Port_Information[30].io_sta = (gpio_data_E >> 11) & 0x0001;				//通道38输入端口	
//	Port_Information[31].io_sta = (gpio_data_E >> 8)  & 0x0001;				//通道39输入端口	

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
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE);

		if(  size > 0 )												//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);				//指令处理
		}
		
		switch(CMD_VAL){	
					
			case	0x1a:								//重置命令BUZZER提醒
				OSTaskSuspend((OS_TCB*)&Task2_TaskTCB, &err);							//重置模式下挂起其他任务，停止计时
				OSTaskSuspend((OS_TCB*)&Task100ms_TaskTCB, &err);
				OSTaskSuspend((OS_TCB*)&Task1000ms_TaskTCB, &err);
				
				SetBuzzer(0x3A);				//重置提醒
				time_info_init();				//初始化所有参数
				Clear_GUI(Port_Information,	CH_NUM, ch_time.cnt_100ms);		 		//清空屏幕
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				
				wait_start_signal();																			//阻塞等待开始信号
				
				OSTaskResume((OS_TCB*)&Task2_TaskTCB, &err);							//等待到开始信号之后恢复任务，开始下次计时
				OSTaskResume((OS_TCB*)&Task100ms_TaskTCB, &err);
				OSTaskResume((OS_TCB*)&Task1000ms_TaskTCB, &err);			
				break;

			case	0x2a:									//自检命令
				OSTaskSuspend((OS_TCB*)&Task2_TaskTCB, &err);							//自检模式下挂起其他任务，停止计时
				OSTaskSuspend((OS_TCB*)&Task100ms_TaskTCB, &err);
				OSTaskSuspend((OS_TCB*)&Task1000ms_TaskTCB, &err);

				SetBuzzer(0x3A);					//自检命令开始BUZZER提醒
				time_info_init();					//初始化所有参数
				Clear_GUI(Port_Information,	CH_NUM, ch_time.cnt_100ms);		 		//清空屏幕
				Device_Check(Port_Information,	CH_NUM);
				SetBuzzer(0x3A);					//自检命令结束BUZZER提醒
				CMD_VAL = 0;																							//清除本次命令，防止重复执行				
				
				wait_start_signal();																			//阻塞等待开始信号				
				
				OSTaskResume((OS_TCB*)&Task2_TaskTCB, &err);							//等待到开始信号之后恢复任务，开始下次计时
				OSTaskResume((OS_TCB*)&Task100ms_TaskTCB, &err);
				OSTaskResume((OS_TCB*)&Task1000ms_TaskTCB, &err);
				break;
			
			case	0xB1:								//继电器1闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_0); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC1:								//继电器1断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_0); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;			
			case	0xB2:								//继电器2闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_1); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC2:								//继电器2断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_1); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;					
			case	0xB3:								//继电器3闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_2); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC3:								//继电器3断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_2); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;			
			case	0xB4:								//继电器4闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_3); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC4:								//继电器4断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_3); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;					
			case	0xB5:								//继电器5闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_4); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC5:								//继电器5断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_4); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;			
			case	0xB6:								//继电器6闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_5); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC6:								//继电器6断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_5); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;					
			case	0xB7:								//继电器7闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_6); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC7:								//继电器7断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_6); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;			
			case	0xB8:								//继电器8闭合指令
				GPIO_SetBits(GPIOD,GPIO_Pin_7); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;
			case	0xC8:								//继电器8断开指令
				GPIO_ResetBits(GPIOD,GPIO_Pin_7); 
				CMD_VAL = 0;																							//清除本次命令，防止重复执行
				break;

			
			default:
				break;
		}
		
		OSTimeDlyHMSM(0,0,0,550,OS_OPT_TIME_HMSM_STRICT,&err); //延时 500ms
		
	}
}

//task2任务函数，刷新屏幕任务
void task2_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		LED2 = ~LED2;
//		LED3 = ~LED3;
		
		gpio_sta_read();
				
		Fresh_GUI(Port_Information,	CH_NUM, ch_time.cnt_100ms);		 		//刷新屏幕
		
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); 		//延时200ms
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
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);	 //延时1000ms
		}
}



