#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions

#include "co_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "rf.h"
#include "prf_utils.h"
#include "ke_timer.h"
#include "uart.h"
#include "gpio.h"


#include "audio.h"
#include "icu.h"
#include "BK3435_reg.h"
#include "lld_evt.h"

#include "ALL_Includes.h"
#include "flash.h"         // flash definition
void Init_LED1(void)
{ 
	PWM_DRV_DESC PWM0;
	RTC_DATE_DESC RTC_DATE;
	PWM0.channel=0;
	PWM0.mode=0X11;
	PWM0.end_value=0XFFFF;
	PWM0.duty_cycle=100;
	pwm_init(&PWM0);
  pwm_enable(0);
//	  adc_init(1,1);//��ʼ��ADC,���ģʽ
	GLOBAL_INT_START();//ʹ��ȫ���ж�
	//��ʼ��LED
	gpio_config(BlueLedPort, OUTPUT, PULL_NONE);
	gpio_config(RedLedPort, OUTPUT, PULL_NONE);
//  gpio_config(MotorPort, OUTPUT, PULL_NONE);
	gpio_config(ButtonPort, INPUT, PULL_HIGH);
	gpio_set(BlueLedPort, 1);
	gpio_set(RedLedPort, 1);
  gpio_set(ButtonPort, 0);
	
RTC_DATE.hour=18;
RTC_DATE.minute=30;
RTC_DATE.second=0;
RTC_DATE.week_day=3;
rtc_init(&RTC_DATE);
rtc_enable();
// u16 adc_val=adc_get_value(8);
//	u16 BAT_VCC=(adc_val*490)>>8;//VCC��ѹ
while(1)
{

if(gpio_get_input(ButtonPort))
{	gpio_set(BlueLedPort, 1);
	gpio_set(RedLedPort, 1);
//	gpio_set(MotorPort, 0);
}
	else
	{
  gpio_set(BlueLedPort, 0);
	gpio_set(RedLedPort, 0);
//	gpio_set(MotorPort, 1);
	}	
//	adc_get_value(1);
		rtc_get_time(&RTC_DATE);
	ANO_DT_Send_Version(0xf1,RTC_DATE.hour, RTC_DATE.minute, RTC_DATE.second, RTC_DATE.week_day, 5);
	}





}
void BlueLed(void)
{
	
BlueLedToggle();
//RedLedToggle();

}

#include "stdarg.h"
#include "app_fff0.h"              // Battery Application Module Definitions
char ble_uart_buff[128];
int ble_uart_putchar(char * st)
{
	uint8_t num = 0;
	char *st_temp=st;
	while (*st)
	{
			st++;
			num++;
	}
	app_fff1_send_lvl((uint8_t*)st_temp,num);

	uart_write((uint8_t*)st_temp,num,0,0);
	return num;
}
int ble_printf(const char *fmt,...)
{
#if (UART_PRINTF_EN && UART_DRIVER)
	int n;

	va_list ap;
	va_start(ap, fmt);
	n=vsprintf(ble_uart_buff, fmt, ap);
	va_end(ap);
	ble_uart_putchar(ble_uart_buff);
	if(n > sizeof(ble_uart_buff))
	{
		ble_uart_putchar("buff full \r\n");
	}

	return n;
#else
    return 0;
#endif
}
#define  USMART_PRINTF1 ble_printf
void ble_printf_text(void)
{
////	ble_printf("hallo word");
//	while(1);
      USMART_PRINTF1("------------------------USMART V3.1------------------------ \r\n");
//			USMART_PRINTF1("    USMART����ALIENTEK������һ�����ɵĴ��ڵ��Ի������,ͨ�� \r\n");
//			USMART_PRINTF1("��,�����ͨ���������ֵ��ó���������κκ���,��ִ��.���,���\r\n");
//			USMART_PRINTF1("��������ĺ������������(֧������(10/16����)���ַ�����������\r\n");	  
//			USMART_PRINTF1("�ڵ�ַ����Ϊ����),�����������֧��10���������,��֧�ֺ����� \r\n");
//			USMART_PRINTF1("��ֵ��ʾ.����������ʾ�������ù���,��������ת������.\r\n");
//			USMART_PRINTF1("����֧��:www.openedv.com\r\n");
//			USMART_PRINTF1("USMART��7��ϵͳ����:\r\n");
//			USMART_PRINTF1("?:      ��ȡ������Ϣ\r\n");
//			USMART_PRINTF1("help:   ��ȡ������Ϣ\r\n");
//			USMART_PRINTF1("list:   ���õĺ����б�\r\n\n");
//			USMART_PRINTF1("id:     ���ú�����ID�б�\r\n\n");
//			USMART_PRINTF1("hex:    ����16������ʾ,����ո�+���ּ�ִ�н���ת��\r\n\n");
//			USMART_PRINTF1("dec:    ����10������ʾ,����ո�+���ּ�ִ�н���ת��\r\n\n");
//			USMART_PRINTF1("runtime:1,�����������м�ʱ;0,�رպ������м�ʱ;\r\n\n");
//			USMART_PRINTF1("�밴�ճ����д��ʽ���뺯�������������Իس�������.\r\n");    
//			USMART_PRINTF1("--------------------------ALIENTEK------------------------- \r\n");

}
typedef void(*LED_CTRL_env)(void);
//void LED_OFF(void)    {RES_Get1();RES_Get2();RES_Get3();Set_COM1();Set_COM2();Set_COM3();Set_COM4();}
//void LED_ON(void)     {Set_Get1();Set_Get2();Set_Get3();RES_COM1();RES_COM2();RES_COM3();RES_COM4();}
//void LED1_ON(void)    {Set_Get2();RES_COM1();}
//void LED2_ON(void)    {Set_COM1();Set_Get2();RES_COM2();}
//void LED3_ON(void)    {RES_Get2();Set_Get1();RES_COM2();}
//void LED4_ON(void)    {RES_Get1();Set_COM2();Set_Get2();RES_COM3();}
//void LED5_ON(void)    {Set_COM3();Set_Get2();RES_COM4();}
//void LED6_ON(void)    {RES_Get2();Set_COM4();Set_Get1();RES_COM3();}
//void LED7_ON(void)    {RES_Get1();Set_COM3();Set_Get3();RES_COM1();}
//void LED8_ON(void)    {Set_COM1();Set_Get3();RES_COM2();}
//void LED9_ON(void)    {RES_Get3();Set_COM2();Set_Get1();RES_COM4();}
//void LED10_ON(void)   {RES_Get1();Set_COM4();Set_Get3();RES_COM3();}
//void LED11_ON(void)   {Set_COM3();Set_Get3();RES_COM4();}
//void LED12_ON(void)   {Set_COM3();Set_Get3();RES_COM4();}

void LED_OFF(void)    {RES_Get1();RES_Get2();RES_Get3();Set_COM1();Set_COM2();Set_COM3();Set_COM4();}
void LED_ON(void)     {BlueLedToggle();}
void LED1_ON(void)    {BlueLedToggle();}
void LED2_ON(void)    {RedLedToggle();}
void LED3_ON(void)    {BlueLedToggle();}
void LED4_ON(void)    {RedLedToggle();}
void LED5_ON(void)    {BlueLedToggle();}
void LED6_ON(void)    {RedLedToggle();}
void LED7_ON(void)    {BlueLedToggle();}
void LED8_ON(void)    {RedLedToggle();}
void LED9_ON(void)    {BlueLedToggle();}
void LED10_ON(void)   {RedLedToggle();}
void LED11_ON(void)   {BlueLedToggle();}
void LED12_ON(void)   {RedLedToggle();}
static const LED_CTRL_env LED_ctrl_handler[14]={LED_OFF,LED1_ON,LED2_ON,LED3_ON,LED4_ON,LED5_ON,LED6_ON,LED7_ON,LED8_ON,LED9_ON,LED10_ON,LED11_ON,LED12_ON,};

struct LED_Dev_tag LED_Dev;
#define LED_OPEN_T      10;
#define LED_CLOSE_T     90;
void Init_LED(void)
{	 
	gpio_config(BlueLedPort, OUTPUT, PULL_NONE);
	gpio_config(RedLedPort, OUTPUT, PULL_NONE);
	gpio_set(BlueLedPort, 0);
	gpio_set(RedLedPort, 0);
  memset(&LED_Dev, 0, sizeof(struct LED_Dev_tag)); 
  LED_Dev.Mode=1;
 }
 u32 LED_Scan(void)
 {static u8 STEP=1;
//	static u8 TIME_S=0;
	u32 LED_Time;
	 
switch(STEP)
	 {
		 case 1:
     LED_Time=LED_OPEN_T;
		 if(LED_Dev.Mode&0x03)
		 {
		  LED_ctrl_handler[(LED_Dev.TIME_s/5)+1](); 
		   }
		 else if(LED_Dev.Mode&0x0c)
		 {
		  LED_ctrl_handler[(LED_Dev.TIME_s%12)+1](); 
		 }
		 else {
		  LED_ctrl_handler[13](); 
		 }
		 STEP++;
		 break;
		 case 2:
		 LED_Time=LED_CLOSE_T;

		 if(LED_Dev.Mode&0x05)
		 {
			 LED_ctrl_handler[0](); 
		 }
     LED_Dev.TIME_s++;
		 if(LED_Dev.TIME_s>=60)
		 {LED_Dev.TIME_s=0;
		  LED_Dev.TIME_min++;}
		 STEP--;
		 break;
}
	
return LED_Time;
 }
 
 /**
 ****************************************************************************************
 * LEDɨ�����

 ****************************************************************************************
 */
int app_led_ctrl_scan_handler(ke_msg_id_t const msgid, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{   uint32_t LED_Ctrl_Time;

    LED_Ctrl_Time=LED_Scan();
	  if(LED_Dev.TIME_min>=2)
	  {LED_Dev.TIME_min=0;
//		 ke_timer_clear(APP_LED_CTRL_SCAN,TASK_APP);
	  }
		else
		{
	  ke_timer_set(APP_LED_CTRL_SCAN,TASK_APP , LED_Ctrl_Time);
		}

    return KE_MSG_CONSUMED;
}
