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
//、、、、、、提醒结构体、、、、、、、、、、、、
typedef struct
{   
//	  u8      year;     // 提醒年
	  u8       mon;     // 提醒月
	  u8       day;     // 提醒日
    u8       hour;     // 提醒时
    u8       minute;     //提醒分
    u8       week_loop;
} Remind_dev;
typedef struct 
	{
 u8 on_off;
 u8 hour;
 u8 min;
 u8 week_loop;
}flash_alarm_date_tag;
extern u8 ALARM_ID_LIST[10];
extern u8 REMIND_ID_LIST[10];;
void Init_RTC(void);
void Creat_Alarm(u8 Class,u8 hour,u8 min,u8 Week_loop);
void Creat_Remind(u8 Class,u8 mon,u8 day,u8 hour,u8 min,u8 Week_loop);
void flash_Close_alarm(u8 Class);
void flash_Close_Remind(u8 Class);
//u8 RTC_Get_Week(u16 year,u8 month,u8 day);
u8 Calendar_Seting(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec);
int Calendar_Update_handler(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id, ke_task_id_t const src_id);
//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void);
void Text_Alarm(u8 mode,u8 hour,u8 min,u8 sec);
int user_alarm_ind(ke_msg_id_t const msgid, void const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
void Alarm_Get(u8 Class);	
void Alarm_date_Get(u8 Class);
//void flash_write_alarm(u8 Class,flash_alarm_date_tag *flash_alarm);
void flash_Load_alarm(flash_alarm_date_tag *flash_alarm);//从FLASH中写闹钟数据
void flash_Load_Remind(Remind_dev *flash_Remind);
u8 Load_Alarm_Date(void);
void Alarm_Setup(void);//从flash读出闹钟和提醒
#endif



