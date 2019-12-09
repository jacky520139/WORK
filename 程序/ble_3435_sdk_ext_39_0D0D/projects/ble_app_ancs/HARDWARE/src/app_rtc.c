#include "ALL_Includes.h"
#define USER_APP_CALENDAR_UPDATE_TIME   360000//1�����һ��
//#define USER_APP_CALENDAR_UPDATE_TIME   36//1�����һ��
calendar_dev calendar;//�����ṹ��
alarm_dev alarm1,alarm2,alarm3,alarm4,alarm5;
alarm_dev*  alarm[alarm_class]={&alarm1,&alarm2,&alarm3,&alarm4,&alarm5};//����

u8 RTC_Get_Week(u16 year,u8 month,u8 day);
static u32 daycnt=0;
static u8 weekcnt=0;

void Init_RTC(void)
{
calendar.year=DEFAULT_YEAR;
calendar.mon=DEFAULT_MON;
calendar.date=DEFAULT_DATE;
calendar.RTC.hour=RTC_DEFAULT_HOUR; 
calendar.RTC.minute=RTC_DEFAULT_MINUTE;
calendar.RTC.second=RTC_DEFAULT_SECOND;
calendar.RTC.week_day=RTC_DEFAULT_WEEK_DAY;
rtc_init(&calendar.RTC);
rtc_enable();

}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	
	
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 Calendar_Seting(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec)
{
//calendar.year=year;
//calendar.mon=mon;
//calendar.date=day;
//calendar.RTC.hour=hour;
//calendar.RTC.minute=min;
//calendar.RTC.second=sec;
//calendar.RTC.week_day=RTC_Get_Week(year,mon,day);
//rtc_init(&calendar.RTC);
calendar.year=year;
calendar.mon=mon;
calendar.date=day;
calendar.RTC.hour=hour;
calendar.RTC.minute=min;
calendar.RTC.second=sec;
calendar.RTC.week_day=RTC_Get_Week(year,mon,day);
rtc_init(&calendar.RTC);
	u16 t;
//	u32 seccount=0;
	daycnt=0;
	if(year<2010||year>2099)return 1;	   
	for(t=2010;t<year;t++)	//��������ݵ��������
	{
		if(Is_Leap_Year(t))daycnt+=366;//�����������
		else daycnt+=365;			  //ƽ���������
	}
	mon-=1;
	for(t=0;t<mon;t++)	   //��ǰ���·ݵ����������
	{
		daycnt+=(u32)mon_table[t];//�·����������
		if(Is_Leap_Year(year)&&t==1)daycnt+=1;//����2�·�����һ���������	   
	}
	daycnt+=(u32)(day-1);//��ǰ�����ڵ����������
	weekcnt=calendar.RTC.week_day;
	ke_msg_send_basic(USER_APP_CALENDAR_UPDATE, TASK_APP,TASK_APP);
	UART_PRINTF("daycnt=%d\r\n",daycnt);		
  UART_PRINTF("%d-%d-%d %d:%d:%d-%d\r\n",calendar.year,calendar.mon,calendar.date,calendar.RTC.hour,calendar.RTC.minute,calendar.RTC.second,calendar.RTC.week_day);
	
//u8 Calendar_Update();
return 0;	    
}
//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 Calendar_Update_handler(ke_msg_id_t const msgid, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{


	u16 temp1=0;	 
	rtc_get_time(&calendar.RTC);	
	if(calendar.RTC.week_day!=weekcnt)//����һ����
	{	  
		weekcnt=calendar.RTC.week_day;
		daycnt++;
		temp1=2010;	//��2010�꿪ʼ
//		temp1=calendar.year;
		while(daycnt>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(daycnt>=366)daycnt-=366;//�����������
				else {temp1++;break;}  
			}
			else daycnt-=365;	  //ƽ�� 
			temp1++;  
		}   
		calendar.year=temp1;//�õ����
		temp1=0;
		while(daycnt>=28)//������һ����
		{
			if(Is_Leap_Year(calendar.year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(daycnt>=29)daycnt-=29;//�����������
				else break; 
			}
			else 
			{
				if(daycnt>=mon_table[temp1])daycnt-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		calendar.mon=temp1+1;	//�õ��·�
		calendar.date=daycnt+1;  	//�õ����� 
	}
	ke_timer_set(USER_APP_CALENDAR_UPDATE,TASK_APP , USER_APP_CALENDAR_UPDATE_TIME);//1�����һ����������
	return 0;
}	 
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}		

u8 Alarm_ID;
void Alarm_cb(void)
{



}
//����5������
u8 Load_Alarm(void)
{	u8 Ret=0;
	RTC_DATE_DESC Alarm_DATE={0xff,0xff,0xff,calendar.RTC.week_day};
for(u8 i=0;i<alarm_class;i++)
{
	if((alarm[i]->state==alarm_on)&&(Alarm_DATE.week_day==alarm[i]->alarm_DATE.week_day))//��������Ǽ���״̬
	{
   if(Alarm_DATE.hour>alarm[i]->alarm_DATE.hour)
   {
    Alarm_DATE.hour=alarm[i]->alarm_DATE.hour;
    Alarm_DATE.minute=alarm[i]->alarm_DATE.minute;
    Alarm_DATE.second=alarm[i]->alarm_DATE.second;		 
              }
	 else if(Alarm_DATE.hour==alarm[i]->alarm_DATE.hour)
	 {  if(Alarm_DATE.minute>alarm[i]->alarm_DATE.minute)
	      {
				 Alarm_DATE.hour=alarm[i]->alarm_DATE.hour;
         Alarm_DATE.minute=alarm[i]->alarm_DATE.minute;
         Alarm_DATE.second=alarm[i]->alarm_DATE.second;
				}
	     else if(Alarm_DATE.minute==alarm[i]->alarm_DATE.minute)
			 {     if(Alarm_DATE.second>alarm[i]->alarm_DATE.second)
			         {
							 	Alarm_DATE.hour=alarm[i]->alarm_DATE.hour;
                Alarm_DATE.minute=alarm[i]->alarm_DATE.minute;
                Alarm_DATE.second=alarm[i]->alarm_DATE.second;
							  }
			 
			    }
	     }

   }

}

if((Alarm_DATE.hour!=0xff)&&(Alarm_DATE.minute!=0xff)&&(Alarm_DATE.second!=0xff))//˵������������Ҫ����
{
  rtc_alarm_init(0, &Alarm_DATE,0, Alarm_cb);//��������
	Ret=1;
}

return Ret;
}

























