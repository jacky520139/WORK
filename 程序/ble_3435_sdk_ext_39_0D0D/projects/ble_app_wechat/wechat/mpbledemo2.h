
#ifndef __WECHATAPP_H__
#define __WECHATAPP_H__


#include "epb_MmBp.h"
#include "wechat_util.h"
#include "wechat_task.h"
#include "wechat.h"
#include "app_task.h"
#include "mpbledemo2.h"
#include "user_config.h"


typedef enum 
{
	CMD_NULL = 0,
	CMD_AUTH,
	CMD_AUTH_COMPLETE,
	CMD_INIT,
	CMD_INIT_COMPLETE,
	CMD_SENDDAT,
	CMD_PUSH,
	CMD_SWITH_VIEW,
	CMD_SWITCH_BACKGROUP,
}wdp_cmd_t;



#define PROTO_VERSION 0x010004
#define AUTH_PROTO 1

#define MAC_ADDRESS_LENGTH 6

//#define EAM_md5AndNoEnrypt 1     //认证方式只能定义其中的一种
//#define EAM_md5AndAesEnrypt 1
#define EAM_macNoEncrypt 2



#ifdef EAM_macNoEncrypt
	#define AUTH_METHOD EAM_macNoEncrypt
	#define MD5_TYPE_AND_ID_LENGTH 0
	#define CIPHER_TEXT_LENGTH 0
#endif

#ifdef EAM_md5AndAesEnrypt
	#define AUTH_METHOD EAM_md5AndAesEnrypt
	#define MD5_TYPE_AND_ID_LENGTH 16
	#define CIPHER_TEXT_LENGTH 16
#endif
#ifdef EAM_md5AndNoEnrypt
	#define AUTH_METHOD EAM_md5AndNoEnrypt
	#define MD5_TYPE_AND_ID_LENGTH 16
	#define CIPHER_TEXT_LENGTH 0
#endif

#define CHALLENAGE_LENGTH 4

#define MPBLEDEMO2_MAGICCODE_H 0xfe
#define MPBLEDEMO2_MAGICCODE_L 0xcf
#define MPBLEDEMO2_VERSION 0x01

#define SEND_HELLO_WECHAT "Hello, WeChat!"



typedef enum
{
	errorCodeUnpackAuthResp = 0x9990,
	errorCodeUnpackInitResp = 0x9991,
	errorCodeUnpackSendDataResp = 0x9992,
	errorCodeUnpackCtlCmdResp = 0x9993,
	errorCodeUnpackRecvDataPush = 0x9994,
	errorCodeUnpackSwitchViewPush = 0x9995,
	errorCodeUnpackSwitchBackgroundPush = 0x9996,
	errorCodeUnpackErrorDecode = 0x9997,
}mpbledemo2UnpackErrorCode;
typedef enum
{
	errorCodeProduce = 0x9980,
}mpbledemo2PackErrorCode;
typedef enum
{
	sendTextReq = 0x01,
	sendTextResp = 0x1001,
	openLightPush = 0x2001,
	closeLightPush = 0x2002,
}BleDemo2CmdID;

typedef struct
{
	uint8_t m_magicCode[2];
	uint16_t m_version;
	uint16_t m_totalLength;
	uint16_t m_cmdid;
	uint16_t m_seq;
	uint16_t m_errorCode;
}BlueDemoHead;

typedef struct 
{
	int cmd;
	xCString send_msg;
} mpbledemo2_info;

 
 typedef struct
 {
	 uint8_t *data;
	 uint16_t len;
	 uint16_t offset;
 } data_info;


 typedef struct 
{
	bool wechats_switch_state; //公众账号切换到前台的状态
	bool indication_state;
	bool auth_state;
	bool init_state;
	bool auth_send;
	bool init_send;
	unsigned short send_data_seq;
	unsigned short push_data_seq;
	unsigned short seq; 
}wdp_state_t;



typedef void (*we_chat_RecvCB_t)(uint8_t event, uint8_t *buf, uint8_t len);

extern void wechat_clear( void );

extern void wechat_process(void);

extern void wechat_set_indicate_state(uint8_t state);

extern void wechat_Init(void);

extern int wechat_consume(uint8_t *data, uint32_t len);

uint16_t wechat_send_pack( uint8_t* pkg, uint16_t length );

void wechat_register_Cb(we_chat_RecvCB_t  recv_cb);

#endif
