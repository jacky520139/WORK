#include "ALL_Includes.h"
#include "app_task.h"                // application task definitions
#include "icu.h" 

//��ʱ���ص�����
void user_timer_cb(unsigned char ucChannel)
{
// static uint32_t cnt=0;
//	cnt++;
//	if(cnt>1000)
//	{
//	 pwm_disable(ucChannel);
//	 icu_set_sleep_mode(0);//����ϵͳ�����ѹ����
////	 rwip_prevent_sleep_clear(BK_DRIVER_TIMER_ACTIVE);//�����־λ
//	 
//	}
//	UART_PRINTF("0x%x ", 33);
	usmart_dev.scan();	//ִ��usmartɨ��	
}
void user_timer_init(void)
{
//  rwip_prevent_sleep_set(BK_DRIVER_TIMER_ACTIVE);
  PWM_DRV_DESC timer_desc;
	timer_desc.channel=1;
	timer_desc.mode=1<<0|1<<1|1<<2|0<<4;//����ʱ��Ϊ32.768k
  timer_desc.end_value=600;//��ʱ��ʱ��
	timer_desc.duty_cycle=0;
	timer_desc.p_Int_Handler=user_timer_cb;//ע���жϻص�����
	pwm_init(&timer_desc);
//  pwm_enable(1);
}





