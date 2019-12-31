

////////////////////////////用户配置区///////////////////////////////////////////////
//这下面要包含所用到的函数所申明的头文件(用户自己添加) 
//#include "delay.h"	

#include "ALL_Includes.h"
#include "app.h"                     // Application Definition
#include "timer.h"        // timer definition
#include "app_batt.h" 
#include "lld_evt.h" 
#include "ke_timer.h" 
#include "flash.h"         // flash definition
#include "SL_SC7A21_Application.h"
//extern u16 gui_color_chg(u32 rgb);
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
extern void Get_Loc_Mac(void);
extern void Push_Step(u8 mon,u8 day);
extern void flash_Write_Step(u8 mon,u8 day,u8 hour,u16 step);
extern void Upload_Step_Data(u32 stata);
extern void flash_Write_Step(u8 mon,u8 day,u8 hour,u16 step);//从FLASH中写步数数据
extern u16 flash_Read_hour_Step(u8 mon,u8 day,u8 hour);//从FLASH中读出某天步数数据
extern void appm_start_white_list_dvertising(void);
struct _m_usmart_nametab usmart_nametab[]=
{
//(void*)Init_LED,"void Init_LED(void)",
//(void*)BlueLed,"void BlueLed(void)",
(void*)appm_start_advertising,"void appm_start_advertising(void)",
(void*)appm_stop_advertising,"void appm_stop_advertising(void)",
//(void*)appm_disconnect,"void appm_disconnect(void)",
//(void*)appm_get_dev_name,"uint8_t appm_get_dev_name(uint8_t* name)",
//(void*)timer_set_timeout,"void timer_set_timeout(uint32_t to)",
//(void*)timer_enable,"void timer_enable(bool enable)",
//(void*)timer_get_time,"uint32_t timer_get_time(void)",
(void*)app_batt_send_lvl,"void app_batt_send_lvl(uint8_t batt_lvl)",
//(uint32_t*)lld_evt_time_get,"uint32_t lld_evt_time_get(void)",
////(int*)appm_msg_handler,"static int appm_msg_handler(ke_msg_id_t const msgid,void *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)",
//(void*)ke_timer_set,"void ke_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task, uint32_t delay)",
//(void*)ke_timer_clear,"void ke_timer_clear(ke_msg_id_t const timerid, ke_task_id_t const task)",
//(void*)ke_timer_active,"bool ke_timer_active(ke_msg_id_t const timer_id, ke_task_id_t const task_id)",
//(void*)flash_erase,"uint8_t flash_erase(uint8_t flash_type, uint32_t offset, uint32_t size, void (*callback)(void))",
//(void*)flash_write,"uint8_t flash_write(uint8_t flash_type, uint32_t offset, uint32_t length, uint8_t *buffer, void (*callback)(void))",
//(void*)flash_read, "uint8_t flash_read(uint8_t flash_type, uint32_t offset, uint32_t length, uint8_t *buffer, void (*callback)(void))",
//(void*)flash_test,"void flash_test(void)",
//(void*)ble_printf,"int ble_printf(const char *fmt,...)",
(void*)ble_printf_text,"void ble_printf_text(void)",
(void*)Calendar_Seting,	"u8 Calendar_Seting(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec)",
//(void*)PEDO_Get,	"u16 PEDO_Get(u8 day,u8 hour)",
//(void*)PEDO_Set,	"u16 PEDO_Set(u8 day,u8 hour,u16 cnt)",
(void*)Motor_Text,	"void Motor_Text(u8 cnt,u8 Tog_time,u8 Tog_Interval_Time,u8 Strength)",
(void*)Motor_Strength,	"void Motor_Strength(u8 Strength)",
(void*)pwm_duty_cycle,"void pwm_duty_cycle(unsigned char channel,unsigned short duty_cycle)",	
(void*)user_batt_send_lvl_handler,"u8 user_batt_send_lvl_handler(void)",
(void*)SL_SC7A21_Online_Test,"signed char  SL_SC7A21_Online_Test(unsigned char Sl_pull_up_mode)",
(void*)SL_SC7A21_Read_XYZ_Data,"	signed char  SL_SC7A21_Read_XYZ_Data(signed short *SL_SC7A21_Data_XYZ_Buf)",
(void*)user_Motor_handler,"void user_Motor_handler(u8 operation)",	
(void*)LED_Mode,"void LED_Mode(u8 mode)",	
(void*)LED_State,"void LED_State(void)",
(void*)Text_Alarm,"void Text_Alarm(u8 mode,u8 hour,u8 min,u8 sec)",	
(void*)Alarm_Get,"void Alarm_Get(u8 Class)",
(void*)Alarm_date_Get,"void Alarm_date_Get(u8 Class)",		
(void*)User_Anti_ON,"void User_Anti_ON(void)",
(void*)User_Anti_OFF,"void User_Anti_OFF(void)",
(void*)Get_Peer_Mac,"void Get_Peer_Mac(void)",
(void*)flash_Close_alarm,"void flash_Close_alarm(u8 Class)",
(void*)Get_Loc_Mac,"void Get_Loc_Mac(void)",
	(void*)Push_Step,"void Push_Step(u8 mon,u8 day)",
	
		(void*)flash_Write_Step,"void flash_Write_Step(u8 mon,u8 day,u8 hour,u16 step)",
(void*)Upload_Step_Data,"void Upload_Step_Data(u32 stata)",	
(void*)flash_Write_Step,"void flash_Write_Step(u8 mon,u8 day,u8 hour,u16 step)",	
	(void*)flash_Read_hour_Step,"u16 flash_Read_hour_Step(u8 mon,u8 day,u8 hour)",
(void*)appm_start_white_list_dvertising,"void list_dvertising(void)",			
	
};

///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//函数控制管理器初始化
//得到各个受控函数的名字
//得到函数总数量
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//函数数量
	0,	  	//参数数量
	0,	 	//函数ID
	1,		//参数显示类型,0,10进制;1,16进制
	0,		//参数类型.bitx:,0,数字;1,字符串	    
	0,	  	//每个参数的长度暂存表,需要MAX_PARM个0初始化
	0,		//函数的参数,需要PARM_LEN个0初始化
	1,		//显示运行时间
};   



















