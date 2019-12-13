#include "ALL_Includes.h"

#define USER_APP_CALENDAR_UPDATE_TIME   360000//1�����һ��
//#define USER_APP_CALENDAR_UPDATE_TIME   36//1�����һ��
enum 
{
ALARM_CLOCK,/*����*/
REMIND     /*����*/
};
calendar_dev calendar;//�����ṹ��

struct alarm_date
{u8 state;
 u8 week_loop;
 u8 mode;
 u8 hour;
 u8 min;
};

struct alarm_date alarm1,alarm2,alarm3,alarm4,alarm5,alarm6,alarm7,alarm8,alarm9,alarm10,\
	                alarm11,alarm12,alarm13,alarm14,alarm15,alarm16,alarm17,alarm18,alarm19,alarm20;//����5������
 struct alarm_date*  const alarm[ALARM_CLASS]={&alarm1, &alarm2, &alarm3, &alarm4, &alarm5,
                                              &alarm6, &alarm7, &alarm8, &alarm9, &alarm10,
	                                            &alarm11,&alarm12,&alarm13,&alarm14,&alarm15,
                                              &alarm16,&alarm17,&alarm18,&alarm19,&alarm20};//����

struct alarm_env_tag
{u8 mode:2;//�����������Ӻ�����
 u8 ID:6;//����ID
// u8 state;
 u8 hour;
 u8 min;
 u8 time;//����ʱ�䣬��λ��
};
struct alarm_env_tag alarm_env;

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
////////////////////////�жϻص�����///////////////////////////////////
void Alarm_cb(void)
{
	   ke_msg_send_basic(USER_ALARM_IND, TASK_APP,TASK_APP);
     KEY1_Dev.Value=0;
}
////////////////////////�������ú���///////////////////////////////////
void alarm_set(u8 hour,u8 minute)
{ RTC_DATE_DESC Alarm_DATE;
	Alarm_DATE.hour=hour;
	Alarm_DATE.minute=minute;
	Alarm_DATE.second=5;
  rtc_alarm_init(0, &Alarm_DATE,0, Alarm_cb);//��������
	}



//����10������
u8 Load_Alarm_Date(void)//ÿ��������Ϻ���Ҫ�ؼ�����������
{	u8 Ret=0;
	u8 Alarm_DATE_hour=0xaa;
	u8 Alarm_DATE_minute=0xaa;
	u8 i,id=ALARM_CLASS;
    rtc_get_time(&calendar.RTC);//��ȡ����ʱ��
for(i=0;i<ALARM_CLASS;i++)
	{  //�ж����������뵱ǰʱ��Ĵ�С��ϵ��ȷ���Ƿ��������
		if((calendar.RTC.hour>alarm[i]->hour)||
			((calendar.RTC.hour==alarm[i]->hour)&&(calendar.RTC.minute>=alarm[i]->min)))
		{
	   alarm[i]->state=ALARM_OFF;//�رո�������
	  }

	}
for(i=0;i<ALARM_CLASS;i++)
{
	if((alarm[i]->state==ALARM_ON))//��������Ǽ���״̬
	{//��Ѱ�����е��������ݵ���Сֵ
   if((Alarm_DATE_hour>alarm[i]->hour)||
		 ((Alarm_DATE_hour==alarm[i]->hour)&&(Alarm_DATE_minute>alarm[i]->min)))
   {
    Alarm_DATE_hour=alarm[i]->hour;
    Alarm_DATE_minute=alarm[i]->min;
		id=i;
//		UART_PRINTF("alarm.state_id = %d\r\n",id);		 
              }
   }
}
if(id!=ALARM_CLASS)//˵������������Ҫ����
{alarm_env.hour=alarm[id]->hour;
 alarm_env.min=alarm[id]->min;
 alarm_env.mode=alarm[id]->mode;
 alarm_env.ID=id;
	if(alarm_env.mode==ALARM_CLOCK)
 alarm_env.time=30;//����30��
	else if(alarm_env.mode==REMIND)
 alarm_env.time=10;//����10��
			UART_PRINTF("alarm.state_id = %d\r\n",id);
		UART_PRINTF("alarm.state_hour = %d\r\n",alarm_env.hour);
	UART_PRINTF("alarm.state_min = %d\r\n",alarm_env.min);

 alarm_set(alarm_env.hour, alarm_env.min);
 Ret=1;
}

return Ret;
}
void flash_Close_alarm(u8 Class);
////////////////////////�������庯��///////////////////////////////////
int user_alarm_ind(ke_msg_id_t const msgid,
										struct user_alarm_ind *param,
										ke_task_id_t const dest_id,
										ke_task_id_t const src_id)
{	

  static u8 Vib_cont=0;
		Vib_cont++;
   if((Vib_cont>=alarm_env.time)||(KEY1_Dev.Value==1))//�������������
	 {	Vib_cont=0;
		  KEY1_Dev.Value=0;
		  ke_timer_clear(USER_ALARM_IND,TASK_APP);
		 	UART_PRINTF("ke_timer_clear\r\n");
		  
		 alarm[alarm_env.ID]->state=ALARM_OFF;//�رո�������
		 if(alarm[alarm_env.ID]->week_loop==0)//����ǵ�������
		 {
			flash_Close_alarm(alarm_env.ID);
		 }
		 Load_Alarm_Date();//������һ������
		 }
   else
   {  if(alarm_env.mode==ALARM_CLOCK)
	    user_Motor_handler(3);//����
		  else if(alarm_env.mode==REMIND)
	    user_Motor_handler(1);//����
		  ke_timer_set(USER_ALARM_IND,TASK_APP,100);
		  UART_PRINTF("Vib_cont=%d\r\n", Vib_cont);
	  }	
	return KE_MSG_CONSUMED;
}
typedef struct
{   
//    u8      Inde;     // ��������
//	  u8      Total;     // ����������
//	  u16      year;     // ������
	  u8       mon;     // ������
	  u8       day;     // ������
    u8       hour;     // ����ʱ
    u8       minute;     //���ѷ�
    u8       week_loop;
} Remind_dev;
Remind_dev Remind;
//�����ֻ�����������
void Alarm_Set(u8 Class,u8 hour,u8 min,u8 week_loop,u8 State)
{        alarm[Class]->mode=ALARM_CLOCK;
		     alarm[Class]->hour=hour;
//         alarm[Class]->minute=min;
//	       alarm[Class]->week_loop=week_loop;
	       alarm[Class]->state=State;
}
typedef struct 
	{
 u8 on_off;
 u8 hour;
 u8 min;
 u8 week_loop;
}flash_alarm_date_tag;
#define  FLASH_ALARM_CLOCK_ADDRESS   0x42000   //�������ݴ洢��ַ
#define  FLASH_REMIND_ADDRESS        0x42100   //�������ݴ洢��ַ

void flash_read_alarm(u8 Class,flash_alarm_date_tag *flash_alarm)//��FLASH�ж�����������
{u8 len=sizeof(flash_alarm_date_tag);
	flash_read(0,FLASH_ALARM_CLOCK_ADDRESS+len*Class,len,(u8*)flash_alarm,NULL);
}
void flash_write_alarm(u8 Class,flash_alarm_date_tag *flash_alarm)//��FLASH��д��������
{u8 len=sizeof(flash_alarm_date_tag);
  FLASH_Write(FLASH_ALARM_CLOCK_ADDRESS+len*Class,(u8*)flash_alarm,len);
}
void flash_read_Remind(u8 Class,Remind_dev *flash_Remind)//��FLASH�ж�����������
{u8 len=sizeof(Remind_dev);
	flash_read(0,FLASH_REMIND_ADDRESS+len*Class,len,(u8*)flash_Remind,NULL);
}
void flash_write_Remind(u8 Class,Remind_dev *flash_Remind)//��FLASH�ж�����������
{
u8 len=sizeof(Remind_dev);
FLASH_Write(FLASH_REMIND_ADDRESS+len*Class,(u8*)flash_Remind,len);
}
void flash_Close_alarm(u8 Class)//�ر�flash�е�ĳ������
{
flash_alarm_date_tag flash_alarm;
flash_read_alarm(Class,&flash_alarm);
flash_alarm.on_off=ALARM_OFF;
flash_write_alarm(Class,&flash_alarm);//��FLASH��д��������

}
u16 Remind_Total=0;//����������
void Alarm_Setup(void)//ÿ�����һ��
{
 	u8 i,j,k=0;
	flash_alarm_date_tag flash_alarm;
	Remind_dev flash_Remind;
for(i=0;i<10;i++)
	{//flash����10������
		flash_read_alarm(i,&flash_alarm);//��FLASH��������
		if(flash_alarm.on_off==ALARM_ON)//�����ڼ���״̬
		{	
	if((flash_alarm.week_loop&(1<<calendar.RTC.week_day))||(flash_alarm.week_loop==0))
	{ alarm[i]->mode=ALARM_CLOCK;
		alarm[i]->state=ALARM_ON;
		alarm[i]->hour=flash_alarm.hour;
    alarm[i]->min=flash_alarm.min;
		alarm[i]->week_loop=flash_alarm.week_loop;
//	UART_PRINTF("alarm.class%d = %d\r\n",i,flash_alarm.on_off);
//	UART_PRINTF("alarm.mode = %d\r\n",ALARM_CLOCK);
//	UART_PRINTF("alarm.state = %d\r\n",flash_alarm.on_off);
//	UART_PRINTF("alarm.hour = %d\r\n",flash_alarm.hour);
//	UART_PRINTF("alarm.min = %d\r\n",flash_alarm.min);
//	UART_PRINTF("alarm.week_loop = %d\r\n",flash_alarm.week_loop);
	  }
	 else 
	 { 
	 alarm[i]->state=ALARM_OFF;
//		 UART_PRINTF("alarm.class%d = %d\r\n",i,ALARM_OFF);
	 }
   }
	else{alarm[i]->state=ALARM_OFF;
//		UART_PRINTF("alarm.class%d = %d\r\n",i,ALARM_OFF);
	     }
	}
for(j=0;j<10;j++)
{ 
	flash_read_Remind(i,&flash_Remind);//��FLASH��������
	if(flash_Remind.week_loop==0xff)
		continue;
	if(((calendar.mon==flash_Remind.mon)&&(calendar.year==flash_Remind.day))||\
		 (flash_Remind.week_loop&(1<<calendar.RTC.week_day)))
	{ k++;
		if(k<10)
		{
	 alarm[i+k]->mode=REMIND;
	 alarm[i+k]->state=ALARM_ON;
	 alarm[i+k]->hour=flash_Remind.hour;
   alarm[i+k]->min=flash_Remind.minute;
		}	
//   else  
//     break;	//1��5������
	 
	 
	 }
	 
}	
if((i+k)<20)//������Ӻ���������С��10
{
	for(u8 j=i+k+1;j<20;j++)
	{
   alarm[j]->state=ALARM_OFF;
	}
}
 Load_Alarm_Date();
}
	
//void Alarm_get(u8 Class)
//{

//}




//�����ֻ�����������
//void Alarm_Set(u8 Class,u8 hour,u8 min,u8 week_loop,u8 State)
//{        
//	       alarm[Class]->mode=1;
//		     alarm[Class]->alarm_DATE.hour=hour;
//         alarm[Class]->alarm_DATE.minute=min;
//	       alarm[Class]->week_loop=week_loop;
//	       alarm[Class]->state=State;

//}


void Text_Alarm(u8 mode,u8 hour,u8 min,u8 sec)
{
//RTC_DATE_DESC Alarm_DATE={sec,min,hour,1};
//rtc_set_time(&Alarm_DATE);
//Alarm_DATE.second=Alarm_DATE.second+1;
//alarm_env.mode=mode;
//alarm_env.ID=1;
////alarm_env.state=1;
//alarm_env.hour=hour;
//alarm_env.min=min;
//UART_PRINTF("rtc.hour=%d,rtc.minute=%d,rtc.second=%d\r\n", Alarm_DATE.hour,Alarm_DATE.minute,Alarm_DATE.second);
//rtc_alarm_init(0, &Alarm_DATE,0, Alarm_cb);//��������
//rtc_get_time(&Alarm_DATE);
//UART_PRINTF("rtc.hour=%d,rtc.minute=%d,rtc.second=%d\r\n", Alarm_DATE.hour,Alarm_DATE.minute,Alarm_DATE.second);
  flash_alarm_date_tag flash_alarm;
	flash_alarm.on_off=ALARM_ON;
	flash_alarm.hour=hour;
	flash_alarm.min=min;
  flash_alarm.week_loop=sec;
  flash_write_alarm(mode,&flash_alarm);
  Alarm_Setup();	
}
void Alarm_Get(u8 Class)
{
  flash_alarm_date_tag flash_alarm;	
  flash_read_alarm(Class,&flash_alarm);
	UART_PRINTF("flash_alarm.on_off = %d\r\n",flash_alarm.on_off);
	UART_PRINTF("flash_alarm.hour = %d\r\n",flash_alarm.hour);
	UART_PRINTF("flash_alarm.min = %d\r\n",flash_alarm.min);
	UART_PRINTF("flash_alarm.week_loop = %d\r\n",flash_alarm.week_loop);
}
void Alarm_date_Get(u8 Class)
{u8 i=0;
	for(i=0;i<20;i++)
	{
	UART_PRINTF("alarm[%d]->mode= %d\r\n",i,alarm[i]->mode);
	UART_PRINTF("alarm[%d]->state= %d\r\n",i,alarm[i]->state);
	UART_PRINTF("alarm[%d]->hour= %d\r\n",i,alarm[i]->hour);
	UART_PRINTF("alarm[%d]->min= %d\r\n",i,alarm[i]->min);
	UART_PRINTF("alarm[%d]->week_loop= %d\r\n",i,alarm[i]->week_loop);

	}
}





















