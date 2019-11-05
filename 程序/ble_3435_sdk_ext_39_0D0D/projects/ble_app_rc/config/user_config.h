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

//�����Ҫʹ��GPIO���е��ԣ���Ҫ�������
#define GPIO_DBG_MSG					0
//UARTʹ�ܿ��ƺ�
#define UART_PRINTF_EN					1
//����Ӳ�����Կ���
#define DEBUG_HW						0



/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 ******************************************************************************/
 
//���Ӳ������¿���
#define UPDATE_CONNENCT_PARAM  			1

//��С���Ӽ�� 
#define BLE_UAPDATA_MIN_INTVALUE		6
//������Ӽ�� 
#define BLE_UAPDATA_MAX_INTVALUE		10
//����Latency
#define BLE_UAPDATA_LATENCY				180
//���ӳ�ʱ
#define BLE_UAPDATA_TIMEOUT				600

//�豸����
#define APP_HID_DEVICE_NAME				("BK3435RC")


/**
 * UUID List part of ADV Data
 * -----------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x12\x18 - HID Service UUID
 * -----------------------------------------------------------------------------
 */
 //�㲥��UUID����
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
 //�㲥������ʾ����
#define APP_HID_ADV_DATA_APPEARANCE   	"\x03\x19\xC1\x03"
#define APP_ADV_DATA_APPEARANCE_LEN  	(4)

//ɨ����Ӧ������
#define APP_SCNRSP_DATA        		"\x09\x08\x42\x4B\x33\x34\x33\x35\x52\x43" //BK3435RC"
#define APP_SCNRSP_DATA_LEN     	(10)


//�㲥��������
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           	(0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         	(160)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         	(160)
/// Fast advertising interval
#define APP_ADV_FAST_INT        	(32)


//������꽫�򿪿ɱ��ַ�������ܣ���ֻ����󶨵��豸�������ӣ�����Ͽ�����
#define BK_CONNNECT_FILTER_CTRL		 (0)
//������ʹ��
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
