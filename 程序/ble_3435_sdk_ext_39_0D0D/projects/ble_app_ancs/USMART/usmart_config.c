

////////////////////////////�û�������///////////////////////////////////////////////
//������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
//#include "delay.h"	

#include "ALL_Includes.h"
#include "app.h"                     // Application Definition
#include "timer.h"        // timer definition
#include "app_batt.h" 
#include "lld_evt.h" 
#include "ke_timer.h" 
extern u16 gui_color_chg(u32 rgb);
//�������б��ʼ��(�û��Լ����)
//�û�ֱ������������Ҫִ�еĺ�����������Ҵ�
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
(uint32_t*)lld_evt_time_get,"uint32_t lld_evt_time_get(void)",
//(int*)appm_msg_handler,"static int appm_msg_handler(ke_msg_id_t const msgid,void *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)",
(void*)ke_timer_set,"void ke_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task, uint32_t delay)",
(void*)ke_timer_clear,"void ke_timer_clear(ke_msg_id_t const timerid, ke_task_id_t const task)",
(void*)ke_timer_active,"bool ke_timer_active(ke_msg_id_t const timer_id, ke_task_id_t const task_id)"


};

///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�������ƹ�������ʼ��
//�õ������ܿغ���������
//�õ�����������
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//��������
	0,	  	//��������
	0,	 	//����ID
	1,		//������ʾ����,0,10����;1,16����
	0,		//��������.bitx:,0,����;1,�ַ���	    
	0,	  	//ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		//�����Ĳ���,��ҪPARM_LEN��0��ʼ��
	1,		//��ʾ����ʱ��
};   



















