#ifndef	_HMI_APP_H
#define	_HMI_APP_H

#include "hmi_user_uart.h"
#include "hmi_driver.h"
#include "cmd_process.h"
#include "stdbool.h"

#define	CH_MAX	13
#define	CH_ON		1
#define	CH_OFF	0

void time_info_init(void);

typedef struct
{
		int16_t	cnt_1000ms;
		int16_t cnt_100ms;
		int16_t	cnt_10ms;
	
}	TIME_INFO, *PTIME_INFO;


// .内部只能定义，例化需要在 mian 函数文件里
 typedef	struct{
 
		float		on_time_stamp;
		float		off_time_stamp;
		uchar 	on_time_string[12];
		uchar		off_time_string[12];
		bool		io_sta;
		bool		io_last_sta;
		int8_t	page_ID;
		int8_t	on_text_ID;
		int8_t	off_text_ID;
		int8_t	name_ID;

 }IO_INFO;
 
void  Clear_GUI(IO_INFO info[], int8_t	size, int16_t	time_stp_100ms);
void  Fresh_GUI(IO_INFO info[], int8_t	size, int16_t	time_stp_100ms);
void  Device_Check(IO_INFO info[], int8_t	size);
void	GpioInit(void);
void	GpioFlash(void);	

void ProcessMessage( PCTRL_MSG msg, uint16 size );
void NotifyScreen(uint16 screen_id);
void NotifyTouchXY(uint8 press,uint16 x,uint16 y);
extern void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value);
void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value);
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state);
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str);
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value);
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value);
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state);
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item);
void NotifyTimer(uint16 screen_id, uint16 control_id);
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length);
void NotifyWriteFlash(uint8 status);
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second);

#endif


