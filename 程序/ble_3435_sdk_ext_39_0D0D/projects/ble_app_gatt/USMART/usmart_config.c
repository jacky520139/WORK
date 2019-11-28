

////////////////////////////用户配置区///////////////////////////////////////////////
//这下面要包含所用到的函数所申明的头文件(用户自己添加) 
//#include "delay.h"	

#include "ALL_Includes.h"
#include "app.h"                     // Application Definition
#include "timer.h"        // timer definition
#include "app_batt.h" 
extern u16 gui_color_chg(u32 rgb);
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[]=
{
(void*)Init_LED,"void Init_LED(void)",
(void*)BlueLed,"void BlueLed(void)",
(void*)appm_start_advertising,"void appm_start_advertising(void)",
(void*)appm_stop_advertising,"void appm_stop_advertising(void)",
(void*)appm_disconnect,"void appm_disconnect(void)",
(void*)appm_get_dev_name,"uint8_t appm_get_dev_name(uint8_t* name)",
(void*)timer_set_timeout,"void timer_set_timeout(uint32_t to)",
(void*)timer_enable,"void timer_enable(bool enable)",
(void*)timer_get_time,"uint32_t timer_get_time(void)",
(void*)app_batt_send_lvl,"void app_batt_send_lvl(uint8_t batt_lvl)",
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
};   



















