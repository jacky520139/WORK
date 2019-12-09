
#include "ALL_Includes.h"
#include "lld_evt.h" 
//#define  USMART_PRINTF uart_printf
#define  USMART_PRINTF ble_printf

               // Battery Application Module Definitions
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com 
//�汾��V3.1
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2011-2021
//All rights reserved
//********************************************************************************
//����˵��
//V1.4
//�����˶Բ���Ϊstring���͵ĺ�����֧��.���÷�Χ������.
//�Ż����ڴ�ռ��,��̬�ڴ�ռ��Ϊ79���ֽ�@10������.��̬��Ӧ���ּ��ַ�������
//V2.0 
//1,�޸���listָ��,��ӡ�������������ʽ.
//2,������idָ��,��ӡÿ����������ڵ�ַ.
//3,�޸��˲���ƥ��,֧�ֺ��������ĵ���(������ڵ�ַ).
//4,�����˺��������Ⱥ궨��.	
//V2.1 20110707		 
//1,����dec,hex����ָ��,�������ò�����ʾ����,��ִ�н���ת��.
//ע:��dec,hex����������ʱ��,���趨��ʾ��������.�����������ʱ��,��ִ�н���ת��.
//��:"dec 0XFF" ��Ὣ0XFFתΪ255,�ɴ��ڷ���.
//��:"hex 100" 	��Ὣ100תΪ0X64,�ɴ��ڷ���
//2,����usmart_get_cmdname����,���ڻ�ȡָ������.
//V2.2 20110726	
//1,������void���Ͳ����Ĳ���ͳ�ƴ���.
//2,�޸�������ʾ��ʽĬ��Ϊ16����.
//V2.3 20110815
//1,ȥ���˺�����������"("������.
//2,�������ַ��������в�����"("��bug.
//3,�޸��˺���Ĭ����ʾ������ʽ���޸ķ�ʽ. 
//V2.4 20110905
//1,�޸���usmart_get_cmdname����,������������������.����������������ʱ����������.
//2,����USMART_ENTIM_SCAN�궨��,���������Ƿ�ʹ��TIM��ʱִ��scan����.
//V2.5 20110930
//1,�޸�usmart_init����Ϊvoid usmart_init(u8 sysclk),���Ը���ϵͳƵ���Զ��趨ɨ��ʱ��.(�̶�100ms)
//2,ȥ����usmart_init�����е�uart_init����,���ڳ�ʼ���������ⲿ��ʼ��,�����û����й���.
//V2.6 20111009
//1,������read_addr��write_addr��������.��������������������д�ڲ������ַ(��������Ч��ַ).���ӷ������.
//2,read_addr��write_addr������������ͨ������USMART_USE_WRFUNSΪ��ʹ�ܺ͹ر�.
//3,�޸���usmart_strcmp,ʹ��淶��.			  
//V2.7 20111024
//1,�����˷���ֵ16������ʾʱ�����е�bug.
//2,�����˺����Ƿ��з���ֵ���ж�,���û�з���ֵ,�򲻻���ʾ.�з���ֵʱ����ʾ�䷵��ֵ.
//V2.8 20111116
//1,������list�Ȳ���������ָ��ͺ���ܵ���������bug.
//V2.9 20120917
//1,�޸������磺void*xxx(void)���ͺ�������ʶ���bug��
//V3.0 20130425
//1,�������ַ���������ת�����֧�֡�
//V3.1 20131120
//1,����runtimeϵͳָ��,��������ͳ�ƺ���ִ��ʱ��.
//�÷�:
//����:runtime 1 ,��������ִ��ʱ��ͳ�ƹ���
//����:runtime 0 ,��رպ���ִ��ʱ��ͳ�ƹ���
///runtimeͳ�ƹ���,��������:USMART_ENTIMX_SCAN Ϊ1,�ſ���ʹ��!!
/////////////////////////////////////////////////////////////////////////////////////
//USMART��Դռ�����@MDK 3.80A@2.0�汾��
//FLASH:4K~K�ֽ�(ͨ��USMART_USE_HELP��USMART_USE_WRFUNS����)
//SRAM:72�ֽ�(���ٵ������)
//SRAM���㹫ʽ:   SRAM=PARM_LEN+72-4  ����PARM_LEN������ڵ���4.
//Ӧ�ñ�֤��ջ��С��100���ֽ�.
////////////////////////////////////////////�û����ò���////////////////////////////////////////////////////	  
//ϵͳ����
u8 *sys_cmd_tab[]=
{
	"?",
	"help",
	"list",
	"id",
	"hex",
	"dec",
	"runtime",
  "Calendar_Get",
	"step_Get",	
};	    
//����ϵͳָ��
//0,�ɹ�����;����,�������;
u8 usmart_sys_cmd_exe(u8 *str)
{
	u8 i;
	u8 sfname[MAX_FNAME_LEN];//��ű��غ�����
	u8 pnum;
	u8 rval;
	u32 res;  
	res=usmart_get_cmdname(str,sfname,&i,MAX_FNAME_LEN);//�õ�ָ�ָ���
	if(res)return USMART_FUNCERR;//�����ָ�� 
	str+=i;	 	 			    
	for(i=0;i<sizeof(sys_cmd_tab)/4;i++)//֧�ֵ�ϵͳָ��
	{
		if(usmart_strcmp(sfname,sys_cmd_tab[i])==0)break;
	}
	switch(i)
	{					   
		case 0:
		case 1://����ָ��
			USMART_PRINTF("\r\n");
#if USMART_USE_HELP
			USMART_PRINTF("------------------------USMART V3.1------------------------ \r\n");
//			USMART_PRINTF("    USMART����ALIENTEK������һ�����ɵĴ��ڵ��Ի������,ͨ�� \r\n");
//			USMART_PRINTF("��,�����ͨ���������ֵ��ó���������κκ���,��ִ��.���,���\r\n");
//			USMART_PRINTF("��������ĺ������������(֧������(10/16����)���ַ�����������\r\n");	  
//			USMART_PRINTF("�ڵ�ַ����Ϊ����),�����������֧��10���������,��֧�ֺ����� \r\n");
//			USMART_PRINTF("��ֵ��ʾ.����������ʾ�������ù���,��������ת������.\r\n");
//			USMART_PRINTF("����֧��:www.openedv.com\r\n");
//			USMART_PRINTF("USMART��7��ϵͳ����:\r\n");
//			USMART_PRINTF("?:      ��ȡ������Ϣ\r\n");
//			USMART_PRINTF("help:   ��ȡ������Ϣ\r\n");
//			USMART_PRINTF("list:   ���õĺ����б�\r\n\n");
//			USMART_PRINTF("id:     ���ú�����ID�б�\r\n\n");
//			USMART_PRINTF("hex:    ����16������ʾ,����ո�+���ּ�ִ�н���ת��\r\n\n");
//			USMART_PRINTF("dec:    ����10������ʾ,����ո�+���ּ�ִ�н���ת��\r\n\n");
//			USMART_PRINTF("runtime:1,�����������м�ʱ;0,�رպ������м�ʱ;\r\n\n");
//			USMART_PRINTF("�밴�ճ����д��ʽ���뺯�������������Իس�������.\r\n");    
//			USMART_PRINTF("--------------------------ALIENTEK------------------------- \r\n");
#else
			USMART_PRINTF("ָ��ʧЧ\r\n");
#endif
			break;
		case 2://��ѯָ��
			USMART_PRINTF("\r\n");
			USMART_PRINTF("-------------------------�����嵥--------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)USMART_PRINTF("%s\r\n",usmart_dev.funs[i].name);
			USMART_PRINTF("\r\n");
			break;	 
		case 3://��ѯID
			USMART_PRINTF("\r\n");
			USMART_PRINTF("-------------------------���� ID --------------------------- \r\n");
			for(i=0;i<usmart_dev.fnum;i++)
			{
				usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&pnum,&rval);//�õ����غ����� 
				USMART_PRINTF("%s id is:\r\n0X%08X\r\n",sfname,usmart_dev.funs[i].func); //��ʾID
			}
			USMART_PRINTF("\r\n");
			break;
		case 4://hexָ��
			USMART_PRINTF("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//��������
			{
				i=usmart_str2num(sfname,&res);	   	//��¼�ò���	
				if(i==0)						  	//����ת������
				{
					USMART_PRINTF("HEX:0X%X\r\n",res);	   	//תΪ16����
				}else if(i!=4)return USMART_PARMERR;//��������.
				else 				   				//������ʾ�趨����
				{
					USMART_PRINTF("16���Ʋ�����ʾ!\r\n");
					usmart_dev.sptype=SP_TYPE_HEX;  
				}

			}else return USMART_PARMERR;			//��������.
			USMART_PRINTF("\r\n"); 
			break;
		case 5://decָ��
			USMART_PRINTF("\r\n");
			usmart_get_aparm(str,sfname,&i);
			if(i==0)//��������
			{
				i=usmart_str2num(sfname,&res);	   	//��¼�ò���	
				if(i==0)						   	//����ת������
				{
					USMART_PRINTF("DEC:%lu\r\n",res);	   	//תΪ10����
				}else if(i!=4)return USMART_PARMERR;//��������.
				else 				   				//������ʾ�趨����
				{
					USMART_PRINTF("10���Ʋ�����ʾ!\r\n");
					usmart_dev.sptype=SP_TYPE_DEC;  
				}

			}else return USMART_PARMERR;			//��������. 
			USMART_PRINTF("\r\n"); 
			break;	 
		case 6://runtimeָ��,�����Ƿ���ʾ����ִ��ʱ��
			USMART_PRINTF("\r\n");
			usmart_get_aparm(str,sfname,&i);
//		USMART_PRINTF("10���Ʋ�����ʾ!\r\n");
//		USMART_PRINTF((const char*)sfname);
//				USMART_PRINTF((char*)str);
//		USMART_PRINTF("10���Ʋ�����ʾ!\r\n");
			if(i==0)//��������
			{
				i=usmart_str2num(sfname,&res);	   		//��¼�ò���	
				if(i==0)						   		//��ȡָ����ַ���ݹ���
				{
					if(USMART_ENTIMX_SCAN==0)USMART_PRINTF("\r\nError! \r\nTo EN RunTime function,Please set USMART_ENTIMX_SCAN = 1 first!\r\n");//����
					else
					{
						usmart_dev.runtimeflag=res;
						if(usmart_dev.runtimeflag)USMART_PRINTF("Run Time Calculation ON\r\n");
						else USMART_PRINTF("Run Time Calculation OFF\r\n"); 
					}
				}else return USMART_PARMERR;   			//δ������,���߲�������	 
 			}else return USMART_PARMERR;				//��������. 
			USMART_PRINTF("\r\n"); 
			break;
	  case 7://��ȡ����
//  Calendar_Update();
//	ke_msg_send_basic(USER_APP_CALENDAR_UPDATE, TASK_APP,TASK_APP);
	USMART_PRINTF("%d-%d-%d %d:%d:%d-%d\r\n",calendar.year,calendar.mon,calendar.date,calendar.RTC.hour,calendar.RTC.minute,calendar.RTC.second,calendar.RTC.week_day);
	USMART_PRINTF("\r\n"); 	
	break;
		case 8://��ȡ����
	USMART_PRINTF("%d-%d-%d %d:%d:%d-%d\r\n",calendar.year,calendar.mon,calendar.date,calendar.RTC.hour,calendar.RTC.minute,calendar.RTC.second,calendar.RTC.week_day);
	USMART_PRINTF("\r\n"); 	
	break;
		default://�Ƿ�ָ��
			return USMART_FUNCERR;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
//��ֲע��:��������stm32Ϊ��,���Ҫ��ֲ������mcu,������Ӧ�޸�.
//usmart_reset_runtime,�����������ʱ��,��ͬ��ʱ���ļ����Ĵ����Լ���־λһ������.��������װ��ֵΪ���,������޶ȵ��ӳ���ʱʱ��.
//usmart_get_runtime,��ȡ��������ʱ��,ͨ����ȡCNTֵ��ȡ,����usmart��ͨ���жϵ��õĺ���,���Զ�ʱ���жϲ�����Ч,��ʱ����޶�
//ֻ��ͳ��2��CNT��ֵ,Ҳ���������+���һ��,���������2��,û������,���������ʱ,������:2*������CNT*0.1ms.��STM32��˵,��:13.1s����
//������:TIM4_IRQHandler��Timer4_Init,��Ҫ����MCU�ص������޸�.ȷ������������Ƶ��Ϊ:10Khz����.����,��ʱ����Ҫ�����Զ���װ�ع���!!

#if USMART_ENTIMX_SCAN==1
//��λruntime
//��Ҫ��������ֲ����MCU�Ķ�ʱ�����������޸�
void usmart_reset_runtime(void)
{
	u32 timer_625us_start,timer_1us_start;
//	TIM4->SR&=~(1<<0);	//����жϱ�־λ 
//	TIM4->ARR=0XFFFF;	//����װ��ֵ���õ����
//	TIM4->CNT=0;		//��ն�ʱ����CNT
	lld_evt_time_get_us(&timer_625us_start,&timer_1us_start);
  	usmart_dev.timer_625us_start=timer_625us_start*625+timer_1us_start;
//	USMART_PRINTF("start=%d;\r\n",delta_time);//���ִ�н��(16���Ʋ�����ʾ)
//  USMART_PRINTF("\r\n");//���ִ�н��(16���Ʋ�����ʾ)	
	
	usmart_dev.runtime=0;	
}
//���runtimeʱ��
//����ֵ:ִ��ʱ��,��λ:0.1ms,�����ʱʱ��Ϊ��ʱ��CNTֵ��2��*0.1ms
//��Ҫ��������ֲ����MCU�Ķ�ʱ�����������޸�
u32 usmart_get_runtime(void)
{
//	u32 delta_time_end,delta_time_start;
	u32 timer_625us_end,timer_1us_end;
//	if(TIM4->SR&0X0001)//�������ڼ�,�����˶�ʱ�����
//	{
//		usmart_dev.runtime+=0XFFFF;
//	}
//	usmart_dev.runtime+=TIM4->CNT;
	lld_evt_time_get_us(&timer_625us_end,&timer_1us_end);
//	usmart_dev.timer_625us_end=timer_625us_end*625+timer_1us_end;
//	USMART_PRINTF("start1=%d;\r\n",usmart_dev.timer_625us_end);//���ִ�н��(16���Ʋ�����ʾ)
	
	lld_evt_time_get_us(&timer_625us_end,&timer_1us_end);
	usmart_dev.timer_625us_end=timer_625us_end*625+timer_1us_end;
//	USMART_PRINTF("start2=%d;\r\n",usmart_dev.timer_625us_end);//���ִ�н��(16���Ʋ�����ʾ)	
//	delta_time_end  =usmart_dev.timer_625us_end*625+usmart_dev.timer_1us_end;
//	delta_time_start=usmart_dev.timer_625us_start*625+usmart_dev.timer_1us_start;
//	USMART_PRINTF("start=%d;\r\n",usmart_dev.timer_625us_start);//���ִ�н��(16���Ʋ�����ʾ)
//	USMART_PRINTF("end=%d;\r\n",usmart_dev.timer_625us_end);//���ִ�н��(16���Ʋ�����ʾ)
	if(usmart_dev.timer_625us_end!=usmart_dev.timer_625us_start)
	{
	usmart_dev.runtime=(usmart_dev.timer_625us_end-usmart_dev.timer_625us_start)/100;
	}
	else
	{usmart_dev.runtime=0;}
//	USMART_PRINTF("end=%d;\r\n",delta_time);//���ִ�н��(16���Ʋ�����ʾ)	
//	 delta_time=((usmart_dev.timer_625us_end*625+usmart_dev.timer_1us_end)-(usmart_dev.timer_625us_start*625+usmart_dev.timer_1us_start))/100;
//	usmart_dev.runtime=delta_time;
//	USMART_PRINTF("runtime=%d;\r\n",delta_time);//���ִ�н��(16���Ʋ�����ʾ)
	return usmart_dev.runtime;		//���ؼ���ֵ
}
//��������������,��USMART����,�ŵ�����,����������ֲ. 
//��ʱ��4�жϷ������	 
void TIM4_IRQHandler(void)
{ 		    		  			    
//	if(TIM4->SR&0X0001)//����ж�
//	{ 
//		usmart_dev.scan();	//ִ��usmartɨ��	
//		TIM4->CNT=0;		//��ն�ʱ����CNT
//		TIM4->ARR=1000;		//�ָ�ԭ��������
//	}				   
//	TIM4->SR&=~(1<<0);//����жϱ�־λ 	    
}
//ʹ�ܶ�ʱ��4,ʹ���ж�.
void Timer4_Init(u16 arr,u16 psc)
{
//	RCC->APB1ENR|=1<<2;	//TIM4ʱ��ʹ��    
// 	TIM4->ARR=arr;  	//�趨�������Զ���װֵ  
//	TIM4->PSC=psc;  	//Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��	
//	TIM4->DIER|=1<<0;   //��������ж�			  							    
//	TIM4->CR1|=0x01;    //ʹ�ܶ�ʱ��2
//  	MY_NVIC_Init(3,3,TIM4_IRQn,2);//��ռ3�������ȼ�3����2(��2�����ȼ���͵�)									 
}
#endif
////////////////////////////////////////////////////////////////////////////////////////
//��ʼ�����ڿ�����
//sysclk:ϵͳʱ�ӣ�Mhz��
void usmart_init(u8 sysclk)
{
#if USMART_ENTIMX_SCAN==1
//	Timer4_Init(1000,(u32)sysclk*100-1);//��Ƶ,ʱ��Ϊ10K ,100ms�ж�һ��,ע��,����Ƶ�ʱ���Ϊ10Khz,�Ժ�runtime��λ(0.1ms)ͬ��.
#endif
	usmart_dev.sptype=1;	//ʮ��������ʾ����
}		
//��str�л�ȡ������,id,��������Ϣ
//*str:�ַ���ָ��.
//����ֵ:0,ʶ��ɹ�;����,�������.
u8 usmart_cmd_rec(u8*str) 
{
	u8 sta,i,rval;//״̬	 
	u8 rpnum,spnum;
	u8 rfname[MAX_FNAME_LEN];//�ݴ�ռ�,���ڴ�Ž��յ��ĺ�����  
	u8 sfname[MAX_FNAME_LEN];//��ű��غ�����
	sta=usmart_get_fname(str,rfname,&rpnum,&rval);//�õ����յ������ݵĺ���������������	  
	if(sta)return sta;//����

	for(i=0;i<usmart_dev.fnum;i++)
	{
		sta=usmart_get_fname((u8*)usmart_dev.funs[i].name,sfname,&spnum,&rval);//�õ����غ���������������
		if(sta)return sta;//���ؽ�������	  
		if(usmart_strcmp(sfname,rfname)==0)//���
		{
			if(spnum>rpnum)return USMART_PARMERR;//��������(���������Դ����������)
			usmart_dev.id=i;//��¼����ID.
			break;//����.
		}	
	}
//	USMART_PRINTF((const char*)rfname);
//	USMART_PRINTF("0x%x ", rpnum);

//	USMART_PRINTF((const char*)sfname);
//	USMART_PRINTF("0x%x ", spnum);
//	
//	USMART_PRINTF("0x%x ", 	usmart_dev.id);
//  USMART_PRINTF("0x%x ", 	usmart_dev.fnum);
	
	if(i==usmart_dev.fnum)return USMART_NOFUNCFIND;	//δ�ҵ�ƥ��ĺ���
 	sta=usmart_get_fparam(str,&i);					//�õ�������������	
	if(sta)return sta;								//���ش���
	usmart_dev.pnum=i;								//����������¼
//USMART_PRINTF("0x%x ", 	usmart_dev.pnum);	
    return USMART_OK;
}
//usamrtִ�к���
//�ú�����������ִ�дӴ����յ�����Ч����.
//���֧��10�������ĺ���,����Ĳ���֧��Ҳ������ʵ��.�����õĺ���.һ��5�����ҵĲ����ĺ����Ѿ����ټ���.
//�ú������ڴ��ڴ�ӡִ�����.��:"������(����1������2...����N)=����ֵ".����ʽ��ӡ.
//����ִ�еĺ���û�з���ֵ��ʱ��,����ӡ�ķ���ֵ��һ�������������.
void usmart_exe(void)
{
	u8 id,i;
	u32 res=0;		   
	u32 temp[MAX_PARM];//����ת��,ʹ֧֮�����ַ��� 
	u8 sfname[MAX_FNAME_LEN];//��ű��غ�����
	u8 pnum,rval;
	id=usmart_dev.id;
	if(id>=usmart_dev.fnum)return;//��ִ��.
	usmart_get_fname((u8*)usmart_dev.funs[id].name,sfname,&pnum,&rval);//�õ����غ�����,���������� 
	USMART_PRINTF("\r\n%s(",sfname);//�����Ҫִ�еĺ�����
	for(i=0;i<pnum;i++)//�������
	{
		if(usmart_dev.parmtype&(1<<i))//�������ַ���
		{
			USMART_PRINTF("%c",'"');			 
			USMART_PRINTF("%s",usmart_dev.parm+usmart_get_parmpos(i));
			USMART_PRINTF("%c",'"');
			temp[i]=(u32)&(usmart_dev.parm[usmart_get_parmpos(i)]);
		}else						  //����������
		{
			temp[i]=*(u32*)(usmart_dev.parm+usmart_get_parmpos(i));
			if(usmart_dev.sptype==SP_TYPE_DEC)USMART_PRINTF("%lu",temp[i]);//10���Ʋ�����ʾ
			else USMART_PRINTF("0X%X",temp[i]);//16���Ʋ�����ʾ 	   
		}
		if(i!=pnum-1)USMART_PRINTF(",");
	}
	USMART_PRINTF(")");
	usmart_reset_runtime();	//��ʱ������,��ʼ��ʱ
	switch(usmart_dev.pnum)
	{
		case 0://�޲���(void����)											  
			res=(*(u32(*)())usmart_dev.funs[id].func)();
			break;
	    case 1://��1������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0]);
			break;
	    case 2://��2������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1]);
			break;
	    case 3://��3������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2]);
			break;
	    case 4://��4������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3]);
			break;
	    case 5://��5������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4]);
			break;
	    case 6://��6������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5]);
			break;
	    case 7://��7������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6]);
			break;
	    case 8://��8������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7]);
			break;
	    case 9://��9������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8]);
			break;
	    case 10://��10������
			res=(*(u32(*)())usmart_dev.funs[id].func)(temp[0],temp[1],temp[2],temp[3],temp[4],\
			temp[5],temp[6],temp[7],temp[8],temp[9]);
			break;
	}
	usmart_get_runtime();//��ȡ����ִ��ʱ��
	if(rval==1)//��Ҫ����ֵ.
	{
		if(usmart_dev.sptype==SP_TYPE_DEC)USMART_PRINTF("=%lu;\r\n",res);//���ִ�н��(10���Ʋ�����ʾ)
		else USMART_PRINTF("=0X%X;\r\n",res);//���ִ�н��(16���Ʋ�����ʾ)	   
	}else USMART_PRINTF(";\r\n");		//����Ҫ����ֵ,ֱ���������
	if(usmart_dev.runtimeflag)	//��Ҫ��ʾ����ִ��ʱ��
	{ 
		USMART_PRINTF("Function Run Time:%d.%1dms\r\n",usmart_dev.runtime/10,usmart_dev.runtime%10);//��ӡ����ִ��ʱ�� 
	}	
}
//usmartɨ�躯��
//ͨ�����øú���,ʵ��usmart�ĸ�������.�ú�����Ҫÿ��һ��ʱ�䱻����һ��
//�Լ�ʱִ�дӴ��ڷ������ĸ�������.
//�������������ж��������,�Ӷ�ʵ���Զ�����.
//�����ALIENTEK�û�,��USART_RX_STA��USART_RX_BUF[]��Ҫ�û��Լ�ʵ��
void usmart_scan(void)
{
	u8 sta,len;  
	if(USART_RX_STA&0x8000)//���ڽ�����ɣ�
	{					   
		len=USART_RX_STA&0x3fff;	//�õ��˴ν��յ������ݳ���
		USART_RX_BUF[len]='\0';	//��ĩβ���������. 
		sta=usmart_dev.cmd_rec(USART_RX_BUF);//�õ�����������Ϣ
		if(sta==0)usmart_dev.exe();	//ִ�к��� 
		else 
		{  
			len=usmart_sys_cmd_exe(USART_RX_BUF);
			if(len!=USMART_FUNCERR)sta=len;
			if(sta)
			{
				switch(sta)
				{
					case USMART_FUNCERR:
						USMART_PRINTF("��������!\r\n");   			
						break;	
					case USMART_PARMERR:
						USMART_PRINTF("��������!\r\n");   			
						break;				
					case USMART_PARMOVER:
						USMART_PRINTF("����̫��!\r\n");   			
						break;		
					case USMART_NOFUNCFIND:
						USMART_PRINTF("δ�ҵ�ƥ��ĺ���!\r\n");   			
						break;		
				}
			}
		}
		USART_RX_STA=0;//״̬�Ĵ������	    
	}
}

#if USMART_USE_WRFUNS==1 	//���ʹ���˶�д����
//��ȡָ����ַ��ֵ		 
u32 read_addr(u32 addr)
{
	return *(u32*)addr;//	
}
//��ָ����ַд��ָ����ֵ		 
void write_addr(u32 addr,u32 val)
{
	*(u32*)addr=val; 	
}
#endif













