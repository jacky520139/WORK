#ifndef APP_RTC_H_
#define APP_RTC_H_
#include "rtc.h"
#include "ALL_Includes.h"
#define ALARM_ON        1
#define ALARM_OFF       0
#define ALARM_CLASS     20   //闹钟组数

#define DEFAULT_YEAR              2010  //默认年
#define DEFAULT_MON               1  //默认月
#define DEFAULT_DATE              1  //默认日
#define RTC_DEFAULT_HOUR          0  //默认时
#define RTC_DEFAULT_MINUTE        0  //默认分
#define RTC_DEFAULT_SECOND        0  //默认秒
#define RTC_DEFAULT_WEEK_DAY      1  //默认周

typedef struct
{   
	  u8       mode;//区分闹钟或者提醒;1为闹钟，2为提醒
    u8       hour;     // 闹钟时
    u8       minute;     //闹钟分
	  u8       state;     //闹钟状态
	  u8       week_loop;     //闹钟模式0：单次,其他：哪位置1为哪周循环
//	  RTC_DATE_DESC alarm_DATE;
} alarm_dev;

typedef struct
{
    u16      year;     // second, 0~59
    u8       mon;     // minute, 0~59
    u8       date;       // hour,   0~23
	  RTC_DATE_DESC       RTC;
} calendar_dev;
extern calendar_dev calendar;
struct user_alarm_ind
{u8 mode;

};
void Init_RTC(void);
//u8 RTC_Get_Week(u16 year,u8 month,u8 day);
u8 Calendar_Seting(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec);
u8 Calendar_Update_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id, ke_task_id_t const src_id);
//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void);
void Text_Alarm(u8 mode,u8 hour,u8 min,u8 sec);
int user_alarm_ind(ke_msg_id_t const msgid,
										struct user_alarm_ind *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id);
void Alarm_Get(u8 Class);	
void Alarm_date_Get(u8 Class);										
#endif



