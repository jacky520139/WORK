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
 *******************************************************************************/
 
//���Ӳ������¿���
#define UPDATE_CONNENCT_PARAM  			1

//��С���Ӽ��
#define BLE_UAPDATA_MIN_INTVALUE		20
//������Ӽ�� 
#define BLE_UAPDATA_MAX_INTVALUE		40
//����Latency
#define BLE_UAPDATA_LATENCY				0
//���ӳ�ʱ
#define BLE_UAPDATA_TIMEOUT				600


//�豸����
#define APP_DFLT_DEVICE_NAME           ("3435-WeChat")

//WECHAT��غ궨��
#define DEVICE_TYPE  				   ("123456") 
#define DEVICE_ID 					   ("BK123456")
#define DEVICE_KEY 					   {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
#define DEF_MD5_ID					   {0x37,0x60,0x8c,0x00,0x79,0x22,0xba,0x35,0x80,0x21,0x53,0x22,0x91,0x6d,0x35,0x45};
//#define CATCH_LOG



 //�㲥��UUID����
#define APP_WECHAT_ADV_DATA_UUID        "\x03\x02\xE7\xFE"
#define APP_WECHAT_ADV_DATA_UUID_LEN    (4)


//�㲥��������
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         (80)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         (80)
/// Fast advertising interval
#define APP_ADV_FAST_INT        (32)



/*******************************************************************************
 *#############################################################################*
 *								DRIVER MACRO CTRL                              *
 *#############################################################################*
 ******************************************************************************/

//DRIVER CONFIG
#define UART_DRIVER						1
#define GPIO_DRIVER						1
#define AUDIO_DRIVER					0
#define RTC_DRIVER						0
#define ADC_DRIVER						0
#define I2C_DRIVER						0
#define PWM_DRIVER						0





#endif /* _USER_CONFIG_H_ */
