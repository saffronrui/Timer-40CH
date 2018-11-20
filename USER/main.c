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
	
	delay_init(168);  //ʱ�ӳ�ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	uart_init(115200);   //���ڳ�ʼ��
	LED_Init();         //LED��ʼ��	
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
//	GPIO ��ȡ����
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
					SetFcolor(65504); 	//����������ɫ��ͨ������ǰ��ɫʵ��
					DisText(150+ch_time.ch_on_time[i]*9.17,109+i*30,0x00,0x02,ch_tim_buf );
					SetFcolor(63488);		//�ָ���ɫǰ��ɫ
			}
			ch_time.ch_sta[i] = temp;
			
			
//			if( ((gpiod_data >> i) & 0x01) && (!ch_time.ch_sta[i]) )
//			{
//					ch_time.ch_on_time[i] = (ch_time.cnt_1000ms + ch_time.cnt_100ms);
//					ch_time.ch_sta[i] = CH_ON;
//			}
	}	

}

//task1������
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
		UpdateUI( &ch_time );
//		SetTextValueInt32( 0, 65, ch_time.cnt_1000ms );
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



