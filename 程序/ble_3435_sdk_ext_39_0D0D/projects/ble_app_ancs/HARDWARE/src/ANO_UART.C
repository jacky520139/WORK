#include "ALL_Includes.h"
uart_send_cb_t uart_send_cb=uart_send;
u8 *data_to_send=uart_tx_buf;

void ANO_DT_Send_Date(u8 type, u16 Data1, u16 Data2, u16 Data3, u16 Data4, u16 Data5)
{
	u8 _cnt=0;
	u8 sum = 0;
	u8 i;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=type;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=BYTE1(Data1);
	data_to_send[_cnt++]=BYTE0(Data1);
	
	data_to_send[_cnt++]=BYTE1(Data2);
	data_to_send[_cnt++]=BYTE0(Data2);
	
	data_to_send[_cnt++]=BYTE1(Data3);
	data_to_send[_cnt++]=BYTE0(Data3);
	
	data_to_send[_cnt++]=BYTE1(Data4);
	data_to_send[_cnt++]=BYTE0(Data4);
	
	data_to_send[_cnt++]=BYTE1(Data5);
	data_to_send[_cnt++]=BYTE0(Data5);
	data_to_send[3] = _cnt-4;
	

	for( i=0;i<_cnt;i++)
		sum += data_to_send[i];
	data_to_send[_cnt++]=sum;
	uart_send_cb(data_to_send, _cnt);

}
