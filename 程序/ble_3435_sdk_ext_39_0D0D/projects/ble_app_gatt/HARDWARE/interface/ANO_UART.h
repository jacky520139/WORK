#ifndef _ANO_UART_H
#define _ANO_UART_H

#include "ALL_Includes.h"
//数据拆分宏定义，在发送大于1字节的数据类型时，比如int16、float等，需要把数据拆分成单独字节进行发送
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

typedef void(*uart_send_cb_t)(unsigned char *,int);
void ANO_DT_Send_Version(u8 type, u16 Data1, u16 Data2, u16 Data3, u16 Data4, u16 Data5);







#endif
