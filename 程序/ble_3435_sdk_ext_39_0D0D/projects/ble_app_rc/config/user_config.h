/**
 *******************************************************************************
 *
 * @file user_config.h
 *
 * @brief Application configuration definition
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *******************************************************************************
 */
 
#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_



 
 /******************************************************************************
  *############################################################################*
  * 							SYSTEM MACRO CTRL                              *
  *############################################################################*
  *****************************************************************************/

//如果需要使用GPIO进行调试，需要打开这个宏
#define GPIO_DBG_MSG					0
//UART使能控制宏
#define UART_PRINTF_EN					1
//蓝牙硬件调试控制
#define DEBUG_HW						0



/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 ******************************************************************************/
 
//连接参数更新控制
#define UPDATE_CONNENCT_PARAM  			1

//最小连接间隔 
#define BLE_UAPDATA_MIN_INTVALUE		6
//最大连接间隔 
#define BLE_UAPDATA_MAX_INTVALUE		10
//连接Latency
#define BLE_UAPDATA_LATENCY				180
//连接超时
#define BLE_UAPDATA_TIMEOUT				600

//设备名称
#define APP_HID_DEVICE_NAME				("BK3435RC")


/**
 * UUID List part of ADV Data
 * -----------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x12\x18 - HID Service UUID
 * -----------------------------------------------------------------------------
 */
 //广播包UUID配置
#define APP_HID_ADV_DATA_UUID       	"\x03\x03\x12\x18"
#define APP_HID_ADV_DATA_UUID_LEN   	(4)

/**
 * Appearance part of ADV Data
 * -----------------------------------------------------------------------------
 * x03 - Length
 * x19 - Appearance
 * x03\x00 - Generic Thermometer
 *   or
 * xC2\x03 - HID Mouse
 * xC1\x03 - HID Keyboard
 * xC3\x03 - HID Joystick
 * -----------------------------------------------------------------------------
 */
 //广播数据显示设置
#define APP_HID_ADV_DATA_APPEARANCE   	"\x03\x19\xC1\x03"
#define APP_ADV_DATA_APPEARANCE_LEN  	(4)

//扫描响应包数据
#define APP_SCNRSP_DATA        		"\x09\x08\x42\x4B\x33\x34\x33\x35\x52\x43" //BK3435RC"
#define APP_SCNRSP_DATA_LEN     	(10)


//广播参数配置
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           	(0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         	(160)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         	(160)
/// Fast advertising interval
#define APP_ADV_FAST_INT        	(32)


//打开这个宏将打开可变地址解析功能，即只允许绑定的设备建立连接，否则断开连接
#define BK_CONNNECT_FILTER_CTRL		 (0)
//白名单使能
#define APP_WHITE_LIST_ENABLE        1

#define HID_CONNECT_ANY				 (1)

/*******************************************************************************
 *#############################################################################*
 *								DRIVER MACRO CTRL                              *
 *#############################################################################*
 ******************************************************************************/

//DRIVER CONFIG
#define UART_DRIVER						1
#define GPIO_DRIVER						1
#define AUDIO_DRIVER					1
#define RTC_DRIVER						0
#define ADC_DRIVER						0
#define I2C_DRIVER						0
#define PWM_DRIVER						0






#endif /* _USER_CONFIG_H_ */
