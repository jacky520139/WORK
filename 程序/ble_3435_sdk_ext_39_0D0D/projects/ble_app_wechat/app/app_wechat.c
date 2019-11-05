/**
 ****************************************************************************************
 *
 * @file app_wechat.c
 *
 * @brief wechat Application Module entry point
 *
 * @auth  hai.xu
 *
 * @date  2017.08.17
 *
 * Copyright (C) Beken 2009-2016
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

#include "rwip_config.h"     // SW configuration


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_wechat.h"              // Battery Application Module Definitions
#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "wechat_task.h"             // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "wechat.h"
#include "wechat_util.h"
#include "ke_timer.h"
#include "mpbledemo2.h"
#include "ke_mem.h"
#include "uart.h"


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

static void app_wechat_recvCb(uint8_t event, uint8_t *buf, uint8_t len);

/// fff0 Application Module Environment Structure
struct app_wechat_env_tag app_wechat_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_wechat_init(void)
{
    // Reset the environment
    memset(&app_wechat_env, 0, sizeof(struct app_wechat_env_tag));
	wechat_register_Cb(app_wechat_recvCb);
}

void app_wechat_add_wechat(void)
{

   struct wechat_db_cfg *db_cfg;
		
   struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct wechat_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_WECHAT;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	 
    // Set parameters
    db_cfg = (struct wechat_db_cfg* ) req->param;
	 
    // Sending of notifications is supported
    db_cfg->features = WECHAT_INDICATE_SUP;
    // Send the message
    ke_msg_send(req);
}


static WeChat_Send_Pkt g_send_wechat;
static uint8_t sendSize = 0;

void app_send_indicate( void )
{
	sendSize = 0;

	if( g_send_wechat.offset < g_send_wechat.len )
	{
		if( g_send_wechat.len - g_send_wechat.offset > 20 )
		{
			sendSize = 20;
		}else
		{
			sendSize = g_send_wechat.len - g_send_wechat.offset;
		}

		app_wechat_send_notify(&g_send_wechat.buf[g_send_wechat.offset], sendSize );
	}else
	{
		g_send_wechat.len = 0;
		g_send_wechat.offset = 0;
		memset(g_send_wechat.buf, 0, PKT_LENGTH);
	}
}


void app_wechat_send_data(uint8_t* buf, uint8_t len)
{
	g_send_wechat.len = 0;
	g_send_wechat.offset = 0;
	g_send_wechat.len = len;
	memset( g_send_wechat.buf, 0, PKT_LENGTH );

	memcpy(g_send_wechat.buf, buf, len );
	
	app_send_indicate();
}


void app_wechat_send_notify(uint8_t* buf,uint8_t length)
{
    // Allocate the message
    struct wechat_ind_upd_req * req = KE_MSG_ALLOC(WECHAT_INDICATE_UPD_REQ,
                                                prf_get_task_from_id(TASK_ID_WECHAT),
                                                TASK_APP,
                                                wechat_ind_upd_req);

    // Fill in the parameter structure	
    req->length = length;
    memcpy(req->indicate_buf, buf, length);
		

    // Send the message
    ke_msg_send(req);
}


static int app_wechat_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                               struct wechat_indi_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	UART_PRINTF("ntfg = 0x%x\r\n", param->indi_cfg);
	if(param->indi_cfg == PRF_CLI_STOP_NTFIND)
	{
		UART_PRINTF("indi disable\r\n");
		wechat_set_indicate_state(0);
	}
	else
	{
		UART_PRINTF("indi enable\r\n");
		if(ke_state_get(TASK_APP) == APPM_CONNECTED)
		{
			wechat_Init();
			wechat_set_indicate_state(1);
			wechat_process();
		}
	}
	
    return (KE_MSG_CONSUMED);
}


static void app_wechat_recvCb(uint8_t event, uint8_t *buf, uint8_t len)
{
	UART_PRINTF("recv data wechat\r\n");
	UART_PRINTF("recv event: %d\r\n", event);
	UART_PRINTF("recv length: %d\r\n", len);
	UART_PRINTF("recv data: ");
	for(uint8_t i=0; i<len; i++)
	{
		UART_PRINTF("0x%x ", buf[i]);
	}
	UART_PRINTF("\r\n");
}


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
static int app_wechat_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    // Drop the message
    return (KE_MSG_CONSUMED);
}

static WeChat_Recv_Pkt  g_recv_data = {NULL, 0, 0};

static int app_wechat_write_req_handler(ke_msg_id_t const msgid,
                                     struct wechat_write_ind *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	int chunk_size = 0;

    // Drop the message
	UART_PRINTF("WeChat Write\r\n");
    if(ke_state_get(TASK_APP) == APPM_CONNECTED)
    {
		if(param->length <= 20)
		{
			if( g_recv_data.len == 0 )
			{
				BpFixHead *fix_head = (BpFixHead *)param->write_buf;
				g_recv_data.len = ntohs( fix_head->nLength );
				g_recv_data.offset = 0;
				g_recv_data.buf = ke_malloc(g_recv_data.len, KE_MEM_KE_MSG);
			}
			chunk_size = g_recv_data.len - g_recv_data.offset;
			chunk_size = chunk_size < param->length ? chunk_size : param->length;
			memcpy( g_recv_data.buf + g_recv_data.offset, param->write_buf, chunk_size );
			g_recv_data.offset += chunk_size;
			if( g_recv_data.len <= g_recv_data.offset )
			{
				wechat_consume( g_recv_data.buf, g_recv_data.len );
				ke_free(g_recv_data.buf);
				g_recv_data.buf = NULL;
				g_recv_data.len = 0;
				g_recv_data.offset = 0;
			}
		}
	}
		
    return (KE_MSG_CONSUMED);
}



static int app_wechat_gattc_cmp_evt_handler(ke_msg_id_t const msgid,  
										struct gattc_cmp_evt const *param,
                                 		ke_task_id_t const dest_id, 
                                 		ke_task_id_t const src_id)
{
	if(param->operation == GATTC_INDICATE)
	{
		UART_PRINTF("recv cfm\r\n");
		
		g_send_wechat.offset += sendSize;

		app_send_indicate();
	}
	
    return KE_MSG_CONSUMED;
}



/// Default State handlers definition
const struct ke_msg_handler app_wechat_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,       (ke_msg_func_t)app_wechat_msg_dflt_handler},
    {WECHAT_INDICATE_IND_CFG_IND,  (ke_msg_func_t)app_wechat_ntf_cfg_ind_handler},
    {WECHAT_WRITER_REQ_IND,		   (ke_msg_func_t)app_wechat_write_req_handler},
    {WECHAT_GATTC_CMP_EVT,         (ke_msg_func_t)app_wechat_gattc_cmp_evt_handler},
};

const struct ke_state_handler app_wechat_table_handler =
    {&app_wechat_msg_handler_list[0], (sizeof(app_wechat_msg_handler_list)/sizeof(struct ke_msg_handler))};



