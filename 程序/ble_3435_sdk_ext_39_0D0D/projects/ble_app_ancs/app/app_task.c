/**
 ****************************************************************************************
 *
 * @file appm_task.c
 *
 * @brief RW APP Task implementation
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"          // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
#include "app_task.h"              // Application Manager Task API
#include "app.h"                      // Application Manager Definition
#include "gapc_task.h"            // GAP Controller Task API
#include "gapm_task.h"          // GAP Manager Task API
#include "gattc_task.h"
#include "arch.h"                    // Platform Definitions
#include "app_fff0.h"                 // Application security Definition
#include "ke_timer.h"             // Kernel timer
#include "app_dis.h"              // Device Information Module Definition
#include "app_wlist.h"
#include "diss_task.h"
#include "app_batt.h"             // Battery Module Definition
#include "bass_task.h"
#include "app_oads.h"             
#include "oads_task.h"              
#include "gpio.h"
#include "audio.h"
#include "uart.h"
#include "BK3435_reg.h"
#include "icu.h"
#include "reg_ble_em_cs.h"
#include "lld.h"
#include "app_ancsc.h"
#include "ancsc_task.h"
#include "app_sec.h"              // Security Module Definition
#include "wdt.h"
#include "rc32k_cal.h"
#include "LED.h"
#include "ALL_Includes.h"
/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
 //this flag use indicate adv timeout
static bool adv_timeout_flag = false;
bool Pairing_Flag=0;//配对状态
static uint8_t appm_get_handler(const struct ke_state_handler *handler_list,
                                ke_msg_id_t msgid,
                                void *param,
                                ke_task_id_t src_id)
{
    // Counter
    uint8_t counter;

    // Get the message handler function by parsing the message table
    for (counter = handler_list->msg_cnt; 0 < counter; counter--)
    {
			
        struct ke_msg_handler handler = (*(handler_list->msg_table + counter - 1));
			
        if ((handler.id == msgid) ||
            (handler.id == KE_MSG_DEFAULT_HANDLER))
        {
            // If handler is NULL, message should not have been received in this state
            ASSERT_ERR(handler.func);

            return (uint8_t)(handler.func(msgid, param, TASK_APP, src_id));
        }
    }

    // If we are here no handler has been found, drop the message
    return (KE_MSG_CONSUMED);
}

/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP. - Reset the stack
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
//设备已经就绪
 ****************************************************************************************
 */
static int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    // Application has not been initialized
    ASSERT_ERR(ke_state_get(dest_id) == APPM_INIT);

    // Reset the stack
    struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                              TASK_GAPM, TASK_APP,
                                              gapm_reset_cmd);

    cmd->operation = GAPM_RESET;//复位GAPM

    ke_msg_send(cmd);

    return (KE_MSG_CONSUMED);
}
//static int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
//                                         void const *param,
//                                         ke_task_id_t const dest_id,
//                                         ke_task_id_t const src_id)
//int Get_Peer_Name(void)
//{
//    // Application has not been initialized
////    ASSERT_ERR(ke_state_get(dest_id) == APPM_INIT);

//    // Reset the stack
//    struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_PEER_NAME_IND,
//                                              TASK_GAPM, TASK_APP,
//                                              gapm_reset_cmd);

//    cmd->operation = GAPM_PEER_NAME_IND;//复位GAPM

//    ke_msg_send(cmd);

//    return (KE_MSG_CONSUMED);
//}
/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
//广播超时处理事件
 ****************************************************************************************
 */
static int app_adv_timeout_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
//	UART_PRINTF("%s\r\n", __func__);

	adv_timeout_flag = true;

//	// Stop advertising
//	appm_stop_advertising();

////	key_wakeup_config();
	wdt_disable_flag=1;//关闭看门狗
	return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("param->operation = 0x%x, param->status = 0x%x \r\n", param->operation, param->status);
    switch(param->operation)
    {
        // Reset completed
        case (GAPM_RESET)://复位成功
        {
            if(param->status == GAP_ERR_NO_ERROR)
            {
                // Set Device configuration设置设备配置信息
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
	                                                                   TASK_GAPM, TASK_APP,
                                                                   gapm_set_dev_config_cmd);
                // Set the operation
                cmd->operation = GAPM_SET_DEV_CONFIG;
                // Set the device role - Peripheral设置蓝牙设备角色
                cmd->role      = GAP_ROLE_PERIPHERAL;
                // Set Data length parameters 设置数据长度参数
                cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;//8位
                cmd->sugg_max_tx_time   = BLE_MIN_TIME;
								
		 		       cmd->max_mtu = 131;//BLE_MIN_OCTETS;MTU=131
//								 cmd->max_mtu = 231;//BLE_MIN_OCTETS;MTU=131
                //Do not support secure connections 不支持安全连接(设置配对模式)
                cmd->pairing_mode = GAPM_PAIRING_LEGACY;//传统配对模式
//								 cmd->pairing_mode = GAPM_PAIRING_SEC_CON;
                
 				//cmd->addr_type   = GAPM_CFG_ADDR_HOST_PRIVACY; //2017-10-24 by alen
                // load IRK
                memcpy(cmd->irk.key, app_env.loc_irk, KEY_LEN);//拷贝本地IRK

				        app_env.next_svc = 0;
                // Send message
                ke_msg_send(cmd);
            }
            else
            {
                ASSERT_ERR(0);
            }
        }
        break;
        case (GAPM_PROFILE_TASK_ADD)://加载服务
        {
            // Add the next requested service
            if (!appm_add_svc())
            {                       //服务加载完成后开始广播
                // Go to the ready state
                ke_state_set(TASK_APP, APPM_READY);
							  ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
//								ke_msg_send_basic(APP_LED_CTRL_SCAN,TASK_APP,TASK_APP);//LED闪烁扫描
		            USER_PERIOD_TIMER_Conf();//配置用户周期性消息任务
//				         appm_start_advertising();
							// No more service to add, start advertising
            }
        }
        break;
        // Device Configuration updated
        case (GAPM_SET_DEV_CONFIG):
        {
            ASSERT_INFO(param->status == GAP_ERR_NO_ERROR, param->operation, param->status);

            // Go to the create db state
            ke_state_set(TASK_APP, APPM_CREATE_DB);

            // Add the first required service in the database
            // and wait for the PROFILE_ADDED_IND
            appm_add_svc();
        }
        break;	

        case (GAPM_ADV_NON_CONN):
        case (GAPM_ADV_UNDIRECT):
        case (GAPM_ADV_DIRECT):
		    case (GAPM_UPDATE_ADVERTISE_DATA):
        case (GAPM_ADV_DIRECT_LDC):
	{
		if (((param->status == GAP_ERR_CANCELED) || (param->status == GAP_ERR_TIMEOUT)) && (!adv_timeout_flag))
		{
			ke_state_set(TASK_APP, APPM_READY);
				ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
//			if(app_sec_get_bond_status())
//			{
//				//if device has bonded, then start direct adv
//				appm_start_direct_dvertising();
//			}
//			else
//			{
//				//device not bonded, start general adv
//				appm_start_advertising();
//			}
		}
		//if device cancel advtising is adv timeout,then reset this flag
		if((param->status == GAP_ERR_CANCELED) && (adv_timeout_flag == true))
		{
			adv_timeout_flag = false;
		}
	}
        break;

        default:
        {
            // Drop the message
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static int gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_get_dev_info_req_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    switch(param->req)
    {
        case GAPC_DEV_NAME://设备名
        {	UART_PRINTF("%s\r\n", GAPC_DEV_NAME);
            struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
            cfm->req = param->req;
            cfm->info.name.length = appm_get_dev_name(cfm->info.name.value);

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_APPEARANCE://设备外观
        {	UART_PRINTF("%s\r\n", GAPC_DEV_APPEARANCE);
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm);
            cfm->req = param->req;
            
            // No appearance
            cfm->info.appearance = 0;

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_SLV_PREF_PARAMS://设备从机首选参数
        {	UART_PRINTF("%s\r\n", GAPC_DEV_SLV_PREF_PARAMS);
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                    								                         src_id, dest_id,
                                                             gapc_get_dev_info_cfm);
            cfm->req = param->req;
            // Slave preferred Connection interval Min
            cfm->info.slv_params.con_intv_min = 8;
            // Slave preferred Connection interval Max
            cfm->info.slv_params.con_intv_max = 10;
            // Slave preferred Connection latency
            cfm->info.slv_params.slave_latency = 180;
            // Slave preferred Link supervision timeout
            cfm->info.slv_params.conn_timeout  = 600;  // 6s (600*10ms)

            // Send message
            ke_msg_send(cfm);
        } break;

        default: /* Do Nothing */
			break;
    }


    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles GAPC_SET_DEV_INFO_REQ_IND message. GAPC设置设备配置信息
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_set_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_set_dev_info_req_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	// Set Device configuration
	struct gapc_set_dev_info_cfm* cfm = KE_MSG_ALLOC(GAPC_SET_DEV_INFO_CFM, src_id, dest_id,
                                                 gapc_set_dev_info_cfm);
	// Reject to change parameters
	cfm->status = GAP_ERR_REJECTED;
	cfm->req = param->req;
	// Send message
	ke_msg_send(cfm);

	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles connection complete event from the GAP. Enable all required profiles
 * 当手机连接蓝牙后的句柄
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_connection_req_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{	
	UART_PRINTF("%s\r\n", __func__);
	adv_timeout_flag = false;
    app_env.conidx = KE_IDX_GET(src_id);
    // Check if the received Connection Handle was valid检查接收到的连接句柄是否有效
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        // Retrieve the connection info from the parameters从参数中检索连接信息
        app_env.conhdl = param->conhdl;
		// Clear the advertising timeout timer
		if (ke_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))//关闭广播超时计时器
		{
			ke_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
		}
		Anti_Lose.Con_State=ON;
#if BK_CONNNECT_FILTER_CTRL
		//for test resolv peer addr :sean 2017.10.30
		if((param->peer_addr_type == 1) && app_sec_env.bonded)
		{
			
			appm_gapm_resolv_dev_addr(param->peer_addr,app_env.peer_irk.irk);
		}
		else
		{
			struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
			                                  KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
			                                  gapc_connection_cfm);

		#if (HID_CONNECT_ANY)
			cfm->auth = GAP_AUTH_REQ_NO_MITM_BOND;
		#else
			cfm->auth = app_sec_get_bond_status() ? GAP_AUTH_REQ_NO_MITM_BOND : GAP_AUTH_REQ_NO_MITM_NO_BOND;
		#endif

			//Send the message
			ke_msg_send(cfm);
		}
#else		
        // Send connection confirmation发送连接确认
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                                          KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                          gapc_connection_cfm);

		#if (HID_CONNECT_ANY)
		cfm->auth = GAP_AUTH_REQ_NO_MITM_BOND;
		#else            //获取邦定状态
		cfm->auth = app_sec_get_bond_status() ? GAP_AUTH_REQ_NO_MITM_BOND : GAP_AUTH_REQ_NO_MITM_NO_BOND;
		#endif
        // Send the message
        ke_msg_send(cfm);
#endif
        /*--------------------------------------------------------------
         * ENABLE REQUIRED PROFILES
         *--------------------------------------------------------------*/
         
        // 使能电池服务
    app_batt_enable_prf(app_env.conhdl);
		UART_PRINTF("peer_addr_type = 0x%x\r\n",param->peer_addr_type);
		UART_PRINTF("peer_addr = ");
		for(uint8_t i = 0; i < sizeof(bd_addr_t); i ++)
		{
			UART_PRINTF("0x%02x ",param->peer_addr.addr[i]);
		}
		UART_PRINTF("\r\n");		
        // We are now in connected State
        ke_state_set(dest_id, APPM_CONNECTED);//设置任务为连接状态

		//如果设备未与本地绑定，则发送绑定请求
	
//		UART_PRINTF("app_sec_get_bond_status = 0x0\r\n");
//		app_sec_env.bonded = false;
//		app_sec_env.peer_pairing = false;
//		app_sec_env.peer_encrypt = false;
	
		wdt_disable_flag=1;//关闭看门狗
	  ke_timer_set(APP_ANCS_REQ_IND,TASK_APP,80); 
		//if device not bond with local, send bond request
		if(!app_sec_get_bond_status())
		{
			UART_PRINTF("app_sec_get_bond_status = 0x0\r\n");
//			ke_timer_set(APP_SEND_SECURITY_REQ,TASK_APP,20);//开始发送安全请求
		}
		else
		{
			UART_PRINTF("app_sec_get_bond_status = 0x1\r\n");
		}			
		
		#if (APP_GET_RSSI_EN)
		ke_timer_set(APP_GET_RSSI_TIMER,TASK_APP,120);//获取信号强度
    #endif
		#if UPDATE_CONNENCT_PARAM
		ke_timer_set(APP_PARAM_UPDATE_REQ_IND,TASK_APP,100); //更新连接参数
		ke_timer_set(GATTC_MTU_CHANGED_REQ,TASK_APP,150); //更新MTU参数
//		ke_timer_set(UPLOAD_DATA,TASK_APP , 500);		
		#endif	
    user_batt_send_lvl_handler();//发送电池电量
		user_Motor_handler(1,50);//振动一下
		User_Anti_Con_IND();//解除防丢提醒
    if(phone_Beep_F)
		{phone_Beep_F=0;
			ke_timer_set(CALL_PHONE_BEEP,TASK_APP,200); //发送手机鸣叫命令
 		}
    }
    else
    {
        // No connection has been establish, restart advertising
		appm_start_advertising();
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("gapc_cmp_evt_handler operation = %x\r\n",param->operation);
	switch(param->operation)
	{
    	case (GAPC_UPDATE_PARAMS):  //0x09连接参数更新成功
    	{
			if (param->status != GAP_ERR_NO_ERROR)
        	{
            UART_PRINTF("gapc update params fail !\r\n");
			    }
			else
					{
						UART_PRINTF("gapc update params ok !\r\n");
					}
    	} break;
		case (GAPC_DISCONNECT): //0x01复位广播
		{
			if(param->status == GAP_ERR_NO_ERROR)
			{
				UART_PRINTF("pairing_fail = 0x%x\r\n",app_sec_env.pairing_fail);
				//bonding info lost and pairing fail
				if(app_sec_env.pairing_fail)
				{
					app_sec_env.pairing_fail = false;
					
					UART_PRINTF("restart advertising\r\n");
					
					ke_state_set(TASK_APP, APPM_READY);
					
					//restart advertising
//					appm_start_advertising();
								// No more service to add, start advertising
//			if(app_sec_get_bond_status())
//			{
//				//if device has bonded, then start direct adv
//				appm_start_direct_dvertising();
//			}
//			else
//			{
//				//device not bonded, start general adv
//				appm_start_advertising();
//			}
			ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
				}
			}
		}break;
		case (GAPC_SECURITY_REQ): //0x0c安全验证
		{
			if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc security req fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc security req ok !\r\n");
	        }
		}break;
		case (GAPC_BOND): // 0xa邦定
    	{
	        if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc bond fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc bond ok !\r\n");
		          Pairing_Flag=0;
	        }
    	}break;
		
		case (GAPC_ENCRYPT): // 0xb加密
		{
			if (param->status != GAP_ERR_NO_ERROR)
			{
				UART_PRINTF("gapc encrypt start fail !\r\n");
			}
			else
			{
				UART_PRINTF("gapc encrypt start ok !\r\n");

			}
		}
		break;
		case (GAPC_GET_CON_RSSI): //获取连接信号强度
		{
			if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc RSSI req fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc RSSI req ok !\r\n");
	        }
		}break;		

    	default:
    	  break;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.//
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
//断开连接后的指示
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	UART_PRINTF("disconnect link reason = 0x%x\r\n",param->reason);//断开连接原因
  Anti_Lose.Con_State=OFF;//防丢断连状态
  ke_msg_send_basic(USER_ANTI_PERIOD_TIMER,TASK_APP,TASK_APP);//发送防丢消息
	ke_state_set(TASK_APP, APPM_READY);
	adv_timeout_flag = false;
	#if (HID_CONNECT_ANY)
	  app_sec_env.bonded = false;
    app_sec_env.peer_pairing = false;
    app_sec_env.peer_encrypt = false;
    app_sec_env.bond_lost = false;
    app_sec_env.pairing_fail = false;
  #endif
	wdt_disable_flag = 1;
	
	// Restart Advertising
//	app_sec_env.bond_lost = false;
//	appm_start_advertising();
			// No more service to add, start advertising
//			if(app_sec_get_bond_status())
//			{
//				//if device has bonded, then start direct adv
//				appm_start_direct_dvertising();
//			}
//			else
//			{
//				//device not bonded, start general adv
//				appm_start_advertising();
//			}
#if (!HID_CONNECT_ANY)
//	if(app_sec_env.bond_lost)
#endif		
	{
		app_sec_env.bond_lost = false;
//		app_sec_env.bonded = false;
		ke_state_set(TASK_APP, APPM_READY);
	  ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
		wdt_disable_flag=0;
	}


    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles profile add indication from the GAP.//建立一个数据库完成后的指示
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_profile_added_ind_handler(ke_msg_id_t const msgid,
                                          struct gapm_profile_added_ind *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Current State
    uint8_t state = ke_state_get(dest_id);

    if (state == APPM_CREATE_DB)
    {
//			 	UART_PRINTF("gapm_profile_added_ind_handler\r\n");
        switch (param->prf_task_id)
        {
            default: 
							
			break;
        }
    }
    else
    {
        ASSERT_INFO(0, state, src_id);
    }

    return KE_MSG_CONSUMED;
}


/*******************************************************************************
 * Function: app_period_timer_handler用硬件定时器校准软定时器
 * Description: app period timer process
 * Input: msgid -Id of the message received.
 *		  param -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance (TASK_GAP).
 *		  ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/

static int app_period_timer_handler(ke_msg_id_t const msgid,
                                          void *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
		
#if (RC_CALIBRATE)   //RC校准 

    uint32_t timer_625us,timer_1us,timer_ms;

    lld_evt_time_get_us(&timer_625us,&timer_1us);
    //timer_625us这个值最大为0x07FFFFFF，超过又从0开始
//    timer_ms=((uint64_t)timer_625us*625+timer_1us)/1000;

    //UART_PRINTF("%d\r\n", timer_ms);        
    user_timer_init();


    ke_timer_set(APP_PERIOD_TIMER,TASK_APP,30000);
#endif  
    Key_Scan();
	User_Anti_KEY_Text();
  if(KEY1_Dev.Value==Long_Trg_Value)//按键长按
	{KEY1_Dev.Value=0;
	 appm_switch_general_adv();
   Pairing_Flag=1;
//	if(ke_state_get(TASK_APP) == APPM_ADVERTISING)//如果在广播状态下 
//	{	appm_stop_advertising();
//	  wdt_disable_flag=1;//关闭看门狗
//		ke_state_set(TASK_APP, APPM_READY);
//	}
//	if(ke_state_get(TASK_APP) == APPM_READY)
//			{
//		UART_PRINTF("app_sec_remove_bond\r\n");
//	 app_sec_remove_bond();//解除绑定
//	 ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
//			}
//		Pairing_Flag=1;
	}
 	if(ke_state_get(TASK_APP) == APPM_READY)
	{
 if(KEY1_Dev.Value==1)
 {KEY1_Dev.Value=0;
//  UART_PRINTF("KEY1_Dev.Value=1\r\n");
 ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);//开始广播
  }
		//BlueLedOn();
	}
	else
	{
//		if(++uLedDutyCnt > APP_LEDDUTY_DB)
//		{
//			uLedDutyCnt = 0;
//			//BlueLedToggle();
//		}
	} 
#if (SYSTEM_SLEEP)
	static uint16_t key_free_dalay_cnt=0;
//	app_key_state = key_status_check();


//	if(app_key_state != ALL_KEY_FREE_DELAY)
//	{
//		ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
//	}
// if(KEY1_Dev.Value==1)
// {KEY1_Dev.Value=0;
// UART_PRINTF("KEY1_Dev.Value=1\r\n");
// 
//  }
	if(KEY1_Dev.Down_Time==0)
	{
		key_free_dalay_cnt++;

		if(key_free_dalay_cnt>=200)
		{
//			app_key_state=ALL_KEY_FREE_DELAY;
			key_wakeup_config();

			UART_PRINTF("key sleep\r\n");
		}
		else
		{ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
		}
	}
	else if(KEY1_Dev.Down_Time>0)
	{key_free_dalay_cnt=0;
		ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
	}


#else
	ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
#endif	
    return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief Handles reception of all messages sent from the lower layers to the application
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
 int appm_msg_handler(ke_msg_id_t const msgid,
                            void *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
    // Retrieve identifier of the task from received message
    ke_task_id_t src_task_id = MSG_T(msgid);
    // Message policy
    uint8_t msg_pol          = KE_MSG_CONSUMED;


    switch (src_task_id)
    {
        case (TASK_ID_GAPC):
        {
            if ((msgid >= GAPC_BOND_CMD) &&
                (msgid <= GAPC_SECURITY_IND))
            {
                // Call the Security Module
                msg_pol = appm_get_handler(&app_sec_table_handler, msgid, param, src_id);
            }
        } break;

        case (TASK_ID_GATTC):
        {
            // Service Changed - Drop
        } break;
        case (TASK_ID_FFF0S):
        {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_fff0_table_handler, msgid, param, src_id);//在消息表中找到对应实例
        } break;
        case (TASK_ID_ANCSC):
        {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_ancsc_table_handler, msgid, param, src_id);
        } break;
				
        case (TASK_ID_DISS):
        {
            // Call the Device Information Module
            msg_pol = appm_get_handler(&app_dis_table_handler, msgid, param, src_id);
        } break;

        case (TASK_ID_BASS):
        {
            // Call the Battery Module
            msg_pol = appm_get_handler(&app_batt_table_handler, msgid, param, src_id);
        } break;
		
        case (TASK_ID_OADS):
        {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_oads_table_handler, msgid, param, src_id);
        } break;

        default:
        {
        } break;
    }

    return (msg_pol);
}


/*******************************************************************************
 * Function: gapc_update_conn_param_req_ind_handler更新连接参数
 * Description: Update request command processing from slaver connection parameters
 * Input: msgid   -Id of the message received.
 *		  param   -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance
 *		  src_id  -ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int gapc_update_conn_param_req_ind_handler (ke_msg_id_t const msgid, 
									const struct gapc_param_update_req_ind  *param,
                 					ke_task_id_t const dest_id, 
                 					ke_task_id_t const src_id)
{
	UART_PRINTF("slave send param_update_req\r\n");
	struct gapc_conn_param  up_param;
	
	up_param.intv_min   = BLE_UAPDATA_MIN_INTVALUE;
	up_param.intv_max   = BLE_UAPDATA_MAX_INTVALUE; 
	up_param.latency    = BLE_UAPDATA_LATENCY;  
	up_param.time_out   = BLE_UAPDATA_TIMEOUT; 
	
	appm_update_param(&up_param);
	return KE_MSG_CONSUMED;
}

 
/*******************************************************************************
 * Function: gapc_le_pkt_size_ind_handler
 * Description: GAPC_LE_PKT_SIZE_IND
 * Input: msgid   -Id of the message received.
 *		  param   -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance
 *		  src_id  -ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int gapc_le_pkt_size_ind_handler (ke_msg_id_t const msgid, 
									const struct gapc_le_pkt_size_ind  *param,
                 					ke_task_id_t const dest_id, 
                 					ke_task_id_t const src_id)
{
  UART_PRINTF("%s \r\n", __func__);
	UART_PRINTF("1max_rx_octets = %d\r\n",param->max_rx_octets);
	UART_PRINTF("1max_rx_time = %d\r\n",param->max_rx_time);
	UART_PRINTF("1max_tx_octets = %d\r\n",param->max_tx_octets);
	UART_PRINTF("1max_tx_time = %d\r\n",param->max_tx_time);
	
	return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief  GAPC_PARAM_UPDATED_IND接受完主设备更新过来的连接参数并显示
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_param_updated_ind_handler (ke_msg_id_t const msgid, 
										const struct gapc_param_updated_ind  *param,
                 						ke_task_id_t const dest_id, 
                 						ke_task_id_t const src_id)
{
    UART_PRINTF("%s \r\n", __func__);
	UART_PRINTF("con_interval = %d\r\n",param->con_interval);
	UART_PRINTF("con_latency = %d\r\n",param->con_latency);
	UART_PRINTF("sup_to = %d\r\n",param->sup_to);
	
	return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
* @brief  APP_SEND_SECURITY_REQAPP   发送安全请求
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
 extern unsigned char ancs_flag;
static int gapc_send_security_req_handler(ke_msg_id_t const msgid, 
										void const *param,
        								ke_task_id_t const dest_id, 
        								ke_task_id_t const src_id)
{
//	if(ancs_flag == 1)
//	{
//		ancs_flag = 0;
    	appm_send_seurity_req();
//	}
	
    return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief  GATTC_MTU_CHANGED_IND
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,
                                     struct gattc_mtu_changed_ind const *ind,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n",__func__);
	UART_PRINTF("ind->mtu = %d,seq = %d\r\n",ind->mtu,ind->seq_num);
	
 	return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief  GATTC_MTU_CHANGED_IND
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_mtu_changed_req_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n",__func__);
   struct gattc_exc_mtu_cmd *cmd=KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
		                             KE_BUILD_ID(TASK_GATTC,app_env.conidx),
	                               TASK_APP,gattc_exc_mtu_cmd);
	 cmd->operation =GATTC_MTU_EXCH;
	 cmd->seq_num =0;
	 ke_msg_send(cmd);
 	return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief   GAPC_PARAM_UPDATE_REQ_IND接受主设备更新的连接参数
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
                                struct gapc_param_update_req_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n", __func__);
	// Prepare the GAPC_PARAM_UPDATE_CFM message
    struct gapc_param_update_cfm *cfm = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM,
                                             src_id, dest_id,
                                             gapc_param_update_cfm);
	 
	cfm->ce_len_max = 0xffff;
	cfm->ce_len_min = 0xffff;
	cfm->accept = true; 

	// Send message
    ke_msg_send(cfm);
	 
	return (KE_MSG_CONSUMED);
}
//苹果中心通知服务句柄
static int app_ancs_req_handler(ke_msg_id_t const msgid, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    //ke_timer_set(APP_LOOP_EVENT_IND,TASK_APP, 3);
    app_ancsc_enable_prf(app_env.conhdl);
    return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief   app_adv_enable_handler广播开始
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_adv_enable_handler(ke_msg_id_t const msgid,
                                  void const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
	if(ke_state_get(TASK_APP) == APPM_CONNECTED)//如果已经连接则不处理
	{
		return KE_MSG_CONSUMED;
	}
	UART_PRINTF("%s\r\n", __func__);
UART_PRINTF("%d\r\n", app_sec_get_bond_status());
	// Restart Advertising
	if(app_sec_get_bond_status())//如果是邦定状态下则发送定向广播
	{
		struct gapc_bond_ind  param2;
			  uint8_t peer_address_len = NVDS_LEN_PEER_IRK;//7
//	
           	if (nvds_get(NVDS_TAG_PEER_IRK, &peer_address_len, (uint8_t *)&(param2.data.irk)) != NVDS_OK)
           	{
               	ASSERT_ERR(0);
           	}		
			UART_PRINTF("irk.keyll = ");
			for(int i = 0;i<sizeof(struct gap_sec_key);i++)
			{
				UART_PRINTF("0x%x ",param2.data.irk.irk.key[i]);
			}
			UART_PRINTF("\r\n");
			UART_PRINTF("addr.typell = %x\r\n",param2.data.irk.addr.addr_type);
			UART_PRINTF("addr.addrll = ");
			for(int i = 0;i<sizeof(struct bd_addr);i++)
			{
				UART_PRINTF("0x%x ",param2.data.irk.addr.addr.addr[i]);
				                   
			}
			UART_PRINTF("\r\n");						
		appm_gapm_resolv_dev_addr(param2.data.irk.addr.addr,param2.data.irk.irk);
//		//if device has bonded, then start direct adv
		appm_start_direct_dvertising();
//		appm_start_advertising();
//		appm_start_white_list_dvertising();
//		}				
	}
	else
	{
		//device not bonded, start general adv非邦定状态下发送广播
		appm_start_advertising();
	}
	if(Anti_Lose.State==OFF)//防丢功能关闭才用广播超时
	{
	ke_timer_set(APP_ADV_TIMEOUT_TIMER, TASK_APP, APP_DFLT_ADV_DURATION);
	}
//  ke_timer_set(APP_LED_CTRL_SCAN,TASK_APP,10);
	return (KE_MSG_CONSUMED);
}
s16 Data_Filte(s16 Data)
{ static u8 cnt=0;
	static s8 Filte_buff[18];
	s16 sun=0;
	s8 Ret,Filte_MAX=0,Filte_MIN=0XFF;

  Filte_buff[cnt]=Data;
  cnt++;
	if(cnt>=18)
	{cnt=0;}
	for(u8 i=0;i<18;i++)
	{
		if(Filte_buff[i]>Filte_MAX)
		{Filte_MAX=Filte_buff[i];}
    if(Filte_buff[i]<Filte_MIN)
		{Filte_MIN=Filte_buff[i];}
	  sun+=Filte_buff[i];
	}
	sun-=Filte_MAX;
	sun-=Filte_MIN;
	Ret=sun>>4;
	return Ret;

}
/**
 ****************************************************************************************
 * @brief gapc_conn_rssi_ind_handler.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_conn_rssi_ind_handler(ke_msg_id_t const msgid,
        struct gapc_con_rssi_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

	if(ke_state_get(dest_id) == APPM_CONNECTED)
	{
//		UART_PRINTF("get rssi = %d\r\n",param->rssi);
		ANO_DT_Send_Date(0xf1, Data_Filte(param->rssi), 0,0,0,0);
		
	}
	return (KE_MSG_CONSUMED);
}


/*******************************************************************************
 * Function: app_get_rssi_timer_handler
 * Description: app period timer process
 * Input: msgid -Id of the message received.
 *		  param -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance (TASK_GAP).
 *		  ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int app_get_rssi_timer_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{	
	UART_PRINTF("%s\r\n", __func__);
	
	if(ke_state_get(dest_id) == APPM_CONNECTED)
	{
		appm_get_conn_rssi();
	}
	
	ke_timer_set(APP_GET_RSSI_TIMER,TASK_APP,5);
	return KE_MSG_CONSUMED;

}
/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */

/* Default State handlers definition. */
const struct ke_msg_handler appm_default_state[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,    	(ke_msg_func_t)appm_msg_handler},
		{APP_ADV_TIMEOUT_TIMER,     	(ke_msg_func_t)app_adv_timeout_handler},/*广播信号连接超时*/
    {GAPM_DEVICE_READY_IND,     	(ke_msg_func_t)gapm_device_ready_ind_handler},/*设备就绪指示*/
    {GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},         /*GAPM指令操作完成指示*/
    {GAPC_GET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_get_dev_info_req_ind_handler},/*获取设备信息*/
    {GAPC_SET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_set_dev_info_req_ind_handler},/*GAPC设置设备配置信息*/
    {GAPC_CONNECTION_REQ_IND,   	(ke_msg_func_t)gapc_connection_req_ind_handler},  /*手机连接后的处理事件*/
    {GAPC_CMP_EVT,             		(ke_msg_func_t)gapc_cmp_evt_handler},           /*GAPC命令事件的处理*/
    {GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},   /*断开连接后的指示*/
    {GAPM_PROFILE_ADDED_IND,    	(ke_msg_func_t)gapm_profile_added_ind_handler},/*建立一个数据库完成后的指示*/
    {GAPC_LE_PKT_SIZE_IND,			  (ke_msg_func_t)gapc_le_pkt_size_ind_handler},
    {GAPC_PARAM_UPDATED_IND,		  (ke_msg_func_t)gapc_param_updated_ind_handler},/*接受完主设备更新的连接参数*/
    {APP_SEND_SECURITY_REQ,     	(ke_msg_func_t)gapc_send_security_req_handler},/*APP发送安全请求*/
    {GATTC_MTU_CHANGED_IND,			  (ke_msg_func_t)gattc_mtu_changed_ind_handler},/*数据包变更指示*/	
		{GATTC_MTU_CHANGED_REQ,			  (ke_msg_func_t)gattc_mtu_changed_req_handler},/*数据包变更指示*/
		
    {GAPC_PARAM_UPDATE_REQ_IND, 	(ke_msg_func_t)gapc_param_update_req_ind_handler},/*接受主设备更新的连接参数*/
    {APP_PARAM_UPDATE_REQ_IND, 		(ke_msg_func_t)gapc_update_conn_param_req_ind_handler},/*APP更新设备连接参数*/
    {APP_ANCS_REQ_IND,				    (ke_msg_func_t)app_ancs_req_handler},
    {APP_PERIOD_TIMER,				    (ke_msg_func_t)app_period_timer_handler},/*周期定时器*/
		{APP_ADV_ENABLE_TIMER,			  (ke_msg_func_t)app_adv_enable_handler},/*使能广播*/
		{APP_GET_RSSI_TIMER,			    (ke_msg_func_t)app_get_rssi_timer_handler},/*定时获取连接信号强度定时器*/
    {GAPC_CON_RSSI_IND, 			    (ke_msg_func_t)gapc_conn_rssi_ind_handler},/*获取连接信号强度*/
//、、、、、、、、、、、、、、、、、、、用户程序消息、、、、、、、、、、、、、、、、	
		{APP_LED_CTRL_SCAN,			      (ke_msg_func_t)app_led_ctrl_scan_handler},/*LED控制*/
		{USER_APP_CALENDAR_UPDATE,		(ke_msg_func_t)Calendar_Update_handler},/*日历更新数据*/
		{SL_SC7A21_CLICK_TIMER,		    (ke_msg_func_t)SL_SC7A21_Click_Timer_Cnt},/*SC7A21敲击计数器*/
		{SL_SC7A21_GET_PEDO,		      (ke_msg_func_t)SL_SC7A21_GET_PEDO_VALUE},/*SC7A21获取计步值*/
	  {USER_MOTOR_PERIOD_TIMER,		  (ke_msg_func_t)user_Motor_period_timer_handler},/*振动电机周期性扫描*/
//		{USER_BATT_SEND_LVL,		       (ke_msg_func_t)user_batt_send_lvl_handler},/*发送电池电量*/
		{USER_ALARM_IND,		           (ke_msg_func_t)user_alarm_ind},/*闹钟提醒*/
//////////////////////////////////防丢//////////////////////////////////////////////////////////////////		
{USER_ANTI_PERIOD_TIMER,		           (ke_msg_func_t)User_Anti_period_timer_handler},//防丢
//////////////////////////////////同步数据//////////////////////////////////////////////////////////////////	
{UPLOAD_DATA,		                   (ke_msg_func_t)Upload_Data},//同步步数数据
//////////////////////////////////同步数据///////////////////////////////////////////////////////////
{CALL_PHONE_BEEP,		                   (ke_msg_func_t)call_phone_beep},//同步步数数据
};

/* 指定所有状态通用的消息处理程序. */
const struct ke_state_handler appm_default_handler = KE_STATE_HANDLER(appm_default_state);

/* 定义所有任务实例状态的占位符. */
ke_state_t appm_state[APP_IDX_MAX];

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
