#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "led.h"
#include "stdio.h"
#include "hmi_app.h"
extern uint8 cmd_buffer[CMD_MAX_SIZE];							 //ָ���
#define TIME_100MS 10 											 //100����(10����λ)

extern  int8_t	CMD_VAL;

volatile uint32  timer_tick_count = 0; 			 //��ʱ������

static uint16 current_screen_id = 0;				 //��ǰ����ID

/*! 
 *  \brief  ��Ϣ��������
 *  \param msg ��������Ϣ
 *  \param size ��Ϣ����
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//ָ������
	uint8 ctrl_msg = msg->ctrl_msg;   //��Ϣ������
	uint8 control_type = msg->control_type;//�ؼ�����
	uint16 screen_id = PTR2U16(&msg->screen_id);//����ID
	uint16 control_id = PTR2U16(&msg->control_id);//�ؼ�ID
	uint32 value = PTR2U32(msg->param);//��ֵ
	
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://����������
	case NOTIFY_TOUCH_RELEASE://�������ɿ�
		NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://дFLASH�ɹ�
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://дFLASHʧ��
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://��ȡFLASH�ɹ�
		NotifyReadFlash(1,cmd_buffer+2,size-6);//ȥ��֡ͷ֡β
		break;
	case NOTIFY_READ_FLASH_FAILD://��ȡFLASHʧ��
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://��ȡRTCʱ��
		NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
			{
				NotifyScreen(screen_id);
			}
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //��ť�ؼ�
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://�ı��ؼ�
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //�������ؼ�
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //�������ؼ�
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //�Ǳ�ؼ�
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://�˵��ؼ�
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://ѡ��ؼ�
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://����ʱ�ؼ�
					NotifyTimer(screen_id,control_id);
					break;
				default:
					break;
				}
			}			
		}
		break;
	default:
		break;
	}
}

/*! 
 *  \brief  �����л�֪ͨ
 *  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
 *  \param screen_id ��ǰ����ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: ����û�����
	current_screen_id = screen_id;//�ڹ��������п��������л�֪ͨ����¼��ǰ����ID

	if(current_screen_id==4)//�¶�����
	{
		uint16 i = 0;
		uint8 dat[100] = {0};

		//���ɷ���
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//������ݵ�ͨ��0

		//���ɾ�ݲ�
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//������ݵ�ͨ��1
	}
	else if(current_screen_id==9)//��ά��
	{
		//��ά��ؼ���ʾ�����ַ�ʱ����Ҫת��ΪUTF8���룬
		//ͨ����ָ�����֡���ת�������ݴ��123�� ���õ��ַ�����������
		uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  ���������¼���Ӧ
 *  \param press 1���´�������3�ɿ�������
 *  \param x x����
 *  \param y y����
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: ����û�����
}

void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%ld",value); //������ת��Ϊ�ַ���
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//�Ѹ�����ת��Ϊ�ַ���(����һλС��)
	SetTextValue(screen_id,control_id,buffer);
}

void Fresh_GUI( IO_INFO info[], int8_t	size, int16_t	time_stp_100ms )
{
	int8_t i ;

	SetProgressValue(0,  5,  (int)(time_stp_100ms / 10.0));		//����PAGE0ʱ���������ÿ��ˢ�¸���1000ms
	SetProgressValue(1, 15,  (int)(time_stp_100ms / 10.0));		//����PAGE1ʱ���������ÿ��ˢ�¸���1000ms	

		for( i = 0; i < size; i++ ){
		
		if((info[i].io_sta == true)	&& (info[i].io_last_sta == false)){		     //�˿ڵ�ͨ
			
			info[i].on_time_stamp = time_stp_100ms / 10.0;											 //����ʱ�䣬��λ0.1s
			sprintf(info[i].on_time_string,"%.1f",info[i].on_time_stamp );			 //��ʱ���ת��Ϊ�ַ���
			SetTextValue(info[i].page_ID, info[i].on_text_ID, info[i].on_time_string);						 //����Ӧ��ID��ʾʱ���ַ���
			SetControlBackColor(info[i].page_ID, info[i].name_ID, 0x67E6);
			info[i].io_last_sta = true;																					 //�����ϴζ˿�ֵ
		
		}else	if( (info[i].io_sta == false)	&& (info[i].io_last_sta == true) ){
		
							info[i].off_time_stamp = time_stp_100ms / 10.0;
							sprintf(info[i].off_time_string,"%.1f",info[i].off_time_stamp );
							SetTextValue(info[i].page_ID, info[i].off_text_ID, info[i].off_time_string);
							SetControlBackColor(info[i].page_ID, info[i].name_ID, 0xF980);
							info[i].io_last_sta = false;
		}
	}
}
//*****************************************************//
// Clear GUI
// return: None
//*****************************************************//
void  Clear_GUI(IO_INFO info[], int8_t	size, int16_t	time_stp_100ms)
{
	int8_t i ;

	SetProgressValue(0,  5, 0);		//���PAGE0ʱ�������
	SetProgressValue(1, 15, 0);		//���PAGE1ʱ�������

		for( i = 0; i < size; i++ ){
			
			info[i].on_time_stamp = 0;											 										 //����ʱ�䣬��λ0.1s
			info[i].off_time_stamp = 0;
			SetTextValue(info[i].page_ID, info[i].on_text_ID, "---");						 //����Ӧ��ID��ʾʱ���ַ���
			SetTextValue(info[i].page_ID, info[i].off_text_ID, "---");					 //��տؼ�����
		}
}

//******************************************************//
//Device_Check function()
//Desprition: Use to test the Control Device and Touch screen work Normally
//Input: Info struct , size
//Output: None
//******************************************************//
void  Device_Check(IO_INFO info[], int8_t	size)
{
	int8_t	i = 0, ch_val;
	
	for( i = 0; i < 11; i++ ){
			
			SetProgressValue(0,  5,  60*( 1 - (i%2) ));		//����PAGE0ʱ���������ÿ��ˢ�¸���1000ms
			SetProgressValue(1, 15,  60*( 1 - (i%2) ));		//����PAGE1ʱ���������ÿ��ˢ�¸���1000ms		
			SetProgressValue(1, 26,  i);									//����PAGE1�Լ������
			
			delay_ms(1000);
		
	}

			SetProgressValue(0,  5,  0);		//����������
			SetProgressValue(1, 15,  0);		
			SetProgressValue(1, 26,  10);		//�Լ��������ʾ 100%	

}


/*! 
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: ����û�����
	
//	uint8 i = 0;
	
	if((screen_id == 0)&&(state == 1))				// PAGE0 ��ť���� 
	{
		 switch( control_id )
		 {
			 case	7:
				 		LED3 = ~LED3;
						CMD_VAL = 0x1A;					//�����������
					break;
			 default:
					break;
		 }
	}

	if((screen_id == 1)&&(state == 1))				// PAGE1 ��ť���� 
	{
		 switch( control_id )
		 {
			 case	7:											//��������
				 		LED3 = ~LED3;
						CMD_VAL = 0X1A;					//�����������

					break;
			 case 28:
						LED3 = ~LED3;
					break;
			 case 30:
						LED3 = ~LED3;
					break;
			 case 32:
						LED3 = ~LED3;
					break;
			 case 27:										 //�豸�Լ����ť
						LED3 = ~LED3;
						CMD_VAL = 0X2A;				 //�豸�Լ��������
					break;
			 default:
					break;
		 }
	}
}

/*! 
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: ����û�����
	int32 value = 0; 

	if(screen_id==2)//����ID2���ı����ú���ʾ
	{
		sscanf(str,"%ld",&value);//���ַ���ת��Ϊ����

		if(control_id==1)//��ߵ�ѹ
		{
			//�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //������ߵ�ѹ
			SetTextValueInt32(2,4,value/2);  //������ߵ�ѹ/2
		}
	}
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
	if(screen_id==5&&control_id==2)//�������
	{
	}
}

/*! 
 *  \brief  �Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡ�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 *  \param _data ��������
 *  \param length ���ݳ���
 */
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  д�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
 *  \param year �꣨BCD��
 *  \param month �£�BCD��
 *  \param week ���ڣ�BCD��
 *  \param day �գ�BCD��
 *  \param hour ʱ��BCD��
 *  \param minute �֣�BCD��
 *  \param second �루BCD��
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
}


////��ʱ��3�жϷ�����
//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //����ж�
//	{
//		LED1=!LED1;						//DS1��ת
//		cnt_10ms++;
//		gpio_en = 1;
//	}
//	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //����жϱ�־λ
//}




////��ʱ��3�жϷ�����
//void TIM3_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
//	{
////		LED1=!LED1;						//DS1��ת
//		cnt_10ms = 0;
//		
//		if( update_en )				//ʱ����¼�����
//		{
//				test_value++;
//		}
//		
//		flash_en = 1;						//ʹ��������ˢ����Ļ����
//		
//	}
//	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
//}
