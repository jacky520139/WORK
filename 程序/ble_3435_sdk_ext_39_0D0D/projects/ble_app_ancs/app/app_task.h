/**
 ****************************************************************************************
 *
 * @file app_task.h
 *
 * @brief Header file - APPTASK.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_TASK_H_
#define APP_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPTASK Task
 * @ingroup APP
 * @brief Routes ALL messages to/from APP block.
 *
 * The APPTASK is the block responsible for bridging the final application with the
 * RWBLE software host stack. It communicates with the different modules of the BLE host,
 * i.e. @ref SMP, @ref GAP and @ref GATT.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"    // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>         // Standard Integer
#include "rwip.h"
#include "rwip_task.h"      // Task definitions
#include "ke_task.h"        // Kernel Task

/*
 * DEFINES
 ****************************************************************************************
 */

/// Number of APP Task InstancesӦ�ó�������ʵ����
#define APP_IDX_MAX                 (1)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of APP task
enum appm_state
{
    /// Initialization state��ʼ��״̬
    APPM_INIT,
    /// Database create state�������ݿ�״̬
    APPM_CREATE_DB,
    /// Ready State       ����״̬
    APPM_READY,          
    /// Advertising state�㲥״̬
    APPM_ADVERTISING,
    /// Connected state����״̬
    APPM_CONNECTED,
    /// Number of defined states.
    APPM_STATE_MAX
};


/// APP Task messages
enum appm_msg
{
    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),
		
    APP_PARAM_UPDATE_REQ_IND,

	APP_SEND_SECURITY_REQ,
	
	APP_ANCS_REQ_IND,
	
	APP_PERIOD_TIMER,
	APP_ADV_TIMEOUT_TIMER,
  APP_ADV_ENABLE_TIMER,	/*APP��ʼ�㲥*/
	APP_GET_RSSI_TIMER,
	APP_LED_CTRL_SCAN,
	USER_APP_CALENDAR_UPDATE,/*APP��������*/
	SL_SC7A21_CLICK_TIMER,/*�û�������*/
	SL_SC7A21_GET_PEDO,/*��ȡ�Ʋ�ֵ*/
	USER_ALARM_IND,//��������
	USER_MOTOR_PERIOD_TIMER,//�������ʱ

};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler appm_default_handler;
extern ke_state_t appm_state[APP_IDX_MAX];


/// @} APPTASK

#endif //(BLE_APP_PRESENT)

#endif // APP_TASK_H_
