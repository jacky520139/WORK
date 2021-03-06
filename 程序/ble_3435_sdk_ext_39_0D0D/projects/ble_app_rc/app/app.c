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
#include "app_wlist.h"
#include "app_sec.h"                 // Application security Definition
#include "app_dis.h"                 // Device Information Service Application Definitions
#include "app_batt.h"                // Battery Application Definitions
#include "app_hid.h"                 // HID Application Definitions
#include "app_oads.h"                 // Application oads Definition
#include "nvds.h"                    // NVDS Definitions
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "wdt.h"
#include "gpio.h"




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


static remote_ctrl_struct target_remote = 
{
	CONN_MODE_LONG,
	0xffff,
};

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_DIS,
    APPM_SVC_BATT,
    APPM_SVC_HIDS,
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
    (appm_add_svc_func_t)app_dis_add_dis,
    (appm_add_svc_func_t)app_batt_add_bas,
    (appm_add_svc_func_t)app_hid_add_hids,
	(appm_add_svc_func_t)app_oad_add_oads,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void appm_init()
{	
    
    uint8_t peer_irk_len = NVDS_LEN_PEER_IRK;
    uint8_t key_len = KEY_LEN;
    


    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APPM_INIT);

	app_env.dev_name_len = sizeof(APP_HID_DEVICE_NAME);  ///190409
	memcpy(&app_env.dev_name[0], APP_HID_DEVICE_NAME, sizeof(APP_HID_DEVICE_NAME));

    if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, app_env.loc_irk) != NVDS_OK)
    {
        uint8_t counter;

        // generate a new IRK
        for (counter = 0; counter < KEY_LEN; counter++)
        {
            app_env.loc_irk[counter]    = (uint8_t)co_rand_word();
        }

        // Store the generated value in NVDS
        if (nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&app_env.loc_irk) != NVDS_OK)
        {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
    }
    // Store peer identity in NVDS
    if (nvds_get(NVDS_TAG_PEER_IRK, &peer_irk_len, (uint8_t *)&app_env.peer_irk.irk.key) != NVDS_OK)
    {
    	UART_PRINTF("not NVDS_TAG_PEER_IRK\r\n");				 
    }
    else
    {
    	UART_PRINTF("appm irk.key = ");
    	for(int i = 0;i<sizeof(struct gap_sec_key);i++)
    	{
    		UART_PRINTF("0x%x ",app_env.peer_irk.irk.key[i]);
    	}
    	UART_PRINTF("\r\n");
    				
    	UART_PRINTF("appm addr type = %x\r\n",app_env.peer_irk.addr.addr_type);
    	UART_PRINTF("appm addr.addr = ");
    	for(int i = 0;i<sizeof(struct bd_addr);i++)
    	{
    		UART_PRINTF("0x%x ", app_env.peer_irk.addr.addr.addr[i]);
    	}	
    	UART_PRINTF("\r\n");
    }

					 			
    /*------------------------------------------------------
     * INITIALIZE ALL MODULES
     *------------------------------------------------------*/
    // Security Module
    app_sec_init();

    // Device Information Module
    app_dis_init();

    // HID Module
    app_hid_init();

    // Battery Module
    app_batt_init();
			
    app_oads_init();
		
}

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

	// Check if the advertising procedure is already is progress
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
UART_PRINTF("%s \r\n",__func__);
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
    UART_PRINTF("%s \r\n",__func__);
        

#if !(SYSTEM_SLEEP)
    ke_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
#endif

	
    // Check if the advertising procedure is already is progress
    if (ke_state_get(TASK_APP) == APPM_READY)
    {				
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;
        cmd->intv_min 		= APP_ADV_INT_MIN;
        cmd->intv_max 		= APP_ADV_INT_MAX;	
        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

#if (APP_WHITE_LIST_ENABLE)
        if(wlist_enable_flag)
        {
            wlist_enable_flag=0;
            cmd->info.host.adv_filt_policy=ADV_ALLOW_SCAN_WLST_CON_WLST;
        }
#endif

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = ADV_DATA_LEN;
        cmd->info.host.scan_rsp_data_len  = SCAN_RSP_DATA_LEN;

        // Advertising Data
        if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                    &cmd->info.host.adv_data[0]) != NVDS_OK)
        {
            cmd->info.host.adv_data[0] = 2;// Length of ad type flags
            cmd->info.host.adv_data[1] = GAP_AD_TYPE_FLAGS;
            cmd->info.host.adv_data[2] = GAP_BR_EDR_NOT_SUPPORTED;
            // set mode in ad_type
            switch(cmd->info.host.mode)
            {
                // General discoverable mode
                case GAP_GEN_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_GEN_DISCOVERABLE_FLG;
                }
                break;
                // Limited discoverable mode
                case GAP_LIM_DISCOVERABLE:
                {
                    cmd->info.host.adv_data[2] |= GAP_LE_LIM_DISCOVERABLE_FLG;
                }
                break;
                default: break; // do nothing
            }
            cmd->info.host.adv_data_len = 3;
           
            //Add list of UUID and appearance
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len],
                   APP_HID_ADV_DATA_UUID, APP_HID_ADV_DATA_UUID_LEN);
            cmd->info.host.adv_data_len += APP_HID_ADV_DATA_UUID_LEN;
			
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len],
                   APP_HID_ADV_DATA_APPEARANCE, APP_ADV_DATA_APPEARANCE_LEN);
            cmd->info.host.adv_data_len += APP_ADV_DATA_APPEARANCE_LEN;
        }

		
        //  Device Name Length
        uint8_t device_name_length;
        uint8_t device_name_avail_space;
        uint8_t device_name_temp_buf[APP_DEVICE_NAME_LENGTH_MAX];


        // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
        device_name_avail_space = ADV_DATA_LEN  - cmd->info.host.adv_data_len - 2;

        // Check if data can be added to the Advertising data
        if (device_name_avail_space > 2)
        {
            device_name_length = NVDS_LEN_DEVICE_NAME;
            if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length,
                         &device_name_temp_buf[0]) != NVDS_OK)
            {
                device_name_length = strlen(APP_HID_DEVICE_NAME);
                // Get default Device Name (No name if not enough space)
                memcpy(&device_name_temp_buf[0], APP_HID_DEVICE_NAME, device_name_length);
            }
							
	     	if(device_name_length > 0)
            {
                // Check available space
                device_name_length = co_min(device_name_length, device_name_avail_space);
                cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
								 
                // Fill Device Name Flag
                cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';
                // Copy device name
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2],
                device_name_temp_buf, device_name_length);

                // Update Advertising Data Length
                cmd->info.host.adv_data_len += (device_name_length + 2);
            }
          
        }

        // Scan Response Data
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

	#if (SYSTEM_SLEEP)
		//Start the advertising timer
    	ke_timer_set(APP_ADV_TIMEOUT_TIMER, TASK_APP, APP_DFLT_ADV_DURATION);
	#endif

		wdt_enable(0x3fff);
		
        // Set the state.
        ke_state_set(TASK_APP, APPM_ADVERTISING);	
    }
	
    // else ignore the request
}


/* 设备主动停止广播函数*/
void appm_stop_advertising(void)
{
    if (ke_state_get(TASK_APP) == APPM_ADVERTISING)
    {
        // Stop the advertising timer if needed
        if (ke_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
        {
            ke_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
        }

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

    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
		
    UART_PRINTF("intv_min = %d,intv_max = %d,latency = %d,time_out = %d\r\n",
		cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
	
    // Send the message
    ke_msg_send(cmd);
}


uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
    // return name length
    return app_env.dev_name_len;
}


void appm_switch_general_adv(void)
{
	app_sec_remove_bond();
	appm_stop_advertising();	
}


void appm_send_seurity_req(void)
{
	UART_PRINTF("%s \r\n",__func__);
    app_sec_send_security_req(app_env.conidx);
}


#endif //(BLE_APP_PRESENT)


/*******************************************************************************
 * Function: app_set_mode
 * Description: app set remote ctrl connect mode.
 * Input: remote_conn_mode CONN_MODE_LONG
 * 				            CONN_MODE_SHORT
 *           uint32_t timeout
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
void app_set_mode(remote_conn_mode mode, uint32_t timeout)
{
 	if(mode == CONN_MODE_LONG)
	{
		target_remote.target_mode = CONN_MODE_LONG;
		target_remote.target_timeout = 0;
	}
	else
	{
		target_remote.target_mode = CONN_MODE_SHORT;
		target_remote.target_timeout = timeout;
	}
}

remote_ctrl_struct app_get_mode(void)
{
	return (target_remote);
}


//Defines an array to store audio data that has been encoded
uint8_t encode_voice_data[TOTAL_BLOCK_NUM][20];
//reaord index
uint8_t pbuff_write = 0;
//read index
uint8_t pbuff_read = 0;


 
 /*******************************************************************************
 * Function: read_encode_data
 * Description: read encoded data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
uint8_t  read_encode_data(uint8_t *buf)
{
	//Read 20 encode data from loop buffer to designated buffer 
	if(pbuff_write != pbuff_read)
	{
		memcpy(buf,encode_voice_data[pbuff_read],20);
		//Update the buffer index of the data 
		//(in fact, the index is between 0-79)
		pbuff_read = ((pbuff_read + 1 )% TOTAL_BLOCK_NUM);
		
		//uart_printf("read 0x%x\r\n",pbuff_read);
		return 1;
	}else
	{
		//uart_printf("buff empty!!0x%x\r\n",pbuff_read);
		return 0;
	} 
}


/*******************************************************************************
 * Function: store_encode_data
 * Description: store encoded data into loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
uint8_t store_encode_data(uint8_t *buf)
{
	uint8_t free_cnt;
	uint8_t status ;
	
	//Calculates the number of empty buffer in the circular queue (the 
	//data stored in this queue is encoded)
	if(pbuff_write >= pbuff_read)
	{
		free_cnt = (TOTAL_BLOCK_NUM - pbuff_write + pbuff_read);
	}else
	{
		free_cnt = pbuff_read - pbuff_write;
	}
	//UART_PRINTF("free cnt: %d\r\n", free_cnt);
	
	//If there are at least two empty buffer in the loop queue, the current 
	//encoded data will be stored in buffer. 
	if(free_cnt >= 2) 
	{
		memcpy(encode_voice_data[pbuff_write],buf,20);
		
		//Update the buffer index of the data 
		//(in fact, the index is between 0-79)
		pbuff_write = ((pbuff_write + 1 )% TOTAL_BLOCK_NUM);
		status = 1;
		//UART_PRINTF("buff write 0x%x,0x%x!!!\r\n",buf[0],pbuff_write);
	}else
	{
		app_hid_env.encode_no_send_cnt++;
		UART_PRINTF("buff full %d,,0x%x!!!\r\n",app_hid_env.encode_no_send_cnt,pbuff_write); // for test show
		status = 0;
	}
		
	return status;
}



/*******************************************************************************
 * Function: print_debug_info
 * Description: print debug info to UART
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/
void print_debug_info(void) //for test  debug
{
	UART_PRINTF("pbuff_write = %d,pbuff_read = %d\r\n",pbuff_write,pbuff_read);
} 


/*******************************************************************************
 * Function: encode_voice_init
 * Description:  encode variable init
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void encode_voice_init(void) // for test debug
{
	UART_PRINTF("pbuff_write = %d,pbuff_read = %d\r\n",pbuff_write,pbuff_read);
	pbuff_read = 0;
	pbuff_write = 0;
	app_hid_env.encode_cnt = 0;
	app_hid_env.encode_no_send_cnt = 0;
	app_hid_env.encode_send_cnt = 0;
	app_hid_env.encode_tx_cnt = 0;
} 


/*******************************************************************************
 * Function: app_send_encode_evt
 * Description:  send event to app_task and start encode.
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void app_send_encode_evt(void)
{
	ke_msg_send_basic(APP_ENCODE_IND,TASK_APP,TASK_APP);
}


/*******************************************************************************
 * Function: print_debug_status
 * Description:  
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void print_debug_status(void)
{
	uint8_t buf_cnt;
	
	buf_cnt = (pbuff_read > pbuff_write) ? (TOTAL_BLOCK_NUM - pbuff_read + pbuff_write) 
			: (pbuff_write - pbuff_read);
	UART_PRINTF("total encode cnt %d,send %d,loss %d\r\n",
		app_hid_env.encode_cnt,app_hid_env.encode_send_cnt + buf_cnt,
		app_hid_env.encode_cnt - app_hid_env.encode_send_cnt - buf_cnt);
}

/// @} APP


