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

void Init_LED(void)
{ PWM_DRV_DESC PWM0;
		RTC_DATE_DESC RTC_DATE;


	PWM0.channel=0;
	PWM0.mode=0X11;
	PWM0.end_value=0XFFFF;
	PWM0.duty_cycle=100;
	pwm_init(&PWM0);
  pwm_enable(0);
	  adc_init(1,1);//初始化ADC,软件模式
		GLOBAL_INT_START();//使能全局中断
	//初始化LED
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
	gpio_config(ButtonPort, INPUT, PULL_HIGH);
BlueLedToggle();


}





