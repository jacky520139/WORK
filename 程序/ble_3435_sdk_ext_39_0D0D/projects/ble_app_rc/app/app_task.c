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
#include "app_wlist.h"
#include "gapc_task.h"            // GAP Controller Task API
#include "gapm_task.h"          // GAP Manager Task API
#include "gattc_task.h"
#include "arch.h"                    // Platform Definitions
#include "ke_timer.h"             // Kernel timer
#include "app_sec.h"              // Security Module Definition
#include "app_dis.h"              // Device Information Module Definition
#include "diss_task.h"
#include "app_batt.h"             // Battery Module Definition
#include "bass_task.h"
#include "app_hid.h"              // HID Module Definition
#include "hogpd_task.h"
#include "app_oads.h"
#include "oads_task.h"
#include "gpio.h"
#include "audio.h"
#include "uart.h"
#include "BK3435_reg.h"
#include "icu.h"
#include "reg_ble_em_cs.h"
#include "lld.h"
#include "app_hid.h"
#include "app_key.h"
#include "user_config.h"
#include "wdt.h"


//this flag use indicate adv timeout
static bool adv_timeout_flag = false;




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
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_adv_timeout_handler(ke_msg_id_t const msgid,
                                   void const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);

	adv_timeout_flag = true;

	// Stop advertising
	appm_stop_advertising();

	key_wakeup_config();
	wdt_disable_flag=1;
	return (KE_MSG_CONSUMED);
}

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

	cmd->operation = GAPM_RESET;

	ke_msg_send(cmd);

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
	case (GAPM_RESET):
	{
		if(param->status == GAP_ERR_NO_ERROR)
		{
			// Set Device configuration
			struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
			                                      TASK_GAPM, TASK_APP,
			                                      gapm_set_dev_config_cmd);
			// Set the operation
			cmd->operation = GAPM_SET_DEV_CONFIG;
			// Set the device role - Peripheral
			cmd->role      = GAP_ROLE_PERIPHERAL;
			// Set Data length parameters
			cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;
			cmd->sugg_max_tx_time   = BLE_MIN_TIME;

			cmd->max_mtu = 131;//BLE_MIN_OCTETS;
			//Do not support secure connections
			cmd->pairing_mode = GAPM_PAIRING_LEGACY;
			// Enable Slave Preferred Connection Parameters present
			cmd->att_cfg = GAPM_MASK_ATT_SLV_PREF_CON_PAR_EN;
			// load IRK
			memcpy(cmd->irk.key, app_env.loc_irk, KEY_LEN);

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
	case (GAPM_PROFILE_TASK_ADD):
	{
		// Add the next requested service
		if (!appm_add_svc())
		{
			// Go to the ready state
			ke_state_set(TASK_APP, APPM_READY);

			// No more service to add, start advertising
			if(app_sec_get_bond_status())
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
		if (((param->status == GAP_ERR_CANCELED) || (param->status == GAP_ERR_TIMEOUT)) && (!adv_timeout_flag))
		{
			ke_state_set(TASK_APP, APPM_READY);
			if(app_sec_get_bond_status())
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
		//if device cancel advtising is adv timeout,then reset this flag
		if((param->status == GAP_ERR_CANCELED) && (adv_timeout_flag == true))
		{
			adv_timeout_flag = false;
		}
	}
	break;
	case (GAPM_RESOLV_ADDR):
	{
		UART_PRINTF("GAPM_RESOLV_ADDR CMP STATUS = %x\r\n",param->status);
		if(param->status == GAP_ERR_NO_ERROR)
		{
			struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
			                                  KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
			                                  gapc_connection_cfm);

			cfm->auth = app_sec_get_bond_status() ? GAP_AUTH_REQ_NO_MITM_BOND : GAP_AUTH_REQ_NO_MITM_NO_BOND;

			//Send the message
			ke_msg_send(cfm);
		}
		else
		{
			appm_disconnect();
		}
	}
	break;
	case GAPM_ADD_DEV_IN_WLIST:


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
	case GAPC_DEV_NAME:
	{
		struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
		                                     src_id, dest_id,
		                                     gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
		cfm->req = param->req;
		cfm->info.name.length = appm_get_dev_name(cfm->info.name.value);

		// Send message
		ke_msg_send(cfm);
	}
	break;

	case GAPC_DEV_APPEARANCE:
	{
		// Allocate message
		struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
		                                    src_id, dest_id,
		                                    gapc_get_dev_info_cfm);
		cfm->req = param->req;
		// Set the device appearance
		cfm->info.appearance = 961;

		// Send message
		ke_msg_send(cfm);
	}
	break;

	case GAPC_DEV_SLV_PREF_PARAMS:
	{
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
	}
	break;

	default: /* Do Nothing */
		break;
	}


	return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles GAPC_SET_DEV_INFO_REQ_IND message.
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
 *
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
	// Check if the received Connection Handle was valid
	if (app_env.conidx != GAP_INVALID_CONIDX)
	{
		// Retrieve the connection info from the parameters
		app_env.conhdl = param->conhdl;

		// Clear the advertising timeout timer
		if (ke_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
		{
			ke_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
		}
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
#endif


		app_hid_init();
		/*--------------------------------------------------------------
		 * ENABLE REQUIRED PROFILES
		 *--------------------------------------------------------------*/

		// Enable Battery Service
		app_batt_enable_prf(app_env.conhdl);
		// Enable HID Service
		app_hid_enable_prf(app_env.conhdl);


		UART_PRINTF("peer_addr_type = 0x%x\r\n",param->peer_addr_type);
		UART_PRINTF("peer_addr = ");
		for(uint8_t i = 0; i < sizeof(bd_addr_t); i ++)
		{
			UART_PRINTF("0x%02x ",param->peer_addr.addr[i]);
		}
		UART_PRINTF("\r\n");

		// We are now in connected State
		ke_state_set(dest_id, APPM_CONNECTED);

		key_state_reset();

		//if device not bond with local, send bond request
		if(!app_sec_get_bond_status())
		{
			UART_PRINTF("app_sec_get_bond_status = 0x0\r\n");
			ke_timer_set(APP_SEND_SECURITY_REQ,TASK_APP,20);
		}
		else
		{
			UART_PRINTF("app_sec_get_bond_status = 0x1\r\n");
		}


#if UPDATE_CONNENCT_PARAM
		ke_timer_set(APP_PARAM_UPDATE_REQ_IND,TASK_APP,100);
#endif

		//set short connect
		if(app_get_mode().target_mode == CONN_MODE_SHORT)
		{
			ke_timer_set(APP_DISCONNECT_TIMER, TASK_APP,app_get_mode().target_timeout);
		}
	}
	else
	{
		// No connection has been establish, restart advertising
		if(app_sec_get_bond_status())
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
	UART_PRINTF("gapc_cmp_evt_handler operation = %x, status = %x\r\n",param->operation, param->status);
	switch(param->operation)
	{
	case (GAPC_UPDATE_PARAMS):
	{
		if (param->status != GAP_ERR_NO_ERROR)
		{
			//appm_disconnect();
		}
		UART_PRINTF("gapc update paras state: 0x%x\r\n", param->status);
	}
	break;

	case (GAPC_DISCONNECT):
	{
		UART_PRINTF("rc disconnect link: 0x%x\r\n", param->status);
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
				appm_start_direct_dvertising();
			}
		}
	}
	break;

	default:
		break;
	}

	return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.
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

	UART_PRINTF("disconnect link reason = 0x%x\r\n",param->reason);

	// Go to the ready state
	ke_state_set(TASK_APP, APPM_READY);

	app_hid_set_send_flag(false);

	audio_stop();

	adv_timeout_flag = false;
#if (HID_CONNECT_ANY)
	app_sec_env.bonded = false;
    app_sec_env.peer_pairing = false;
    app_sec_env.peer_encrypt = false;
    app_sec_env.bond_lost = false;
    app_sec_env.pairing_fail = false;
#endif
	wdt_disable_flag=1;

#if (!HID_CONNECT_ANY)
	if(app_sec_env.bond_lost)
#endif		
	{
		app_sec_env.bond_lost = false;
		ke_state_set(TASK_APP, APPM_READY);
		appm_start_advertising();
		wdt_disable_flag=0;
	}


	return (KE_MSG_CONSUMED);
}


/*******************************************************************************
 * Function: app_period_timer_handler
 * Description: app period timer process
 * Input: msgid -Id of the message received.
 *		  param -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance (TASK_GAP).
 *		  ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int app_period_timer_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
	static uint8_t uLedDutyCnt = 0;
	static uint16_t key_free_dalay_cnt=0;

	//gpio_triger(0x15);
	key_process();
	hid_send_keycode();

	if(ke_state_get(TASK_APP) == APPM_CONNECTED)
	{
		//BlueLedOn();
	}
	else
	{
		if(++uLedDutyCnt > APP_LEDDUTY_DB)
		{
			uLedDutyCnt = 0;
			//BlueLedToggle();
		}
	}

#if (SYSTEM_SLEEP)
	app_key_state = key_status_check();


	if(app_key_state != ALL_KEY_FREE_DELAY)
	{
		ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
	}

	if(app_key_state == ALL_KEY_FREE)
	{
		key_free_dalay_cnt++;

		if(key_free_dalay_cnt>=200)
		{
			app_key_state=ALL_KEY_FREE_DELAY;
			key_wakeup_config();

			UART_PRINTF("key sleep\r\n");
		}
	}
	else if((app_key_state == VOICE_KEY_DOWN)||(app_key_state == GENERAL_KEY_DOWN))
		key_free_dalay_cnt=0;


#else
	ke_timer_set(APP_PERIOD_TIMER,TASK_APP,APP_KEYSCAN_DURATION);
#endif

	return KE_MSG_CONSUMED;

}


/**
 ****************************************************************************************
 * @brief Handles profile add indication from the GAP.
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
static int appm_msg_handler(ke_msg_id_t const msgid,
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
		// else drop the message
	}
	break;

	case (TASK_ID_GATTC):
	{
		// Service Changed - Drop
	} break;

	case (TASK_ID_DISS):
	{
		// Call the Device Information Module
		msg_pol = appm_get_handler(&app_dis_table_handler, msgid, param, src_id);
	}
	break;

	case (TASK_ID_HOGPD):
	{
		// Call the HID Module
		msg_pol = appm_get_handler(&app_hid_table_handler, msgid, param, src_id);
	}
	break;

	case (TASK_ID_BASS):
	{
		// Call the Battery Module
		msg_pol = appm_get_handler(&app_batt_table_handler, msgid, param, src_id);
	}
	break;

	case (TASK_ID_OADS):
	{
		// Call the Health Thermometer Module
		msg_pol = appm_get_handler(&app_oads_table_handler, msgid, param, src_id);
	}
	break;

	default:
		break;
	}

	return (msg_pol);
}


/*******************************************************************************
 * Function: gapc_update_conn_param_req_ind_handler
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
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
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
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
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
 * @brief  GAPC_PARAM_UPDATED_IND
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
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n", __func__);

	UART_PRINTF("con_interval = %d\r\n",param->con_interval);
	UART_PRINTF("con_latency = %d\r\n",param->con_latency);
	UART_PRINTF("sup_to = %d\r\n",param->sup_to);

	return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief  APP_SEND_SECURITY_REQ
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_send_security_req_handler(ke_msg_id_t const msgid, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	appm_send_seurity_req();

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
	UART_PRINTF("ind->mtu = %d.seq = %d\r\n",ind->mtu,ind->seq_num);

	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   GAPC_PARAM_UPDATE_REQ_IND
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

	//cfm->accept = false;
	cfm->ce_len_max = 0xffff;
	cfm->ce_len_min = 0xffff;
	cfm->accept = true;

	// Send message
	ke_msg_send(cfm);

	return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief   app_adv_enable_handler
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
	if(ke_state_get(TASK_APP) == APPM_CONNECTED)
	{
		return KE_MSG_CONSUMED;
	}
	UART_PRINTF("%s\r\n", __func__);

	// Restart Advertising
	if(app_sec_get_bond_status())
	{
		//if device has bonded, then start direct adv
		appm_start_direct_dvertising();
	}
	else
	{
		//device not bonded, start general adv
		appm_start_advertising();
	}

	return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief   app_disconnect_timer_handler
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_disconnect_timer_handler(ke_msg_id_t const msgid,
                                        void const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);

	if(ke_state_get(TASK_APP) == APPM_CONNECTED)
	{
		appm_disconnect();
	}

	return (KE_MSG_CONSUMED);
}


SbcEncoderContext sbc_env;

//voice buffer
uint8_t send_buf[20];


/**
 ****************************************************************************************
 * @brief   APP_ENCODE_IND
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_sbc_encode_handler(ke_msg_id_t const msgid,
                                  struct sbc_encode const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
	if(app_hid_env.audio_start == 1) //adc started
	{
		uint8_t delta;

		//Determines whether the ADC data to be encoded exists in the loop buffer.
		if(pcm_index_in >= pcm_index_out)
		{
			delta = pcm_index_in - pcm_index_out;
		}
		else
		{
			delta = MAX_PCM_BUF_ROW - pcm_index_out + pcm_index_in;
		}
		if(delta > 0)
		{
			sbc_encoder_frame_encode(&sbc_env,&pcm_sample_buffer[pcm_index_out][0]);
			if(++pcm_index_out > (MAX_PCM_BUF_ROW - 1))
			{
				pcm_index_out = 0;
			}
			sbc_env.stream[3] = app_hid_env.encode_cnt++; //for test

			//Store the data to be encoded into the loop buffer
			store_encode_data(&sbc_env.stream[3]);
		}
	}

	return (KE_MSG_CONSUMED);
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
	{APP_ADV_TIMEOUT_TIMER,     	(ke_msg_func_t)app_adv_timeout_handler},
	{GAPM_DEVICE_READY_IND,     	(ke_msg_func_t)gapm_device_ready_ind_handler},
	{GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},
	{GAPC_GET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_get_dev_info_req_ind_handler},
	{GAPC_SET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_set_dev_info_req_ind_handler},
	{GAPC_CONNECTION_REQ_IND,   	(ke_msg_func_t)gapc_connection_req_ind_handler},
	{GAPC_CMP_EVT,             		(ke_msg_func_t)gapc_cmp_evt_handler},
	{GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},
	{GAPM_PROFILE_ADDED_IND,    	(ke_msg_func_t)gapm_profile_added_ind_handler},
	{GAPC_LE_PKT_SIZE_IND,			(ke_msg_func_t)gapc_le_pkt_size_ind_handler},
	{GAPC_PARAM_UPDATED_IND,		(ke_msg_func_t)gapc_param_updated_ind_handler},
	{GATTC_MTU_CHANGED_IND,			(ke_msg_func_t)gattc_mtu_changed_ind_handler},
	{GAPC_PARAM_UPDATE_REQ_IND, 	(ke_msg_func_t)gapc_param_update_req_ind_handler},
	{APP_PARAM_UPDATE_REQ_IND, 		(ke_msg_func_t)gapc_update_conn_param_req_ind_handler},
	{APP_ADV_ENABLE_TIMER,			(ke_msg_func_t)app_adv_enable_handler},
	{APP_PERIOD_TIMER,				(ke_msg_func_t)app_period_timer_handler},
	{APP_SEND_SECURITY_REQ,     	(ke_msg_func_t)gapc_send_security_req_handler},
	{APP_ENCODE_IND,				(ke_msg_func_t)app_sbc_encode_handler},
	{APP_DISCONNECT_TIMER,      	(ke_msg_func_t)app_disconnect_timer_handler},
	{GAPM_WHITE_LIST_SIZE_IND,    	(ke_msg_func_t)app_gapm_white_list_ind_handler},
	{GAPM_RAL_SIZE_IND,				(ke_msg_func_t)app_gapm_ral_list_size_ind_handler},
	{GAPM_RAL_ADDR_IND,				(ke_msg_func_t)app_gapm_ral_addr_ind_handler},
	{GAPM_ADDR_SOLVED_IND,			(ke_msg_func_t)app_gapm_resolv_dev_addr_ind_handler},
	{GAPM_DEV_BDADDR_IND,			(ke_msg_func_t)app_gapm_gen_rand_addr_ind_handler},
	{GAPM_GEN_RAND_NB_IND,			(ke_msg_func_t)app_gapm_gen_rand_number_ind_handler},
};

/* Specifies the message handlers that are common to all states. */
const struct ke_state_handler appm_default_handler = KE_STATE_HANDLER(appm_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t appm_state[APP_IDX_MAX];

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
