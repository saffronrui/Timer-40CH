#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "led.h"
#include "stdio.h"
#include "hmi_app.h"
extern uint8 cmd_buffer[CMD_MAX_SIZE];							 //指令缓存
#define TIME_100MS 10 											 //100毫秒(10个单位)


volatile uint32  timer_tick_count = 0; 			 //定时器节拍

static uint16 current_screen_id = 0;				 //当前画面ID
static long test_value = 0;								 //测试值
static  int update_en = 0;									 //更新标记
static	int	flash_en = 0;
static	int	cnt_10ms = 0;

//static	float   ch_tim[42]  = {0};
//static	uint16	ch_sta[42]	=	{0};

//static uint16_t	gpiod_data;
//static uint16_t	gpioe_data;
//static uint16_t	gpioc_data;
//static int	gpio_en	=	0;


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

void UpdateUI( PTIME_INFO info )
{
	int i = 0;
	uchar ch_tim_buf[12] = {0} ;
//	current_screen_id = 0;		//调试添加语句
	
	SetTextValueInt32( 0, 65, info->cnt_1000ms );
	
//	GUI_RectangleFill(221+info->cnt_1000ms, 109, 250+info->cnt_1000ms,125);
//	SetProgressValue(0, 10, info->cnt_100ms);
//	SetProgressValue(0, 23, info->cnt_100ms);
//	SetProgressValue(0, progress_bar_ID, info -> cnt_100ms);		//更新时间进度条，每次刷新更新0.5s
	
	BatchBegin(0);
	
	if( info ->ch_sta[0] == CH_ON)	
			BatchSetProgressValue(CH1_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[1] == CH_ON)	
			BatchSetProgressValue(CH2_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[2] == CH_ON)	
			BatchSetProgressValue(CH3_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[3] == CH_ON)	
			BatchSetProgressValue(CH4_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[4] == CH_ON)	
			BatchSetProgressValue(CH5_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[5] == CH_ON)	
			BatchSetProgressValue(CH6_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[6] == CH_ON)	
			BatchSetProgressValue(CH7_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[7] == CH_ON)	
			BatchSetProgressValue(CH8_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[8] == CH_ON)	
			BatchSetProgressValue(CH9_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[9] == CH_ON)	
			BatchSetProgressValue(CH10_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[10] == CH_ON)	
			BatchSetProgressValue(CH11_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[11] == CH_ON)	
			BatchSetProgressValue(CH12_bar_ID, info -> cnt_1000ms);
	if( info ->ch_sta[12] == CH_ON)	
			BatchSetProgressValue(CH13_bar_ID, info -> cnt_1000ms);	
	BatchEnd();

	
	
	
////	if( current_screen_id == 0 )															//文本设置和显示
////	{
//	SetTextValueInt32( 0, 65, test_value  );						  	//txt65，更新显示计数器
//	SetTextValueInt32( 1, 65, test_value  );						  	//txt65，更新显示计数器

//	if( info ->cnt_1000ms % 60 == 0)				// 测试蜂鸣器，正式使用时可以删除，每5min响一次
//		SetBuzzer(0x3A);
////	
//	BatchBegin(0x0000);
//	for( i = 0; i < 20; i++ )
//	{
//			sprintf(ch_tim_buf, "%.02f", info ->ch_on_time[i]);
//			BatchSetText(41+i, ch_tim_buf);
//	}
////	BatchEnd();
////	BatchBegin(0x0001);
//	for( i = 0; i < 20; i++ )
//	{
//			sprintf(ch_tim_buf, "%.02f", info ->ch_on_time[i]);
//			BatchSetText(41+i, ch_tim_buf);
//	}
//	BatchEnd();	
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
	
	uint8 i = 0;
	
	if((screen_id == 0)&&(state == 1))
	{
		 switch( control_id )
		 {
			 case	62:
				  update_en = 0;
					flash_en	= 0;
					LED0 = !LED0;
					break;
			 case	63:
				  update_en = 1;
					LED1 = !LED1;
					break;
			 case	64:
					test_value = 0;
					cnt_10ms	 = 0;
					update_en  = 0;
					flash_en	 = 0;
					
					for( i = 0; i < 42; i++ )			//	数据清零
					{
//							ch_sta[i] = 0;
//							ch_tim[i] = 0;
					}
					
//					UpdateUI();
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
		test_value = value;

		SetProgressValue(5,1,test_value); //更新进度条数值
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
