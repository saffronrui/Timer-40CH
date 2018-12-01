#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "led.h"
#include "stdio.h"
#include "hmi_app.h"
extern uint8 cmd_buffer[CMD_MAX_SIZE];							 //指令缓存
#define TIME_100MS 10 											 //100毫秒(10个单位)

extern  int8_t	CMD_VAL;

volatile uint32  timer_tick_count = 0; 			 //定时器节拍

static uint16 current_screen_id = 0;				 //当前画面ID

/*! 
 *  \brief  消息处理流程
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//指令类型
	uint8 ctrl_msg = msg->ctrl_msg;   //消息的类型
	uint8 control_type = msg->control_type;//控件类型
	uint16 screen_id = PTR2U16(&msg->screen_id);//画面ID
	uint16 control_id = PTR2U16(&msg->control_id);//控件ID
	uint32 value = PTR2U32(msg->param);//数值
	
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		NotifyReadFlash(1,cmd_buffer+2,size-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
			{
				NotifyScreen(screen_id);
			}
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //按钮控件
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://文本控件
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //进度条控件
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //滑动条控件
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //仪表控件
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://菜单控件
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://选择控件
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://倒计时控件
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
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: 添加用户代码
	current_screen_id = screen_id;//在工程配置中开启画面切换通知，记录当前画面ID

	if(current_screen_id==4)//温度曲线
	{
		uint16 i = 0;
		uint8 dat[100] = {0};

		//生成方波
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//添加数据到通道0

		//生成锯齿波
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//添加数据到通道1
	}
	else if(current_screen_id==9)//二维码
	{
		//二维码控件显示中文字符时，需要转换为UTF8编码，
		//通过“指令助手”，转换“广州大彩123” ，得到字符串编码如下
		uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: 添加用户代码
}

void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%ld",value); //把整数转换为字符串
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//把浮点数转换为字符串(保留一位小数)
	SetTextValue(screen_id,control_id,buffer);
}

void Fresh_GUI( IO_INFO info[], int8_t	size, int16_t	time_stp_100ms )
{
	int8_t i ;

	SetProgressValue(0,  5,  (int)(time_stp_100ms / 10.0));		//更新PAGE0时间进度条，每次刷新更新1000ms
	SetProgressValue(1, 15,  (int)(time_stp_100ms / 10.0));		//更新PAGE1时间进度条，每次刷新更新1000ms	

		for( i = 0; i < size; i++ ){
		
		if((info[i].io_sta == true)	&& (info[i].io_last_sta == false)){		     //端口导通
			
			info[i].on_time_stamp = time_stp_100ms / 10.0;											 //计算时间，单位0.1s
			sprintf(info[i].on_time_string,"%.1f",info[i].on_time_stamp );			 //将时间戳转换为字符串
			SetTextValue(info[i].page_ID, info[i].on_text_ID, info[i].on_time_string);						 //在相应的ID显示时间字符串
			SetControlBackColor(info[i].page_ID, info[i].name_ID, 0x67E6);
			info[i].io_last_sta = true;																					 //更新上次端口值
		
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

	SetProgressValue(0,  5, 0);		//清空PAGE0时间进度条
	SetProgressValue(1, 15, 0);		//清空PAGE1时间进度条

		for( i = 0; i < size; i++ ){
			
			info[i].on_time_stamp = 0;											 										 //计算时间，单位0.1s
			info[i].off_time_stamp = 0;
			SetTextValue(info[i].page_ID, info[i].on_text_ID, "---");						 //在相应的ID显示时间字符串
			SetTextValue(info[i].page_ID, info[i].off_text_ID, "---");					 //清空控件内容
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
			
			SetProgressValue(0,  5,  60*( 1 - (i%2) ));		//更新PAGE0时间进度条，每次刷新更新1000ms
			SetProgressValue(1, 15,  60*( 1 - (i%2) ));		//更新PAGE1时间进度条，每次刷新更新1000ms		
			SetProgressValue(1, 26,  i);									//更新PAGE1自检进度条
			
			delay_ms(1000);
		
	}

			SetProgressValue(0,  5,  0);		//进度条归零
			SetProgressValue(1, 15,  0);		
			SetProgressValue(1, 26,  10);		//自检进度条显示 100%	

}


/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: 添加用户代码
	
//	uint8 i = 0;
	
	if((screen_id == 0)&&(state == 1))				// PAGE0 按钮命令 
	{
		 switch( control_id )
		 {
			 case	7:
				 		LED3 = ~LED3;
						CMD_VAL = 0x1A;					//重置命令代号
					break;
			 default:
					break;
		 }
	}

	if((screen_id == 1)&&(state == 1))				// PAGE1 按钮命令 
	{
		 switch( control_id )
		 {
			 case	7:											//重置命令
				 		LED3 = ~LED3;
						CMD_VAL = 0X1A;					//重置命令代号

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
			 case 27:										 //设备自检命令按钮
						LED3 = ~LED3;
						CMD_VAL = 0X2A;				 //设备自检命令代号
					break;
			 default:
					break;
		 }
	}
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: 添加用户代码
	int32 value = 0; 

	if(screen_id==2)//画面ID2：文本设置和显示
	{
		sscanf(str,"%ld",&value);//把字符串转换为整数

		if(control_id==1)//最高电压
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //更新最高电压
			SetTextValueInt32(2,4,value/2);  //更新最高电压/2
		}
	}
}

/*! 
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
	if(screen_id==5&&control_id==2)//滑块控制
	{
	}
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
}


////定时器3中断服务函数
//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //溢出中断
//	{
//		LED1=!LED1;						//DS1翻转
//		cnt_10ms++;
//		gpio_en = 1;
//	}
//	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
//}




////定时器3中断服务函数
//void TIM3_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
//	{
////		LED1=!LED1;						//DS1翻转
//		cnt_10ms = 0;
//		
//		if( update_en )				//时间更新计数器
//		{
//				test_value++;
//		}
//		
//		flash_en = 1;						//使能主函数刷新屏幕功能
//		
//	}
//	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
//}
