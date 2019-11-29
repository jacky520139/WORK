#ifndef APP_RTC_H_
#define APP_RTC_H_
#include "rtc.h"
#include "ALL_Includes.h"
#define alarm_on        1
#define alarm_off       0
#define alarm_class     5   //闹钟组数

#define DEFAULT_YEAR              12  //默认年
#define DEFAULT_MON               12  //默认月
#define DEFAULT_DATE              12  //默认日
#define RTC_DEFAULT_HOUR          12  //默认时
#define RTC_DEFAULT_MINUTE        12  //默认分
#define RTC_DEFAULT_SECOND        12  //默认秒
#define RTC_DEFAULT_WEEK_DAY      12  //默认周

typedef struct
{
//    u8       hour;     // 闹钟时
//    u8       minute;     //闹钟分
	  u8       state;     //闹钟状态
	  u8       mode;     //闹钟模式0：单次，1：周循环
	  RTC_DATE_DESC alarm_DATE;
} alarm_dev;

typedef struct
{
    u16      syear;     // second, 0~59
    u8       smon;     // minute, 0~59
    u8       sdate;       // hour,   0~23
	  RTC_DATE_DESC       RTC;
} calendar_dev;
extern calendar_dev calendar;

void Init_RTC(void);
u8 RTC_Get_Week(u16 year,u8 month,u8 day);
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);

//得到当前的时间
//返回值:0,成功;其他:错误代码.
u8 RTC_Get(void);

#endif



