/**
 ****************************************************************************************
 *
 * @file app_fee0.c
 *
 * @brief fee0 Application Module entry point
 *
 * @auth  hai.xu
 *
 * @date  2016.05.31
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
#include "app_fee0.h"              // Battery Application Module Definitions
#include "app.h"                   // Application Definitions
#include "app_task.h"              // application task definitions
#include "fee0s_task.h"            // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"             // Profile common types definition
#include "arch.h"                  // Platform Definitions
#include "prf.h"
#include "fee0s.h"
#include "ke_timer.h"
#include "uart.h"



/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// fff0 Application Module Environment Structure
struct app_fee0_env_tag app_fee0_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


void app_fee0_init(void)
{

    // Reset the environment
    memset(&app_fee0_env, 0, sizeof(struct app_fee0_env_tag));
}

void app_fee0_add_fee0s(void)
{

   struct fee0s_db_cfg *db_cfg;
		
   struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct fee0s_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl =   0;
    req->prf_task_id = TASK_ID_FEE0S;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	 
    // Set parameters
    db_cfg = (struct fee0s_db_cfg* ) req->param;
	 
    // Sending of notifications is supported
    db_cfg->features = FEE0_FEE1_LVL_NTF_SUP | FEE0_FEE3_LVL_NTF_SUP;
    // Send the message
    ke_msg_send(req);
}


void app_fee1_send_lvl(uint8_t* buf, uint8_t len)
{
    // Allocate the message
    struct fee0s_fee1_level_upd_req * req = KE_MSG_ALLOC(FEE0S_FEE1_LEVEL_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FEE0S),
                                                        TASK_APP,
                                                        fee0s_fee1_level_upd_req);
    // Fill in the parameter structure	
    req->length = len;
	memcpy(req->fee1_level, buf, len);

    // Send the message
    ke_msg_send(req);
}

void app_fee3_send_lvl(uint8_t* buf, uint8_t len)
{
    // Allocate the message
    struct fee0s_fee3_level_upd_req * req = KE_MSG_ALLOC(FEE0S_FEE3_LEVEL_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FEE0S),
                                                        TASK_APP,
                                                        fee0s_fee3_level_upd_req);
    // Fill in the parameter structure	
    req->length = len;
	memcpy(req->fee3_level, buf, len);

    // Send the message
    ke_msg_send(req);
}


static int fee0s_fee1_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                               struct fee0s_fee1_level_ntf_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	UART_PRINTF("fee1->param->ntf_cfg = %x\r\n",param->ntf_cfg);
	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		//ke_timer_clear(FEE0S_FEE1_LEVEL_PERIOD_NTF,dest_id);
	}else
	{
		//ke_timer_set(FEE0S_FEE1_LEVEL_PERIOD_NTF,dest_id , 100);
	}
    
    return (KE_MSG_CONSUMED);
}

static int fee0s_fee3_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                               struct fee0s_fee3_level_ntf_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	UART_PRINTF("fee3->param->ntf_cfg = %x\r\n",param->ntf_cfg);
	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		//ke_timer_clear(FEE0S_FEE3_LEVEL_PERIOD_NTF,dest_id);
	}else
	{
		//ke_timer_set(FEE0S_FEE3_LEVEL_PERIOD_NTF,dest_id , 100);
	}
    
    return (KE_MSG_CONSUMED);
}
static int fee1_level_upd_handler(ke_msg_id_t const msgid,
                                      struct fee0s_fee1_level_upd_rsp const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);
	
	if(param->status == GAP_ERR_NO_ERROR)
	{
		//uint8_t buf[128];
		//memset(buf, 0xcc, 128);
		//app_fee1_send_lvl(buf, 128);
	}
	
    return (KE_MSG_CONSUMED);
}

static int fee3_level_upd_handler(ke_msg_id_t const msgid,
                                      struct fee0s_fee3_level_upd_rsp const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);
	
	if(param->status == GAP_ERR_NO_ERROR)
	{
		//uint8_t buf[128];
		//memset(buf, 0xcc, 128);
		//app_fee3_send_lvl(buf, 128);
	}
	
    return (KE_MSG_CONSUMED);
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
static int app_fee0_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);
	
    // Drop the message
    return (KE_MSG_CONSUMED);
}


static int fee2_writer_req_handler(ke_msg_id_t const msgid,
                                     struct fee0s_fee2_writer_ind *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    // Drop the message
	UART_PRINTF("FEE2 param->value = 0x ");
	
	for(uint8_t i = 0;i < param->length;i++)
	{
		UART_PRINTF("%02x ",param->fee2_value[i]);
	}
	UART_PRINTF("\r\n");
		
    return (KE_MSG_CONSUMED);
}


static int fee1_period_ntf_handler(ke_msg_id_t const msgid,
                                               struct fee0s_fee1_level_ntf_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	uint8_t buf[20];
	memset(buf, 0xff, 20);
	app_fee1_send_lvl(buf, 20);
    //ke_timer_set(FEE0S_FEE1_LEVEL_PERIOD_NTF,dest_id , 100);
		
    return (KE_MSG_CONSUMED);
}

static int fee3_period_ntf_handler(ke_msg_id_t const msgid,
                                               struct fee0s_fee3_level_ntf_cfg_ind const *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
	uint8_t buf[20];
	memset(buf, 0xcc, 20);
	app_fee3_send_lvl(buf, 20);
    //ke_timer_set(FEE0S_FEE3_LEVEL_PERIOD_NTF,dest_id , 100);
		
    return (KE_MSG_CONSUMED);
}



/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_fee0_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_fee0_msg_dflt_handler},
    {FEE0S_FEE1_LEVEL_NTF_CFG_IND,  (ke_msg_func_t)fee0s_fee1_level_ntf_cfg_ind_handler},
    {FEE0S_FEE3_LEVEL_NTF_CFG_IND,  (ke_msg_func_t)fee0s_fee3_level_ntf_cfg_ind_handler},
    {FEE0S_FEE1_LEVEL_UPD_RSP,      (ke_msg_func_t)fee1_level_upd_handler},
    {FEE0S_FEE3_LEVEL_UPD_RSP,      (ke_msg_func_t)fee3_level_upd_handler},
    {FEE0S_FEE2_WRITER_REQ_IND,		(ke_msg_func_t)fee2_writer_req_handler},
    {FEE0S_FEE1_LEVEL_PERIOD_NTF,	(ke_msg_func_t)fee1_period_ntf_handler},
    {FEE0S_FEE3_LEVEL_PERIOD_NTF,	(ke_msg_func_t)fee3_period_ntf_handler},
};

const struct ke_state_handler app_fee0_table_handler =
    {&app_fee0_msg_handler_list[0], (sizeof(app_fee0_msg_handler_list)/sizeof(struct ke_msg_handler))};


