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
//使用RC计时校正时间,打开校准同时需要打开PWM_DRIVER这个宏
#define RC_CALIBRATE                    0



 
/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 *******************************************************************************/
 
//连接参数更新控制
#define UPDATE_CONNENCT_PARAM  			1
//获取连接信号强度
#define APP_GET_RSSI_EN             0

//最小连接间隔
#define BLE_UAPDATA_MIN_INTVALUE		20//必须是“0.625ms”的整数倍
//最大连接间隔 
#define BLE_UAPDATA_MAX_INTVALUE		40//必须是“0.625ms”的整数倍
//连接Latency
#define BLE_UAPDATA_LATENCY				0
//连接超时
#define BLE_UAPDATA_TIMEOUT				600


//设备名称
#define APP_DFLT_DEVICE_NAME           ("YAZOLE_TEXT")


 //广播包UUID配置
#define APP_FFF0_ADV_DATA_UUID        "\x03\x03\x0E\x18"//0x1812
#define APP_FFF0_ADV_DATA_UUID_LEN    (4)

//扫描响应包数据
//#define APP_SCNRSP_DATA        "\x0c\x08\x42\x4B\x33\x34\x34\x35\x2D\x41\x4E\x43\x53" //BK3435-ANCS"
#define APP_SCNRSP_DATA          "\x0c\x08\x59\x41\x5A\x4F\x4C\x45\x5F\x54\x45\x58\x54" //BK3435-GATT"
#define APP_SCNRSP_DATA_LEN     (13)


//广播参数配置
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)必须是“0.625ms”的整数倍,范围是“20ms ~ 10.24s”之间
#define APP_ADV_INT_MIN         (160)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         (160)
/// Fast advertising interval
#define APP_ADV_FAST_INT        (32)

//打开这个宏将打开可变地址解析功能，即只允许绑定的设备建立连接，否则断开连接
#define BK_CONNNECT_FILTER_CTRL		 0
//白名单使能
#define APP_WHITE_LIST_ENABLE        1


/*******************************************************************************
 *#############################################################################*
 *								DRIVER MACRO CTRL                              *
 *#############################################################################*
 ******************************************************************************/

//DRIVER CONFIG
#define UART_DRIVER						1
#define GPIO_DRIVER						1
#define AUDIO_DRIVER					0
#define RTC_DRIVER						1
#define ADC_DRIVER						1
#define I2C_DRIVER						0
#define PWM_DRIVER						1





#endif /* _USER_CONFIG_H_ */
