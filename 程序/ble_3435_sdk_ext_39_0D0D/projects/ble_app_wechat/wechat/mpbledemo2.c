/*
 *	WeChat mpbledemo2
 *
 *	author by anqiren  2014/12/02  V1.0bat
 *
 **/
#include "rwip_config.h"
#include "epb_MmBp.h"
#include "wechat_util.h"
#include "crc32.h"
#include "AES.h"
#include "mpbledemo2.h"
#include "app_task.h"
#include "wechat_task.h"
#include "wechat.h"
#include "app_wechat.h"
#include "ke_config.h"
#include "md5.h"
#include "co_utils.h"
#include "ke_mem.h"
#include "user_config.h"
#include "uart.h"



mpbledemo2_info m_info = { CMD_NULL, { NULL, 0 } };

#if defined EAM_md5AndNoEnrypt || EAM_md5AndAesEnrypt
uint8_t md5_type_and_id[16];
#endif

uint8_t	challeange[CHALLENAGE_LENGTH] = { 0x11, 0x22, 0x33, 0x44 }; //为了方便这里填了一组确定的数组，使用过程中请自行生成随机数



static wdp_state_t	wd_process_state = { false, false, false, false, false, false, 0, 0, 0 };

const uint8_t	key[16] = DEVICE_KEY;

uint8_t	session_key[16] = { 0 };

static we_chat_RecvCB_t pWechat_recCb;

static uint16_t device_init( void );


static int32_t device_auth( void );


static int32_t wechat_get_md5( void );

uint16_t device_send_test( void );

static void wechat_data_produce( void *args, uint8_t **r_data, uint32_t *r_len );



/*----------------------------------------------------------------------------*
 *  NAME
 *    wechat_get_md5
 *  DESCRIPTION
 *
 *  PARAMETERS
 *
 *  RETURNS
 *---------------------------------------------------------------------------*/
int32_t wechat_get_md5( void )
{
	int32_t error_code = 0;

#if 0//defined EAM_md5AndNoEnrypt || defined EAM_md5AndAesEnrypt
	memcpy( md5_type_and_id, DEF_MD5_ID, 16 );
#endif


#if defined EAM_md5AndNoEnrypt || EAM_md5AndAesEnrypt
	char device_type[] = DEVICE_TYPE;
	char device_id[] = DEVICE_ID;
	char argv[sizeof( DEVICE_TYPE ) + sizeof( DEVICE_ID ) - 1];
	memcpy( argv, device_type, sizeof( DEVICE_TYPE ) );
	
	/*when add the DEVICE_ID to DEVICE_TYPE, the offset shuld -1 to overwrite '\0'  at the end of DEVICE_TYPE */
	memcpy( argv + sizeof( DEVICE_TYPE ) - 1, device_id, sizeof( DEVICE_ID ) );
#ifdef CATCH_LOG	
	UART_PRINTF ( "\r\nDEVICE_TYPE and DEVICE_ID:%s\r\n",argv);
#endif	
	error_code = md5( argv, md5_type_and_id );
#ifdef CATCH_LOG	
	UART_PRINTF("\r\nMD5: ");
	for ( uint8_t i = 0; i < 16; i++ )
	UART_PRINTF ( " %02x", md5_type_and_id[i] );
	UART_PRINTF ( "\r\n");
#endif
#endif
	
	return error_code;
}


/*----------------------------------------------------------------------------*
 *  NAME  device_auth
 *
 *  DESCRIPTION
 *
 *  PARAMETERS
 *
 *  RETURNS
 *
 *---------------------------------------------------------------------------*/
static int32_t device_auth( void )
{
	uint8_t			*data = NULL;
	uint32_t		len = 0;

	mpbledemo2_info cmd_disponse_arg = { CMD_AUTH, { NULL, 0 } };
	mpbledemo2_info *m_data_produce_args = &cmd_disponse_arg;
	wechat_data_produce( m_data_produce_args, &data, &len );
	if( data == NULL )
	{
		UART_PRINTF("data null\r\n");
		return errorCodeProduce;
	}
#ifdef CATCH_LOG
	UART_PRINTF("\r\nauth send! ");
#endif
#ifdef CATCH_LOG
	UART_PRINTF("\r\n##send data: ");
	uint8_t *d = data;
	for(uint8_t i=0;i<len;++i){
	UART_PRINTF(" %x",d[i]);}
	BpFixHead *fix_head = (BpFixHead *)data;
	UART_PRINTF("\r\n CMDID: %d", ntohs(fix_head->nCmdId));
	UART_PRINTF("\r\n len: %d", ntohs(fix_head->nLength ));
	UART_PRINTF("\r\n Seq: %d", ntohs(fix_head->nSeq));
	#endif
	//sent data
	app_wechat_send_data(data, len);
	ke_free( data );
	return 0;
}

static uint16_t device_init( void )
{
	uint8_t			*data = NULL;
	uint32_t			len = 0;

	mpbledemo2_info cmd_disponse_arg = { CMD_INIT, { NULL, 0 } };
	mpbledemo2_info *m_data_produce_args = &cmd_disponse_arg;
	wechat_data_produce( m_data_produce_args, &data, &len );
	if( data == NULL )
	{
		return errorCodeProduce;
	}
	//sent data
	app_wechat_send_data(data, len);
	ke_free( data );
#ifdef CATCH_LOG
	UART_PRINTF("\r\ninit send! ");
#endif
#ifdef CATCH_LOG
	UART_PRINTF("\r\n##send data: ");
	uint8_t *d = data;
	for(uint8_t i=0;i<len;++i){
	UART_PRINTF(" %x",d[i]);}
	BpFixHead *fix_head = (BpFixHead *)data;
	UART_PRINTF("\r\n CMDID: %d", ntohs(fix_head->nCmdId));
	UART_PRINTF("\r\n len: %d", ntohs(fix_head->nLength ));
	UART_PRINTF("\r\n Seq: %d", ntohs(fix_head->nSeq));
#endif

	return 0;
}

void mac_convert(uint8_t* mac)
{
	uint8_t *d = co_default_bdaddr.addr;
	for( uint8_t i = 6; i > 0; )
	{
		i--;
		mac[5 - i] = d[i];
	}
}


void wechat_register_Cb(we_chat_RecvCB_t  recv_cb)
{
	if(recv_cb)
	{
		pWechat_recCb = recv_cb;
	}
}


static void wechat_data_produce( void *args, uint8_t **r_data, uint32_t *r_len )
{
	static uint16_t	bleDemoHeadLen = sizeof( BlueDemoHead );
	mpbledemo2_info *info = (mpbledemo2_info *)args;
	BaseRequest		basReq = { NULL };
	static uint8_t	fix_head_len = sizeof( BpFixHead );
	BpFixHead		fix_head = { 0xFE, 1, 0, ECI_req_auth, 0 };
	wd_process_state.seq++;
	switch( info->cmd )
	{
		case CMD_AUTH:
		{
		#if defined EAM_md5AndAesEnrypt
			uint8_t			deviceid[] = DEVICE_ID;
			static uint32_t	seq = 0x00000001;   //
			uint32_t			ran = 0x11223344;   //为了方便起见这里放了一个固定值做为随机数，在使用时请自行生成随机数。
			ran = t_htonl( ran );
			seq = t_htonl( seq );
			
			uint8_t	id_len = strlen( DEVICE_ID );
			uint8_t	* data = ke_malloc( id_len + 8,KE_MEM_KE_MSG );
			if( !data )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			memcpy( data, deviceid, id_len );
			memcpy( data + id_len, (uint8_t*)&ran, 4 );
			memcpy( data + id_len + 4, (uint8_t*)&seq, 4 );
			uint32_t crc = crc32( 0, data, id_len + 8 );
			crc = t_htonl( crc );
			memset( data, 0x00, id_len + 8 );
			memcpy( data, (uint8_t*)&ran, 4 );
			memcpy( data + 4, (uint8_t*)&seq, 4 );
			memcpy( data + 8, (uint8_t*)&crc, 4 );			
			uint8_t CipherText[16];
			AES_Init( key );
		#ifdef CATCH_LOG		
			uart_printf("src_data:\r\n");
			for(uint8_t i=0; i<12; i++)
			{
				uart_printf("0x%x ", data[i]);
			}
			uart_printf("\r\n");
		#endif
			AES_Encrypt_PKCS7( data, CipherText, 12, key );
		#ifdef CATCH_LOG	
			uart_printf("CipherText:\r\n");
			for(uint8_t i=0; i<16; i++)
			{
				uart_printf("0x%x ", CipherText[i]);
			}
			uart_printf("\r\n");
		#endif		
		#ifdef CATCH_LOG	
			uint8_t buf[12];
			AES_Decrypt(buf, CipherText, 16, key);
			uart_printf("buf:\r\n");
			for(uint8_t i=0; i<12; i++)
			{
				uart_printf("0x%x ", buf[i]);
			}
			uart_printf("\r\n");
		#endif	
			if( data )
			{
				ke_free( data );
				data = NULL;
			}
	
			AuthRequest authReq = { &basReq, true, { md5_type_and_id, MD5_TYPE_AND_ID_LENGTH }, 
				PROTO_VERSION, AUTH_PROTO, (EmAuthMethod)AUTH_METHOD, true, { CipherText, 
				CIPHER_TEXT_LENGTH }, false, { NULL, 0 }, false, { NULL, 0 }, false, { NULL, 0 }, true, { DEVICE_ID, sizeof( DEVICE_ID ) } };
			seq++;
		#endif

		#if defined EAM_macNoEncrypt
			static uint8_t mac_address[6];
			mac_convert(mac_address);
			AuthRequest authReq = { &basReq, false, { NULL, 0 }, PROTO_VERSION, AUTH_PROTO, 
				(EmAuthMethod)AUTH_METHOD, false, { NULL, 0 }, true, { mac_address,
				MAC_ADDRESS_LENGTH }, false, { NULL, 0 }, false, { NULL, 0 }, true, { DEVICE_ID, sizeof( DEVICE_ID ) } };
		#endif

		#if defined EAM_md5AndNoEnrypt
			AuthRequest authReq = { &basReq, true, { md5_type_and_id, MD5_TYPE_AND_ID_LENGTH },
			PROTO_VERSION, (EmAuthMethod)AUTH_PROTO, (EmAuthMethod)AUTH_METHOD, false, 
			{ NULL, 0 }, false, { NULL, 0 }, false, { NULL, 0 }, false, { NULL, 0 }, true, { DEVICE_ID, sizeof( DEVICE_ID ) } };
		#endif
			*r_len = epb_auth_request_pack_size( &authReq ) + fix_head_len;
			*r_data = (uint8_t *)ke_malloc( (*r_len),KE_MEM_KE_MSG );
			if( !( *r_data ) )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			if( epb_pack_auth_request( &authReq, *r_data + fix_head_len, *r_len - fix_head_len ) < 0 )
			{
				*r_data = NULL;
				return;
			}
			fix_head.nCmdId = htons( ECI_req_auth );
			fix_head.nLength = htons( *r_len );
			fix_head.nSeq = htons( wd_process_state.seq );
			memcpy( *r_data, &fix_head, fix_head_len );
			return;
		}
		case CMD_INIT:
		{
			//has challeange
			InitRequest initReq = { &basReq, false, { NULL, 0 }, true, { challeange, CHALLENAGE_LENGTH } };
			*r_len = epb_init_request_pack_size( &initReq ) + fix_head_len;
		#if defined EAM_md5AndAesEnrypt
			uint8_t	length = *r_len;
			uint8_t	*p = ke_malloc( AES_get_length( *r_len - fix_head_len ),KE_MEM_KE_MSG );
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			*r_len = AES_get_length( *r_len - fix_head_len ) + fix_head_len;
		#endif
			//pack data
			*r_data = (uint8_t *)ke_malloc( (*r_len),KE_MEM_KE_MSG );
			if( !( *r_data ) )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			if( epb_pack_init_request( &initReq, *r_data + fix_head_len, *r_len - fix_head_len ) < 0 )
			{
				*r_data = NULL; 
				return;
			}
			//encrypt body
		#if defined EAM_md5AndAesEnrypt
			AES_Init( session_key );
			AES_Encrypt_PKCS7( *r_data + fix_head_len, p, length - fix_head_len, session_key ); //原始数据长度
			memcpy( *r_data + fix_head_len, p, *r_len - fix_head_len );
			if( p )
			{
				ke_free( p );
			}
		#endif
			fix_head.nCmdId = htons( ECI_req_init );
			fix_head.nLength = htons( *r_len );
			fix_head.nSeq = htons( wd_process_state.seq );
			memcpy( *r_data, &fix_head, fix_head_len );
			return;
		}
		case CMD_SENDDAT:
		{
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n msg to send : %s",(uint8_t*)info->send_msg.str);
		#endif
			BlueDemoHead *bleDemoHead = (BlueDemoHead*)ke_malloc( (bleDemoHeadLen + info->send_msg.len),KE_MEM_KE_MSG );
			if( !bleDemoHead )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			bleDemoHead->m_magicCode[0] = MPBLEDEMO2_MAGICCODE_H;
			bleDemoHead->m_magicCode[1] = MPBLEDEMO2_MAGICCODE_L;
			bleDemoHead->m_version = htons( MPBLEDEMO2_VERSION );
			bleDemoHead->m_totalLength = htons( bleDemoHeadLen + info->send_msg.len );
			bleDemoHead->m_cmdid = htons( sendTextReq );
			bleDemoHead->m_seq = htons( wd_process_state.seq );
			bleDemoHead->m_errorCode = 0;
			/*connect body and head.*/
			/*turn to uint8_t* befort offset.*/
			memcpy( (uint8_t*)bleDemoHead + bleDemoHeadLen, info->send_msg.str, info->send_msg.len );
			//SendDataRequest sendDatReq = {&basReq, {(uint8_t*) bleDemoHead, (bleDemoHeadLen + info->send_msg.len)}, 0, (EmDeviceDataType)NULL};
			//SendDataRequest sendDatReq = { &basReq, { (uint8_t*)bleDemoHead, ( bleDemoHeadLen + info->send_msg.len ) }, 1, (EmDeviceDataType)EDDT_wxDeviceHtmlChatView };
			SendDataRequest sendDatReq = {&basReq, {(uint8_t*) (info->send_msg.str), (info->send_msg.len)}, 1, (EmDeviceDataType)EDDT_manufatureSvr};
			*r_len = epb_send_data_request_pack_size( &sendDatReq ) + fix_head_len;
		#if defined EAM_md5AndAesEnrypt
			uint16_t	length = *r_len;
			uint8_t	*p = ke_malloc( AES_get_length( *r_len - fix_head_len ),KE_MEM_KE_MSG );
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			*r_len = AES_get_length( *r_len - fix_head_len ) + fix_head_len;
		#endif
			*r_data = (uint8_t *)ke_malloc( (*r_len), KE_MEM_KE_MSG );
			if( !( *r_data ) )
			{
				UART_PRINTF("Not enough memory!\r\n");
				return;
			}
			if( epb_pack_send_data_request( &sendDatReq, *r_data + fix_head_len, *r_len - fix_head_len ) < 0 )
			{
				*r_data = NULL;
		#if defined EAM_md5AndAesEnrypt
				if( p )
				{
					ke_free( p );
					p = NULL;
				}
		#endif
				return;
			}
		#if defined EAM_md5AndAesEnrypt
			//encrypt body
			AES_Init( session_key );
			AES_Encrypt_PKCS7( *r_data + fix_head_len, p, length - fix_head_len, session_key ); //原始数据长度
			memcpy( *r_data + fix_head_len, p, *r_len - fix_head_len );
			if( p )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			fix_head.nCmdId = htons( ECI_req_sendData );
			fix_head.nLength = htons( *r_len );
			fix_head.nSeq = htons( wd_process_state.seq );
			memcpy( *r_data, &fix_head, fix_head_len );
			if( bleDemoHead )
			{
				ke_free( bleDemoHead );
				bleDemoHead = NULL;
			}
		#ifdef CATCH_LOG	
			UART_PRINTF("\r\n##send data: ");
			uint8_t *d = *r_data;
			for(uint8_t i=0;i<*r_len;++i)
			{
				UART_PRINTF(" %x",d[i]);
			}
			BpFixHead *fix_head = (BpFixHead *)*r_data;
			UART_PRINTF("\r\n CMDID: %d",ntohs(fix_head->nCmdId));
			UART_PRINTF("\r\n len: %d", ntohs(fix_head->nLength ));
			UART_PRINTF("\r\n Seq: %d", ntohs(fix_head->nSeq));
		#endif
			wd_process_state.send_data_seq++;
			return;
		}
	}
}



int wechat_consume( uint8_t *data, uint32_t len )
{
	//BpFixHead	fix_head_pack = { 0, 0, 0, 0, 0 };
	//BpFixHead	*fix_head = &fix_head_pack;
	BpFixHead *fix_head = (BpFixHead *)data;
	uint8_t		fix_head_len = sizeof( BpFixHead );
	//memcpy( &fix_head_pack, data, sizeof( BpFixHead ) );

#ifdef CATCH_LOG
	UART_PRINTF("\r\n##Received data: ");
	uint8_t *d = data;
	for(uint8_t i=0;i<len;++i){
	UART_PRINTF(" %x",d[i]);}
	UART_PRINTF("\r\n CMDID: %d", ntohs(fix_head->nCmdId));
	UART_PRINTF("\r\n len: %d", ntohs(fix_head->nLength));
	UART_PRINTF("\r\n Seq: %d",ntohs(fix_head->nSeq));
#endif

	switch( ntohs( fix_head->nCmdId ) )
	{
		case ECI_none:
		{
		}
		break;
		case ECI_resp_auth:
		{
			AuthResponse* authResp;
			authResp = epb_unpack_auth_response( data + fix_head_len, len - fix_head_len );
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'authResp'\r\n");
		#endif
			if( !authResp )
			{
				return errorCodeUnpackAuthResp;
			}
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n unpack 'authResp' success!\r\n");
		#endif
			if( authResp->base_response )
			{
				if( authResp->base_response->err_code == 0 )
				{
					wd_process_state.auth_state = true;
				}else
				{
				#ifdef CATCH_LOG
					UART_PRINTF("\r\n error code:%d",authResp->base_response->err_code);
				#endif
				#ifdef CATCH_LOG
				if(authResp->base_response->has_err_msg)
				{
					UART_PRINTF("\r\n base_response error msg:%s",authResp->base_response->err_msg.str);	
				}
				#endif
					epb_unpack_auth_response_free( authResp );
					return authResp->base_response->err_code;
				}
			}
		#if defined EAM_md5AndAesEnrypt  // get sessionkey
			if( authResp->aes_session_key.len )
			{
			#ifdef CATCH_LOG
				UART_PRINTF("\r\nsession_key:");
			#endif
				AES_Init( key );
				AES_Decrypt( session_key, authResp->aes_session_key.data, authResp->aes_session_key.len, key );
			#ifdef CATCH_LOG
				for(uint8_t i = 0;i<16;i++)
				{
					UART_PRINTF(" 0x%02x",session_key[i]);	
				}
			#endif
			}
		#endif
			epb_unpack_auth_response_free( authResp );
			wechat_process();
		}
		break;
		case ECI_resp_sendData:
		{
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'sendDataResp'\r\n");
		#endif
		#if defined EAM_md5AndAesEnrypt
			uint32_t	length = len - fix_head_len; //加密后数据长度
			uint8_t	*p = ke_malloc( length,KE_MEM_KE_MSG );
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				if( data )
				{
					ke_free( data );
				}
				data = NULL;
				return 0;
			}
			AES_Init( session_key );
			//解密数据
			AES_Decrypt( p, data + fix_head_len, len - fix_head_len, session_key );

			uint8_t temp;
			temp = p[length - 1];                               //算出填充长度
			len = len - temp;                                   //取加密前数据总长度
			memcpy( data + fix_head_len, p, length - temp );    //把明文放回
			if( p )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			SendDataResponse *sendDataResp;
			sendDataResp = epb_unpack_send_data_response( data + fix_head_len, len - fix_head_len );
			if( !sendDataResp )
			{
				//return errorCodeUnpackSendDataResp;
			}
		#ifdef CATCH_LOG
			BlueDemoHead *bledemohead = (BlueDemoHead*)sendDataResp->data.data;
			if(ntohs(bledemohead->m_cmdid) == sendTextResp)
			{
				UART_PRINTF("\r\n received msg: %s\r\n",sendDataResp->data.data+sizeof(BlueDemoHead));
			}
		#endif	
			if( sendDataResp->base_response->err_code )
			{
				//epb_unpack_send_data_response_free( sendDataResp );
				//return sendDataResp->base_response->err_code;
			}
			epb_unpack_send_data_response_free( sendDataResp );
		}
		break;
		case ECI_resp_init:
		{
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'initResp'\r\n");
		#endif
		#if defined EAM_md5AndAesEnrypt
			uint32_t	length = len - fix_head_len; //加密后数据长度
			uint8_t	*p = ke_malloc( length ,KE_MEM_KE_MSG);
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				if( data )
				{
					ke_free( data );
				}
				data = NULL;
				return 0;
			}
			AES_Init( session_key );
			//解密数据
			AES_Decrypt( p, data + fix_head_len, len - fix_head_len, session_key );

			uint8_t temp;
			temp = p[length - 1];                               //算出填充长度
			len = len - temp;                                   //取加密前数据总长度
			memcpy( data + fix_head_len, p, length - temp );    //把明文放回
			if( p )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			InitResponse *initResp = epb_unpack_init_response( data + fix_head_len, len - fix_head_len );
			if( !initResp )
			{
				return errorCodeUnpackInitResp;
			}
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n unpack 'initResp' success!");
		#endif	
			if( initResp->base_response )
			{
				if( initResp->base_response->err_code == 0 )
				{
					if( initResp->has_challeange_answer )
					{
						if( crc32( 0, challeange, CHALLENAGE_LENGTH ) == initResp->challeange_answer )
						{
							wd_process_state.init_state = true;
						}
					}else
					{
						wd_process_state.init_state = true;
					}
					wd_process_state.wechats_switch_state = true;
				}else
				{
				#ifdef CATCH_LOG
					UART_PRINTF("\r\n error code:%d",initResp->base_response->err_code);
				#endif	
					if( initResp->base_response->has_err_msg )
					{
					#ifdef CATCH_LOG
						UART_PRINTF("\r\n base_response error msg:%s",initResp->base_response->err_msg.str);
					#endif	
					}
					epb_unpack_init_response_free( initResp );
					return initResp->base_response->err_code;
				}
			}
			epb_unpack_init_response_free( initResp );
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n send resp test data.");
		#endif
			device_send_test(); //debug
		}
		break;
		case ECI_push_recvData:
		{
		#if defined EAM_md5AndAesEnrypt
			uint32_t	length = len - fix_head_len; //加密后数据长度
			uint8_t	*p = ke_malloc( length ,KE_MEM_KE_MSG);
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				if( data )
				{
					ke_free( data );
				}
				data = NULL;
				return 0;
			}
			AES_Init( session_key );
			//解密数据
			AES_Decrypt( p, data + fix_head_len, len - fix_head_len, session_key );

			uint8_t temp;
			temp = p[length - 1];                               //算出填充长度
			len = len - temp;                                   //取加密前数据总长度
			memcpy( data + fix_head_len, p, length - temp );    //把明文放回
			if( p )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			RecvDataPush *recvDatPush;
			recvDatPush = epb_unpack_recv_data_push( data + fix_head_len, len - fix_head_len );
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'recvDataPush'\r\n");
		#endif
			if( !recvDatPush )
			{
				return errorCodeUnpackRecvDataPush;
			}
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n unpack the 'recvDataPush' successfully! \r\n");
			if(recvDatPush->base_push == NULL)
			{
				UART_PRINTF("\r\n recvDatPush->base_push is NULL! \r\n");
			}
			else 
			{
				UART_PRINTF("\r\n recvDatPush->base_push is not NULL! \r\n");
			}
			UART_PRINTF("\r\n recvDatPush->data.len: %x \r\n",recvDatPush->data.len);
			UART_PRINTF("\r\n recvDatPush->data.data:  \r\n");
			const uint8_t *d = recvDatPush->data.data;
			for(uint8_t i=0;i<recvDatPush->data.len;++i)
			{
				UART_PRINTF(" %x",d[i]);
			}
			if(recvDatPush->has_type)
			{
				UART_PRINTF("\r\n recvDatPush has type! \r\n");
				UART_PRINTF("\r\n type: %d\r\n",recvDatPush->type);
			}
		#endif	
			
		#ifdef CATCH_LOG
			BlueDemoHead *bledemohead = (BlueDemoHead*)recvDatPush->data.data;
			UART_PRINTF("\r\n magicCode: %x",bledemohead->m_magicCode[0]);
			UART_PRINTF(" %x",bledemohead->m_magicCode[1]);
			UART_PRINTF("\r\n version: %x",ntohs(bledemohead->m_version));
			UART_PRINTF("\r\n totalLength: %x",ntohs(bledemohead->m_totalLength));
			UART_PRINTF("\r\n cmdid: %x",ntohs(bledemohead->m_cmdid ));
			UART_PRINTF("\r\n errorCode: %x",ntohs(bledemohead->m_errorCode));
		#endif	
		
			(*pWechat_recCb)( CMD_PUSH, (uint8_t*)recvDatPush->data.data, recvDatPush->data.len);
			epb_unpack_recv_data_push_free( recvDatPush );
			wd_process_state.push_data_seq++;
		}
		break;
		case ECI_push_switchView:
		{
			wd_process_state.wechats_switch_state = !wd_process_state.wechats_switch_state;
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'switchViewPush'\r\n");
		#endif
		#if defined EAM_md5AndAesEnrypt
			uint32_t	length = len - fix_head_len; //加密后数据长度
			uint8_t	*p = ke_malloc( length,KE_MEM_KE_MSG );
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				if( data )
				{
					ke_free( data );
				}
				data = NULL;
				return 0;
			}
			AES_Init( session_key );
			//解密数据
			AES_Decrypt( p, data + fix_head_len, len - fix_head_len, session_key );

			uint8_t temp;
			temp = p[length - 1];                               //算出填充长度
			len = len - temp;                                   //取加密前数据总长度
			memcpy( data + fix_head_len, p, length - temp );    //把明文放回
			if( p )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			SwitchViewPush *swichViewPush;
			swichViewPush = epb_unpack_switch_view_push( data + fix_head_len, len - fix_head_len );
			if( !swichViewPush )
			{
				return errorCodeUnpackSwitchViewPush;
			}
			(*pWechat_recCb)( CMD_SWITH_VIEW, NULL, 0);
			epb_unpack_switch_view_push_free( swichViewPush );
		}
		break;
		case ECI_push_switchBackgroud:
		{
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n@@Received 'switchBackgroudPush'\r\n");
		#endif
		#if defined EAM_md5AndAesEnrypt
			uint32_t	length = len - fix_head_len; //加密后数据长度
			uint8_t	*p = ke_malloc( length,KE_MEM_KE_MSG );
			if( !p )
			{
				UART_PRINTF("Not enough memory!\r\n");
				if( data )
				{
					ke_free( data );
				}
				data = NULL;
				return 0;
			}
			AES_Init( session_key );
			//解密数据
			AES_Decrypt( p, data + fix_head_len, len - fix_head_len, session_key );
			uint8_t temp;
			temp = p[length - 1];                               //算出填充长度
			len = len - temp;                                   //取加密前数据总长度
			memcpy( data + fix_head_len, p, length - temp );    //把明文放回
			if( data )
			{
				ke_free( p );
				p = NULL;
			}
		#endif
			SwitchBackgroudPush *switchBackgroundPush = epb_unpack_switch_backgroud_push( data + fix_head_len, len - fix_head_len );
			if( !switchBackgroundPush )
			{
				return errorCodeUnpackSwitchBackgroundPush;
			}
			(*pWechat_recCb)( CMD_SWITCH_BACKGROUP, NULL, 0);
			epb_unpack_switch_backgroud_push_free( switchBackgroundPush );
		}
		break;
		case ECI_err_decode:
			break;
		default:
		{
		#ifdef CATCH_LOG
			UART_PRINTF("\r\n !!ERROR CMDID:%d",ntohs(fix_head->nCmdId));
		#endif
		}
		break;
	}
	return 0;
}



void wechat_clear( void )
{
	wd_process_state.auth_send = 0;
	wd_process_state.auth_state = 0;
	wd_process_state.init_send = 0;
	wd_process_state.init_state = 0;

	wd_process_state.push_data_seq = 0; 
	wd_process_state.send_data_seq = 0; 
	wd_process_state.seq = 0;
	wd_process_state.wechats_switch_state = 0;
}



void wechat_process( void )
{
	int error_code;
#ifdef CATCH_LOG
	UART_PRINTF("wechat_process\r\n");
	UART_PRINTF("wd_process_state.indication_state = %d\r\n", wd_process_state.indication_state);
	UART_PRINTF("wd_process_state.auth_state = %d\r\n", wd_process_state.auth_state);
	UART_PRINTF("wd_process_state.auth_send = %d\r\n", wd_process_state.auth_send);
#endif
	if( ( wd_process_state.indication_state ) && ( !wd_process_state.auth_state ) && ( !wd_process_state.auth_send ) )
	{
		UART_PRINTF("start auth\r\n");
		error_code = device_auth();
		if( !error_code )
		{
			wd_process_state.auth_send = 1;
		}
	}
	if( ( wd_process_state.auth_state ) && ( !wd_process_state.init_state ) && ( !wd_process_state.init_send ) )
	{
		UART_PRINTF("start Init\r\n");
		error_code = device_init( );
		if( !error_code )
		{
			wd_process_state.init_send = 1;
		}
	}
}



uint16_t device_send_test( void )
{
	uint8_t			*data = NULL;
	uint32_t			len = 0;

	mpbledemo2_info cmd_disponse_arg = { CMD_SENDDAT, { SEND_HELLO_WECHAT, sizeof( SEND_HELLO_WECHAT ) } };
	mpbledemo2_info *m_data_produce_args = &cmd_disponse_arg;

	wechat_data_produce( m_data_produce_args, &data, &len );
	if( data == NULL )
	{
		return errorCodeProduce;
	}
	//sent data
	app_wechat_send_data(data, len);
	ke_free( data );
	
	return 0;
}


uint16_t wechat_send_pack( uint8_t* pkg, uint16_t length )
{
	uint8_t			*data = NULL;
	uint32_t			len = 0;

	mpbledemo2_info cmd_disponse_arg = { CMD_SENDDAT, { (const char *)pkg, length } };
	mpbledemo2_info *m_data_produce_args = &cmd_disponse_arg;

	wechat_data_produce( m_data_produce_args, &data, &len );
	if( data == NULL )
	{
		return errorCodeProduce;
	}
	//sent data
	app_wechat_send_data(data, len);
	ke_free( data );
	
	return 0;
}


void wechat_set_indicate_state( uint8_t state )
{
	wd_process_state.indication_state = state;
}

void wechat_Init(void)
{
	wechat_clear();
	wechat_get_md5();
}

/************************************** The End Of File **************************************/
