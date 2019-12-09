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
/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

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
//�豸�Ѿ�����
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

    cmd->operation = GAPM_RESET;//��λGAPM

    ke_msg_send(cmd);

    return (KE_MSG_CONSUMED);
}

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
//�㲥��ʱ�����¼�
 ****************************************************************************************
 */
static int app_adv_timeout_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);

//	adv_timeout_flag = true;

	// Stop advertising
	appm_stop_advertising();

//	key_wakeup_config();
	wdt_disable_flag=1;
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
        case (GAPM_RESET)://��λ�ɹ�
        {
            if(param->status == GAP_ERR_NO_ERROR)
            {
                // Set Device configuration�����豸������Ϣ
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
	                                                                   TASK_GAPM, TASK_APP,
                                                                   gapm_set_dev_config_cmd);
                // Set the operation
                cmd->operation = GAPM_SET_DEV_CONFIG;
                // Set the device role - Peripheral���������豸��ɫ
                cmd->role      = GAP_ROLE_PERIPHERAL;
                // Set Data length parameters �������ݳ��Ȳ���
                cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;//8λ
                cmd->sugg_max_tx_time   = BLE_MIN_TIME;
								
		 		       cmd->max_mtu = 131;//BLE_MIN_OCTETS;MTU=131
                //Do not support secure connections ��֧�ְ�ȫ����(�������ģʽ)
                cmd->pairing_mode = GAPM_PAIRING_LEGACY;//��ͳ���ģʽ
//								 cmd->pairing_mode = GAPM_PAIRING_SEC_CON;
                
 				//cmd->addr_type   = GAPM_CFG_ADDR_HOST_PRIVACY; //2017-10-24 by alen
                // load IRK
                memcpy(cmd->irk.key, app_env.loc_irk, KEY_LEN);//��������IRK

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
        case (GAPM_PROFILE_TASK_ADD)://���ط���
        {
            // Add the next requested service
            if (!appm_add_svc())
            {                       //���������ɺ�ʼ�㲥
                // Go to the ready state
                ke_state_set(TASK_APP, APPM_READY);
							
//				         appm_start_advertising();
							// No more service to add, start advertising
					 if(app_sec_get_bond_status())//�ж��Ƿ����������Ͷ���㲥�����û�оͷ���ͨ�ù㲥
								{
									//if device has bonded, then start direct adv
									appm_start_direct_dvertising();
								}
								else
								{
									//device not bonded, start general adv
									appm_start_advertising();
								}			
							
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
			if (param->status == GAP_ERR_TIMEOUT)
			{
                ke_state_set(TASK_APP, APPM_READY);
				
				//device not bonded, start general adv
				appm_start_advertising();
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
        case GAPC_DEV_NAME://�豸��
        {	UART_PRINTF("%s\r\n", GAPC_DEV_NAME);
            struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
            cfm->req = param->req;
            cfm->info.name.length = appm_get_dev_name(cfm->info.name.value);

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_APPEARANCE://�豸���
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

        case GAPC_DEV_SLV_PREF_PARAMS:
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
 * @brief Handles GAPC_SET_DEV_INFO_REQ_IND message. GAPC�����豸������Ϣ
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
 * ���ֻ�����������ľ��
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
	
    app_env.conidx = KE_IDX_GET(src_id);
    // Check if the received Connection Handle was valid�����յ������Ӿ���Ƿ���Ч
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        // Retrieve the connection info from the parameters�Ӳ����м���������Ϣ
        app_env.conhdl = param->conhdl;

        // Send connection confirmation��������ȷ��
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                gapc_connection_cfm);
       //��ȡ�״̬
        cfm->auth = app_sec_get_bond_status() ? GAP_AUTH_REQ_NO_MITM_BOND : GAP_AUTH_REQ_NO_MITM_NO_BOND;
        // Send the message
        ke_msg_send(cfm);

        /*--------------------------------------------------------------
         * ENABLE REQUIRED PROFILES
         *--------------------------------------------------------------*/
         
        // ʹ�ܵ�ط���
        app_batt_enable_prf(app_env.conhdl);
		
        // We are now in connected State
        ke_state_set(dest_id, APPM_CONNECTED);//��������Ϊ����״̬

		//����豸δ�뱾�ذ󶨣����Ͱ�����
	
		UART_PRINTF("app_sec_get_bond_status = 0x0\r\n");
		app_sec_env.bonded = false;
		app_sec_env.peer_pairing = false;
		app_sec_env.peer_encrypt = false;
	
		
	    ke_timer_set(APP_ANCS_REQ_IND,TASK_APP,80); 
			ke_timer_set(APP_SEND_SECURITY_REQ,TASK_APP,220);  //��ʼ���Ͱ�ȫ����
		#if (APP_GET_RSSI_EN)
		ke_timer_set(APP_GET_RSSI_TIMER,TASK_APP,120);//��ȡ�ź�ǿ��
    #endif
		#if UPDATE_CONNENCT_PARAM
		ke_timer_set(APP_PARAM_UPDATE_REQ_IND,TASK_APP,100); //�������Ӳ���
		#endif	
    user_batt_send_lvl_handler();//���͵�ص���
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
    	case (GAPC_UPDATE_PARAMS):  //0x09���Ӳ������³ɹ�
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
		case (GAPC_DISCONNECT): //0x01��λ�㲥
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
					appm_start_advertising();
				}
			}
		}break;
		case (GAPC_SECURITY_REQ): //0x0c��ȫ��֤
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
		case (GAPC_BOND): // 0xa�
    	{
	        if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc bond fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc bond ok !\r\n");
		
	        }
    	}break;
		
		case (GAPC_ENCRYPT): // 0xb����
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
		case (GAPC_GET_CON_RSSI): //��ȡ�����ź�ǿ��
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
 * @brief Handles disconnection complete event from the GAP.//�Ͽ����Ӻ��ָʾ
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	UART_PRINTF("disconnect link reason = 0x%x\r\n",param->reason);//�Ͽ�����ԭ��

	// Go to the ready state
	ke_state_set(TASK_APP, APPM_READY);

	wdt_disable_flag = 1;
	
	// Restart Advertising
	appm_start_advertising();

	
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles profile add indication from the GAP.//����һ�����ݿ���ɺ��ָʾ
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
 * Function: app_period_timer_handler��Ӳ����ʱ��У׼��ʱ��
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
#if (RC_CALIBRATE)   //RCУ׼ 
    uint32_t timer_625us,timer_1us,timer_ms;

    lld_evt_time_get_us(&timer_625us,&timer_1us);
    //timer_625us���ֵ���Ϊ0x07FFFFFF�������ִ�0��ʼ
//    timer_ms=((uint64_t)timer_625us*625+timer_1us)/1000;

    //UART_PRINTF("%d\r\n", timer_ms);        
    user_timer_init();


    ke_timer_set(APP_PERIOD_TIMER,TASK_APP,30000);
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
            msg_pol = appm_get_handler(&app_fff0_table_handler, msgid, param, src_id);//����Ϣ�����ҵ���Ӧʵ��
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
 * Function: gapc_update_conn_param_req_ind_handler�������Ӳ���
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
 * @brief  GAPC_PARAM_UPDATED_IND���������豸���¹��������Ӳ�������ʾ
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
* @brief  APP_SEND_SECURITY_REQAPP   ���Ͱ�ȫ����
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
 * @brief   GAPC_PARAM_UPDATE_REQ_IND�������豸���µ����Ӳ���
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
//ƻ������֪ͨ������
static int app_ancs_req_handler(ke_msg_id_t const msgid, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    //ke_timer_set(APP_LOOP_EVENT_IND,TASK_APP, 3);
    app_ancsc_enable_prf(app_env.conhdl);
    return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief   app_adv_enable_handler�㲥��ʼ
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
	if(ke_state_get(TASK_APP) == APPM_CONNECTED)//����Ѿ������򲻴���
	{
		return KE_MSG_CONSUMED;
	}
	UART_PRINTF("%s\r\n", __func__);

	// Restart Advertising
	if(app_sec_get_bond_status())//����ǰ״̬�����Ͷ���㲥
	{
		//if device has bonded, then start direct adv
		appm_start_direct_dvertising();
	}
	else
	{
		//device not bonded, start general adv�ǰ״̬�·��͹㲥
		appm_start_advertising();
	}
//  ke_timer_set(APP_LED_CTRL_SCAN,TASK_APP,10);
	return (KE_MSG_CONSUMED);
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
		UART_PRINTF("get rssi = %d\r\n",param->rssi);
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
	
	ke_timer_set(APP_GET_RSSI_TIMER,TASK_APP,500);

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
		{APP_ADV_TIMEOUT_TIMER,     	(ke_msg_func_t)app_adv_timeout_handler},/*�㲥�ź����ӳ�ʱ*/
    {GAPM_DEVICE_READY_IND,     	(ke_msg_func_t)gapm_device_ready_ind_handler},/*�豸����ָʾ*/
    {GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},         /*GAPMָ��������ָʾ*/
    {GAPC_GET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_get_dev_info_req_ind_handler},/*��ȡ�豸��Ϣ*/
    {GAPC_SET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_set_dev_info_req_ind_handler},/*GAPC�����豸������Ϣ*/
    {GAPC_CONNECTION_REQ_IND,   	(ke_msg_func_t)gapc_connection_req_ind_handler},  /*�ֻ����Ӻ�Ĵ����¼�*/
    {GAPC_CMP_EVT,             		(ke_msg_func_t)gapc_cmp_evt_handler},           /*GAPC�����¼��Ĵ���*/
    {GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},   /*�Ͽ����Ӻ��ָʾ*/
    {GAPM_PROFILE_ADDED_IND,    	(ke_msg_func_t)gapm_profile_added_ind_handler},/*����һ�����ݿ���ɺ��ָʾ*/
    {GAPC_LE_PKT_SIZE_IND,			  (ke_msg_func_t)gapc_le_pkt_size_ind_handler},
    {GAPC_PARAM_UPDATED_IND,		  (ke_msg_func_t)gapc_param_updated_ind_handler},/*���������豸���µ����Ӳ���*/
    {APP_SEND_SECURITY_REQ,     	(ke_msg_func_t)gapc_send_security_req_handler},/*APP���Ͱ�ȫ����*/
    {GATTC_MTU_CHANGED_IND,			  (ke_msg_func_t)gattc_mtu_changed_ind_handler},/*���ݰ����ָʾ*/	
    {GAPC_PARAM_UPDATE_REQ_IND, 	(ke_msg_func_t)gapc_param_update_req_ind_handler},/*�������豸���µ����Ӳ���*/
    {APP_PARAM_UPDATE_REQ_IND, 		(ke_msg_func_t)gapc_update_conn_param_req_ind_handler},/*APP�����豸���Ӳ���*/
    {APP_ANCS_REQ_IND,				    (ke_msg_func_t)app_ancs_req_handler},
    {APP_PERIOD_TIMER,				    (ke_msg_func_t)app_period_timer_handler},/*���ڶ�ʱ��*/
		{APP_ADV_ENABLE_TIMER,			  (ke_msg_func_t)app_adv_enable_handler},/*ʹ�ܹ㲥*/
		{APP_GET_RSSI_TIMER,			    (ke_msg_func_t)app_get_rssi_timer_handler},/*��ʱ��ȡ�����ź�ǿ�ȶ�ʱ��*/
    {GAPC_CON_RSSI_IND, 			    (ke_msg_func_t)gapc_conn_rssi_ind_handler},/*��ȡ�����ź�ǿ��*/
//���������������������������������������û�������Ϣ��������������������������������	
		{APP_LED_CTRL_SCAN,			      (ke_msg_func_t)app_led_ctrl_scan_handler},/*LED����*/
		{USER_APP_CALENDAR_UPDATE,		(ke_msg_func_t)Calendar_Update_handler},/*������������*/
		{SL_SC7A21_CLICK_TIMER,		    (ke_msg_func_t)SL_SC7A21_Click_Timer_Cnt},/*SC7A21�û�������*/
		{SL_SC7A21_GET_PEDO,		      (ke_msg_func_t)SL_SC7A21_GET_PEDO_VALUE},/*SC7A21��ȡ�Ʋ�ֵ*/
	  {USER_MOTOR_PERIOD_TIMER,		  (ke_msg_func_t)user_Motor_period_timer_handler},/*SC7A21��ȡ�Ʋ�ֵ*/
//		{USER_BATT_SEND_LVL,		       (ke_msg_func_t)user_batt_send_lvl_handler},/*���͵�ص���*/


};

/* ָ������״̬ͨ�õ���Ϣ�������. */
const struct ke_state_handler appm_default_handler = KE_STATE_HANDLER(appm_default_state);

/* ������������ʵ��״̬��ռλ��. */
ke_state_t appm_state[APP_IDX_MAX];

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
