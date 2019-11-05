/**
 ****************************************************************************************
 *
 * @file app_wechat.h
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
#ifndef APP_WECHAT_H_
#define APP_WECHAT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// fff0s Application Module Environment Structure
struct app_wechat_env_tag
{
    /// Connection handle
    uint8_t conidx;
};


#define PKT_LENGTH		120


typedef struct 
{
	uint8_t* buf;
	uint8_t len;
	uint8_t offset;
}WeChat_Recv_Pkt;

typedef struct 
{
	uint8_t buf[PKT_LENGTH];
	uint8_t len;
	uint8_t offset;
}WeChat_Send_Pkt;



/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// fff0s Application environment
extern struct app_wechat_env_tag app_wechat_env;

/// Table of message handlers
extern const struct ke_state_handler app_wechat_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize wechat Application Module
 ****************************************************************************************
 */
void app_wechat_init(void);
/**
 ****************************************************************************************
 * @brief Add a wechat Service instance in the DB
 ****************************************************************************************
 */
void app_wechat_add_wechat(void);
/**
 ****************************************************************************************
 * @brief Enable the wechat Service
 ****************************************************************************************
 */
void app_wechat_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a wechat data value
 ****************************************************************************************
 */
void app_wechat_send_notify(uint8_t* buf,uint8_t length);

void app_wechat_send_data(uint8_t* buf, uint8_t len);


#endif // APP_BATT_H_
