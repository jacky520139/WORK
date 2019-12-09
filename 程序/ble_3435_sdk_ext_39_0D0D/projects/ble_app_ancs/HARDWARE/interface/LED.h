#ifndef _LED_H
#define _LED_H
#include "ALL_Includes.h" 
//#define LED_PWM_DRIVE    1



#ifdef LED_PWM_DRIVE
#define	Get1		0x11
#define	Set_Get1()		pwm_enable(1);
#define	RES_Get1()		{pwm_disable(1);}

#define	Get2		0x12
#define	Set_Get2()		pwm_enable(2);
#define	RES_Get2()		{pwm_disable(2);}
#define	Get3		0x13
#define	Set_Get3()		pwm_enable(3);
#define	RES_Get3()		{pwm_disable(3);}
#else
#define	Get1		0x11
#define	Set_Get1()		REG_APB5_GPIOB_DATA |= 0x02
#define	RES_Get1()		REG_APB5_GPIOB_DATA &= (~0x02)
#define	Get1Toggle()	REG_APB5_GPIOB_DATA ^= 0x02
#define	Get2		0x12
#define	Set_Get2()		REG_APB5_GPIOB_DATA |= 0x04
#define	RES_Get2()		REG_APB5_GPIOB_DATA &= (~0x04)
#define	Get2Toggle()	REG_APB5_GPIOB_DATA ^= 0x04
#define	Get3		0x13
#define	Set_Get3()		REG_APB5_GPIOB_DATA |= 0x08
#define	RES_Get3()		REG_APB5_GPIOB_DATA &= (~0x08)
#define	Get3Toggle()	REG_APB5_GPIOB_DATA ^= 0x08
#endif

#define	COM1		0x33
#define	Set_COM1()		REG_APB5_GPIOD_DATA |= 0x08
#define	RES_COM1()		REG_APB5_GPIOD_DATA &= (~0x08)
#define	COM1Toggle()	REG_APB5_GPIOD_DATA ^= 0x08
#define	COM2		0x07
#define	Set_COM2()		REG_APB5_GPIOA_DATA |= 0x80
#define	RES_COM2()		REG_APB5_GPIOA_DATA &= (~0x80)
#define	COM2Toggle()	REG_APB5_GPIOA_DATA ^= 0x80
#define	COM3		0x06
#define	Set_COM3()		REG_APB5_GPIOA_DATA |= 0x40
#define	RES_COM3()		REG_APB5_GPIOA_DATA &= (~0x40)
#define	COM3Toggle()	REG_APB5_GPIOA_DATA ^= 0x40
#define	COM4		0x05
#define	Set_COM4()		REG_APB5_GPIOA_DATA |= 0x20
#define	RES_COM4()		REG_APB5_GPIOA_DATA &= (~0x20)
#define	COM4Toggle()	REG_APB5_GPIOA_DATA ^= 0x20


struct LED_Dev_tag
{
  u8 Mode;//模式
	u8 Strength;//亮度
	u8 TIME_s;//运行的时间，秒
	u8 TIME_min;//运行的时间，分钟
};
extern struct LED_Dev_tag LED_Dev;
void Init_LED(void);
void BlueLed(void);
u32 LED_Scan(void);
u8 GET_LED_State(void);
int app_led_ctrl_scan_handler(ke_msg_id_t const msgid,void const *param,ke_task_id_t const dest_id, ke_task_id_t const src_id);
int ble_printf(const char *fmt,...);
void ble_printf_text(void);


#endif




