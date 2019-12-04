/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
#include "app_wlist.h"               //APP白名单
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_batt.h"                // Battery Application Definitions
#include "app_ancsc.h"               // Application ancs Definition
#include "app_oads.h"                 // Application oads Definition
#include "app_fff0.h"                 // Application security Definition
#include "nvds.h"                    // NVDS Definitions
#include "app_sec.h"
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "led.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_DEVICE_NAME_LENGTH_MAX      (18)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef void (*appm_add_svc_func_t)(void);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_ANCSC,
	  APPM_SVC_FFF0,
    APPM_SVC_DIS,
    APPM_SVC_BATT,
	  APPM_SVC_OADS,
    APPM_SVC_LIST_STOP ,
};

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP = {NULL, &appm_default_handler,
                                                  appm_state, APPM_STATE_MAX, APP_IDX_MAX};

/// List of functions used to create the database
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] =
{
    (appm_add_svc_func_t)app_ancs_add_ancsc,
	  (appm_add_svc_func_t)app_fff0_add_fff0s,
    (appm_add_svc_func_t)app_dis_add_dis,
    (appm_add_svc_func_t)app_batt_add_bas,
	  (appm_add_svc_func_t)app_oad_add_oads,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure应用环境结构
struct app_env_tag app_env;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void appm_init()
{
    uint8_t key_len = KEY_LEN;

    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);//创建TASK_APP任务

    // Initialize Task state
    ke_state_set(TASK_APP, APPM_INIT);//初始化状态

    if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, app_env.loc_irk) != NVDS_OK)
    {
        uint8_t counter;

        // generate a new IRK
        for (counter = 0; counter < KEY_LEN; counter++)
        {
            app_env.loc_irk[counter]    = (uint8_t)co_rand_word();
        }

        // Store the generated value in NVDS将生成的值存储在NVDS中
        if (nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&app_env.loc_irk) != NVDS_OK)
        {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
    }

    /*------------------------------------------------------
     * INITIALIZE ALL MODULES
     *------------------------------------------------------*/
    // ANCS Module
	app_ancsc_init();
  app_fff0_init();	
    // Device Information Module
    app_dis_init();

    // Battery Module
    app_batt_init();

	app_sec_init();

	// Oad Module
    app_oads_init();	

		
}
//添加服务
bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}


/*设备主动断开连接函数*/
void appm_disconnect(void)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

    // Send the message
    ke_msg_send(cmd);
}

/**
 ****************************************************************************************
 * Advertising Functions
 ****************************************************************************************
 */


/* 设备发起定向广播函数*/
void appm_start_direct_dvertising(void)
{
#if BK_CONNNECT_FILTER_CTRL
	appm_start_advertising();
	return;
#endif

	// 检查广告程序是否已在进行中
    if (ke_state_get(TASK_APP) == APPM_READY)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            {
		struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

		cmd->op.addr_src    = GAPM_STATIC_ADDR;
		cmd->channel_map    = APP_ADV_CHMAP;


		cmd->intv_min = APP_ADV_FAST_INT;
		cmd->intv_max = APP_ADV_FAST_INT;


		cmd->op.code        = GAPM_ADV_UNDIRECT;
		
		cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

		 
		/*
		 * If the peripheral is already bonded with a central device, use the direct advertising
		 * procedure (BD Address of the peer device is stored in NVDS.
		 */
		if(app_sec_get_bond_status())
		{
			uint8_t bd_len = NVDS_LEN_PEER_BD_ADDRESS;
			cmd->op.code   = GAPM_ADV_DIRECT_LDC;
			
			if(nvds_get(NVDS_TAG_PEER_BD_ADDRESS, &bd_len,
	            		(uint8_t *)cmd->info.direct.addr.addr) != NVDS_OK)
			{
			    // An error has occurred during access to the NVDS
			    ASSERT_INFO(0,NVDS_TAG_PEER_BD_ADDRESS,bd_len);
			}

			ke_msg_send(cmd);

			#if (SYSTEM_SLEEP)
			//Start the advertising timer
			ke_timer_set(APP_ADV_TIMEOUT_TIMER, TASK_APP, APP_DFLT_ADV_DURATION);
			#endif

			#if !(SYSTEM_SLEEP)
			ke_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
			#endif

			wdt_enable(0x3fff);

			UART_PRINTF("appm start direct advertising\r\n");
		}
		else
		{
			ke_msg_free(ke_param2msg(cmd));
			appm_start_advertising();
			
			UART_PRINTF("appm start general advertising\r\n");
		}

	    ke_state_set(TASK_APP, APPM_ADVERTISING);	
	}		
}


uint8_t wlist_enable_flag = 0;

/* 设备发起白名单过滤广播*/
void appm_start_white_list_dvertising(void)
{   
    struct gap_bdaddr whitelist_bdaddr;
    uint8_t peer_address_len = NVDS_LEN_PEER_BD_ADDRESS;

    // Check if the advertising procedure is already is progress
    if (ke_state_get(TASK_APP) == APPM_READY)
    {      
        if(app_sec_get_bond_status())
        {
            UART_PRINTF("appm_start_white_list_dvertising\r\n");
            if(nvds_get(NVDS_TAG_PEER_BD_ADDRESS, &peer_address_len, (uint8_t *)&whitelist_bdaddr) != NVDS_OK)
            {
                // An error has occurred during access to the NVDS
                ASSERT_INFO(0,NVDS_TAG_PEER_BD_ADDRESS,peer_address_len);
            }
            
            appm_add_dev_to_wlist(whitelist_bdaddr);
            wlist_enable_flag = 1;
            appm_start_advertising();
        }
    }		
}
/* 设备发起广播函数*/
void appm_start_advertising(void)
{	
    // Check if the advertising procedure is already is progress
    if (ke_state_get(TASK_APP) == APPM_READY)
    {				
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;//静态地址
        cmd->channel_map    = APP_ADV_CHMAP;//广播频率通道
        cmd->intv_min 		= APP_ADV_INT_MIN;//最小间隔时间
        cmd->intv_max 		= APP_ADV_INT_MAX;	//最大间隔时间
        cmd->op.code        = GAPM_ADV_UNDIRECT;//启动无向可连接广告
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;//一般可发现模式

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = ADV_DATA_LEN;//广播数据长度
        cmd->info.host.scan_rsp_data_len  = SCAN_RSP_DATA_LEN;//响应数据长度

        // Advertising Data广播数据
        if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                    &cmd->info.host.adv_data[0]) != NVDS_OK)
        {
            //cmd->info.host.adv_data_len = 0;

            cmd->info.host.adv_data[0] = 2;// Length of ad type flags  AD类型标志长度
            cmd->info.host.adv_data[1] = GAP_AD_TYPE_FLAGS;//AD类型标志
            cmd->info.host.adv_data[2] = GAP_BR_EDR_NOT_SUPPORTED;//不支持传统BT-广告标志
            // set mode in ad_type
            switch(cmd->info.host.mode)
            {
                // General discoverable mode一般可发现模式
                case GAP_GEN_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_GEN_DISCOVERABLE_FLG;
                }
                break;
                // Limited discoverable mode有限可发现模式
                case GAP_LIM_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_LIM_DISCOVERABLE_FLG;
                }
                break;
                default: break; // do nothing
            }
            cmd->info.host.adv_data_len=3;
            //Add list of UUID and appearance				加载UUID		
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len],
                   APP_FFF0_ADV_DATA_UUID, APP_FFF0_ADV_DATA_UUID_LEN);
						
            cmd->info.host.adv_data_len += APP_FFF0_ADV_DATA_UUID_LEN;
        }

		//  Device Name Length
        uint8_t device_name_length;//设备名的长度
        uint8_t device_name_avail_space;//设备名的有效位
        uint8_t device_name_temp_buf[APP_DEVICE_NAME_LENGTH_MAX];//存放设备名的缓存

        // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
        device_name_avail_space = ADV_DATA_LEN  - cmd->info.host.adv_data_len - 2;

        // 检查数据是否可以添加到广告数据中
        if (device_name_avail_space > 2)/*设备名的有效位置*/
        {
						//加载设备名
            device_name_length = NVDS_LEN_DEVICE_NAME;
            if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length,
                         &device_name_temp_buf[0]) != NVDS_OK)
            {
                device_name_length = strlen(APP_DFLT_DEVICE_NAME);
                // Get default Device Name (No name if not enough space)
                memcpy(&device_name_temp_buf[0], APP_DFLT_DEVICE_NAME, device_name_length);
            }
							
	     	if(device_name_length > 0)
            {
                // Check available space
                device_name_length = co_min(device_name_length, device_name_avail_space);
                cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
								 
                // Fill Device Name Flag 设备名标志
                cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';
                // Copy device name
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2],
                device_name_temp_buf, device_name_length);

                // Update Advertising Data Length
                cmd->info.host.adv_data_len += (device_name_length + 2);					
            }
          
        }

        // 扫描响应数据
        if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                    &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
        {
            cmd->info.host.scan_rsp_data_len = 0;

			memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len],
                   APP_SCNRSP_DATA, APP_SCNRSP_DATA_LEN);
            cmd->info.host.scan_rsp_data_len += APP_SCNRSP_DATA_LEN;
        }

        // Send the message
        ke_msg_send(cmd);
	 	UART_PRINTF("appm start advertising\r\n");

		wdt_enable(0x3fff);
		ke_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
        // Set the state of the task to APPM_ADVERTISING
        ke_state_set(TASK_APP, APPM_ADVERTISING);//设置成广播状态	

    }
//	  ke_timer_set(APP_LED_CTRL_SCAN,TASK_APP,10);
		ke_msg_send_basic(APP_LED_CTRL_SCAN,TASK_APP,TASK_APP);//开始扫描LED
		ke_msg_send_basic(USER_APP_CALENDAR_UPDATE, TASK_APP,TASK_APP);//开始更新日历数据
    // else ignore the request
}


/* 设备主动停止广播函数*/
void appm_stop_advertising(void)
{
    if (ke_state_get(TASK_APP) == APPM_ADVERTISING)
    {
        // Go in ready state
        ke_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        ke_msg_send(cmd);

		wdt_disable_flag = 1;
    }
    // else ignore the request
}



void appm_update_adv_data( uint8_t* adv_buff, uint8_t adv_len, uint8_t* scan_buff, uint8_t scan_len)
{
	if (ke_state_get(TASK_APP) == APPM_ADVERTISING 
                 && (adv_len <= ADV_DATA_LEN) && (scan_len <= ADV_DATA_LEN))
	{
		struct gapm_update_advertise_data_cmd *cmd =  KE_MSG_ALLOC(
            		            GAPM_UPDATE_ADVERTISE_DATA_CMD,
            		            TASK_GAPM,
            		            TASK_APP,
            		            gapm_update_advertise_data_cmd);

		cmd->operation = GAPM_UPDATE_ADVERTISE_DATA;
		cmd->adv_data_len = adv_len;
		cmd->scan_rsp_data_len = scan_len;

		//memcpy
		memcpy(&cmd->adv_data[0], adv_buff, adv_len);
		memcpy(&cmd->scan_rsp_data[0], scan_buff, scan_len);
        
		// Send the message
		ke_msg_send(cmd);
	}
}

//APP更新连接参数

void appm_update_param(struct gapc_conn_param *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

    cmd->operation  = GAPC_UPDATE_PARAMS;
    cmd->intv_min   = conn_param->intv_min;
    cmd->intv_max   = conn_param->intv_max;
    cmd->latency    = conn_param->latency;
    cmd->time_out   = conn_param->time_out;

    // not used by a slave device不被从设备使用
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
		
    UART_PRINTF("intv_min = %d,intv_max = %d,latency = %d,time_out = %d\r\n",
		cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
	
    // Send the message
    ke_msg_send(cmd);
}
//获取连接信号强度
void appm_get_conn_rssi(void)
{
	// connection index has been put in addr_src
//	struct gapc_get_info_cmd* info_cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
//	        KE_BUILD_ID(TASK_GAPC,0), TASK_GAPM,
//	        gapc_get_info_cmd);
	struct gapc_get_info_cmd* info_cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
	        KE_BUILD_ID(TASK_GAPC,0), TASK_GAPM,
	        gapc_get_info_cmd);	
	// request peer device name.
	info_cmd->operation = GAPC_GET_CON_RSSI;

	// send command
	ke_msg_send(info_cmd);
}
//*获取设备名
uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
//	UART_PRINTF("appm_get_dev_name = %s\r\n",app_env.dev_name);
    // return name length
    return app_env.dev_name_len;
}
//*发送安全请求
void appm_send_seurity_req(void)
{
    app_sec_send_security_req(app_env.conidx);
}

#endif //(BLE_APP_PRESENT)

/// @} APP


