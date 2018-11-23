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

uint8 cmd_buffer[CMD_MAX_SIZE];							 //ָ���

void time_info_init(void);

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		128
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define TASK1_TASK_PRIO		4
//�����ջ��С	
#define TASK1_STK_SIZE 		128
//������ƿ�
OS_TCB Task1_TaskTCB;
//�����ջ	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);

//�������ȼ�
#define TASK100MS_TASK_PRIO		6
//�����ջ��С	
#define TASK100MS_STK_SIZE 		128
//������ƿ�
OS_TCB Task100ms_TaskTCB;
//�����ջ	
CPU_STK TASK100MS_TASK_STK[TASK100MS_STK_SIZE];
//������
void task100ms_task(void *p_arg);

//�������ȼ�
#define TASK1000MS_TASK_PRIO		7
//�����ջ��С	
#define TASK1000MS_STK_SIZE 		128
//������ƿ�
OS_TCB Task1000ms_TaskTCB;
//�����ջ	
CPU_STK TASK1000MS_TASK_STK[TASK100MS_STK_SIZE];
//������
void task1000ms_task(void *p_arg);

//�������ȼ�
#define TASK2_TASK_PRIO		9
//�����ջ��С	
#define TASK2_STK_SIZE 		128
//������ƿ�
OS_TCB Task2_TaskTCB;
//�����ջ	
CPU_STK TASK2_TASK_STK[TASK2_STK_SIZE];
//������
void task2_task(void *p_arg);


//������
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init(168); 		  //ʱ�ӳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	uart_init(115200);    //���ڳ�ʼ��
	LED_Init();           //LED��ʼ��	
	time_info_init();
	
	
	OSInit(&err);		    //��ʼ��UCOSIII
	OS_CRITICAL_ENTER();	//�����ٽ���			 
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);      //����UCOSIII
}


//��ʼ����������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	//����TASK1����
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
				 
	//����TASK2����
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
								 
		//����100ms��ʱ����
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
								 
		//����1000ms��ʱ����
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
	
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}

//**********************************************//
//�˿���Ϣ��ʼ��
//���룺 void
//���أ� void
//**********************************************//
void time_info_init(void)										
{
		int8_t	temp;

		for( temp = 0; temp < 40; temp++ ){
				 
			Port_Information[temp].io_last_sta    = false;
			Port_Information[temp].io_sta			    = false;
			Port_Information[temp].on_time_stamp  = 0.0;
			Port_Information[temp].off_time_stamp = 0.0;
			Port_Information[temp].on_text_ID  = ON_TEXT_ID[temp];					//��Ļ��ͨID��ʼ��
			Port_Information[temp].off_text_ID = OFF_TEXT_ID[temp];					//��Ļ�ض�ID��ʼ��
		}
}

/*
//	GPIO ��ȡ����
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
	
	Port_Information[0].io_sta =  (gpio_data_D >> 7)  & 0x0001;				//ͨ��0����˿�
	Port_Information[1].io_sta =  (gpio_data_D >> 5)  & 0x0001;				//ͨ��1����˿�
	Port_Information[2].io_sta =  (gpio_data_D >> 3)  & 0x0001;				//ͨ��2����˿�
	Port_Information[3].io_sta =  (gpio_data_D >> 1)  & 0x0001;				//ͨ��3����˿�
	Port_Information[4].io_sta =  (gpio_data_D >> 15) & 0x0001;				//ͨ��4����˿�
	Port_Information[5].io_sta =  (gpio_data_D >> 13) & 0x0001;				//ͨ��5����˿�
	Port_Information[6].io_sta =  (gpio_data_D >> 11) & 0x0001;				//ͨ��6����˿�
	Port_Information[7].io_sta =  (gpio_data_D >> 9)  & 0x0001;				//ͨ��7����˿�
	Port_Information[8].io_sta =  (gpio_data_E >> 7)  & 0x0001;				//ͨ��8����˿�
	Port_Information[9].io_sta =  (gpio_data_E >> 4)  & 0x0001;				//ͨ��9����˿�
	Port_Information[10].io_sta = (gpio_data_E >> 3)  & 0x0001;				//ͨ��10����˿�
	Port_Information[11].io_sta = (gpio_data_A >> 6)  & 0x0001;				//ͨ��11����˿�
	Port_Information[12].io_sta = (gpio_data_A >> 5)  & 0x0001;				//ͨ��12����˿�	
	Port_Information[13].io_sta = (gpio_data_A)  			& 0x0001;				//ͨ��13����˿�	
	Port_Information[14].io_sta = (gpio_data_E >> 14) & 0x0001;			  //ͨ��14����˿�
	Port_Information[15].io_sta = (gpio_data_E >> 12) & 0x0001;			  //ͨ��15����˿�
	Port_Information[16].io_sta = (gpio_data_E >> 10) & 0x0001;				//ͨ��16����˿�	
	Port_Information[17].io_sta = (gpio_data_D >> 6)  & 0x0001;				//ͨ��17����˿�
	Port_Information[18].io_sta = (gpio_data_D >> 4)  & 0x0001;				//ͨ��18����˿�	
	Port_Information[19].io_sta = (gpio_data_D >> 2)  & 0x0001;				//ͨ��19����˿�
	Port_Information[20].io_sta = (gpio_data_D)  			& 0x0001;				//ͨ��20����˿�
	Port_Information[21].io_sta = (gpio_data_D >> 14) & 0x0001;				//ͨ��21����˿�
	Port_Information[22].io_sta = (gpio_data_D >> 12) & 0x0001;				//ͨ��22����˿�
	Port_Information[23].io_sta = (gpio_data_D >> 10) & 0x0001;				//ͨ��23����˿�	
	Port_Information[24].io_sta = (gpio_data_D >> 8)  & 0x0001;				//ͨ��24����˿�	
	Port_Information[25].io_sta = (gpio_data_E >> 6)  & 0x0001;				//ͨ��25����˿�
	Port_Information[26].io_sta = (gpio_data_E >> 5)  & 0x0001;				//ͨ��26����˿�
	Port_Information[27].io_sta = (gpio_data_E)       & 0x0001;				//ͨ��27����˿�
	Port_Information[28].io_sta = (gpio_data_A >> 7)  & 0x0001;				//ͨ��28����˿�
	Port_Information[29].io_sta = (gpio_data_A >> 2)  & 0x0001;				//ͨ��29����˿�
	Port_Information[30].io_sta = (gpio_data_A >> 1)  & 0x0001;				//ͨ��30����˿�
	Port_Information[31].io_sta = (gpio_data_E >> 13) & 0x0001;				//ͨ��31����˿�	
	Port_Information[32].io_sta = (gpio_data_E >> 9)  & 0x0001;				//ͨ��32����˿�
	Port_Information[33].io_sta = (gpio_data_E >> 2)  & 0x0001;				//ͨ��33����˿�
	Port_Information[34].io_sta = (gpio_data_E >> 1)  & 0x0001;				//ͨ��34����˿�
	Port_Information[35].io_sta = (gpio_data_A >> 4)  & 0x0001;				//ͨ��35����˿�	
	Port_Information[36].io_sta = (gpio_data_A >> 3)  & 0x0001;				//ͨ��36����˿�	
	Port_Information[37].io_sta = (gpio_data_E >> 15) & 0x0001;				//ͨ��37����˿�
	Port_Information[38].io_sta = (gpio_data_E >> 11) & 0x0001;				//ͨ��38����˿�	
	Port_Information[39].io_sta = (gpio_data_E >> 8)  & 0x0001;				//ͨ��39����˿�	

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
					SetFcolor(65504); 	//����������ɫ��ͨ������ǰ��ɫʵ��
					DisText(150+ch_time.ch_on_time[i]*9.17,109+i*30,0x00,0x02,ch_tim_buf );
					SetFcolor(63488);		//�ָ���ɫǰ��ɫ
			}
			ch_time.ch_sta[i] = temp;
			
	}	
*/

}

//*********************************************//
//touchpad CMD_handler,���������������//
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
		if(  size > 0 )												//���յ�ָ��
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);				//ָ���
		}
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
		
	}
}

//task2������
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
		OSTimeDlyHMSM(0,0,0,400,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
	}
}


void task100ms_task(void *p_arg)			// 0.1s��ʱ
{
		OS_ERR err;
		CPU_SR_ALLOC();
		p_arg = p_arg;	
		
		while(1)
		{
				ch_time.cnt_100ms++;
				OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT,&err);	 //��ʱ100ms
		}
}

void task1000ms_task(void *p_arg)			//	1s��ʱ
{
		OS_ERR err;
		CPU_SR_ALLOC();
		p_arg = p_arg;	
		
		while(1)
		{
				ch_time.cnt_1000ms ++;
				test_value ++;
				OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);	 //��ʱ1000ms
		}
}



