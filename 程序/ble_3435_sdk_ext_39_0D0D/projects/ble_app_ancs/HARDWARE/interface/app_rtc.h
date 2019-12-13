#ifndef APP_RTC_H_
#define APP_RTC_H_
#include "rtc.h"
#include "ALL_Includes.h"
#define ALARM_ON        1
#define ALARM_OFF       0
#define ALARM_CLASS     20   //��������

#define DEFAULT_YEAR              2010  //Ĭ����
#define DEFAULT_MON               1  //Ĭ����
#define DEFAULT_DATE              1  //Ĭ����
#define RTC_DEFAULT_HOUR          0  //Ĭ��ʱ
#define RTC_DEFAULT_MINUTE        0  //Ĭ�Ϸ�
#define RTC_DEFAULT_SECOND        0  //Ĭ����
#define RTC_DEFAULT_WEEK_DAY      1  //Ĭ����

typedef struct
{   
	  u8       mode;//�������ӻ�������;1Ϊ���ӣ�2Ϊ����
    u8       hour;     // ����ʱ
    u8       minute;     //���ӷ�
	  u8       state;     //����״̬
	  u8       week_loop;     //����ģʽ0������,��������λ��1Ϊ����ѭ��
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
//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get(void);
void Text_Alarm(u8 mode,u8 hour,u8 min,u8 sec);
int user_alarm_ind(ke_msg_id_t const msgid,
										struct user_alarm_ind *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id);
void Alarm_Get(u8 Class);	
void Alarm_date_Get(u8 Class);										
#endif



