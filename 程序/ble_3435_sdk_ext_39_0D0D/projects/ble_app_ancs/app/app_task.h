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

/// Number of APP Task Instances应用程序任务实例数
#define APP_IDX_MAX                 (1)


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// States of APP task
enum appm_state
{
    /// Initialization state初始化状态
    APPM_INIT,
    /// Database create state建立数据库状态
    APPM_CREATE_DB,
    /// Ready State       就绪状态
    APPM_READY,          
    /// Advertising state广播状态
    APPM_ADVERTISING,
    /// Connected state连接状态
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
  APP_ADV_ENABLE_TIMER,	/*APP开始广播*/
	APP_GET_RSSI_TIMER,
	APP_LED_CTRL_SCAN,
	USER_APP_CALENDAR_UPDATE,/*APP日历更新*/
	SL_SC7A21_CLICK_TIMER,/*敲击计数器*/
	SL_SC7A21_GET_PEDO,/*获取计步值*/
	USER_ALARM_IND,//闹钟提醒
	USER_MOTOR_PERIOD_TIMER,//电机振动延时

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
