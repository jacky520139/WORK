/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ******** ********************************************************************************
 */


/*
 * INCLUDES
 ****************************************************************************************
 */

#include "rwip_config.h" // RW SW configuration

#include "arch.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "boot.h"      // boot definition
#include "rwip.h"      // RW SW initialization
#include "syscntl.h"   // System control initialization
#include "emi.h"       // EMI initialization
#include "intc.h"      // Interrupt initialization
#include "timer.h"     // TIMER initialization
#include "icu.h"
#include "flash.h"
#include "uart.h"      	// UART initialization
#include "flash.h"     // Flash initialization
//#include "led.h"       // Led initialization
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
#include "rf.h"        // RF initialization
#endif // BLE_EMB_PRESENT || BT_EMB_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"       // application functions
#endif // BLE_APP_PRESENT

#include "nvds.h"         // NVDS definitions

#include "reg_assert_mgr.h"
#include "BK3435_reg.h"
#include "RomCallFlash.h"
#include "gpio.h"
#include "pwm.h"
#include "audio.h"
#include "app_task.h"
#include "ir.h"
#include "oads.h"
#include "wdt.h"
#include "user_config.h"
#include "ADC.h"
#include "ALL_Includes.h"
/**
 ****************************************************************************************
 * @addtogroup DRIVERS
 * @{
 *
 *
 * ****************************************************************************************
 */




/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

// Creation of uart external interface api
struct rwip_eif_api uart_api;

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

static void Stack_Integrity_Check(void);

extern void code_sanity_check(void);

extern void rw_fcc_enter(void);

extern void rw_pn9_enter(void);

#if (UART_DRIVER)
void uart_rx_handler(uint8_t *buf, uint8_t len);
#endif

#if ((UART_PRINTF_EN) &&(UART_DRIVER))
void assert_err(const char *condition, const char * file, int line)
{
	uart_printf("%s,condition %s,file %s,line = %d\r\n",__func__,condition,file,line);

}

void assert_param(int param0, int param1, const char * file, int line)
{
	uart_printf("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void assert_warn(int param0, int param1, const char * file, int line)
{
	uart_printf("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void dump_data(uint8_t* data, uint16_t length)
{
	uart_printf("%s,data = %d,length = %d,file = %s,line = %d\r\n",__func__,data,length);

}
#else
void assert_err(const char *condition, const char * file, int line)
{

}

void assert_param(int param0, int param1, const char * file, int line)
{

}

void assert_warn(int param0, int param1, const char * file, int line)
{

}

void dump_data(uint8_t* data, uint16_t length)
{

}
#endif //UART_PRINTF_EN

extern uint8_t system_mode;


#if 1
/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

//用来加密的原始数据，需要在烧录代码的时候提供
//0102210630355cff0078b69d5538dd22
uint8_t encrypt_key_array[16] =
{
	0x01, 0x02, 0x21, 0x06,
	0x30, 0x35, 0x5c, 0xff,
	0x00, 0x78, 0xb6, 0x9d,
	0x55, 0x38, 0xdd, 0x22
};
#endif

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

//错误时系统复位
void platform_reset(uint32_t error)
{
	//void (*pReset)(void);

	UART_PRINTF("error = %x\r\n", error);

	// Disable interrupts失能中断
	GLOBAL_INT_STOP();

#if UART_PRINTF_EN
	// Wait UART transfer finished
	uart_finish_transfers();
#endif //UART_PRINTF_EN


	if(error == RESET_AND_LOAD_FW || error == RESET_TO_ROM)
	{
		// Not yet supported
	}
	else
	{
		//Restart FW
		//pReset = (void * )(0x0);
		//pReset();
		wdt_enable(10);
		while(1);
	}
}



void bdaddr_env_init(void)
{
	struct bd_addr co_bdaddr;
	flash_read_data(&co_bdaddr.addr[0],0x400e3,6);
	if(co_bdaddr.addr[0]!=0xff ||co_bdaddr.addr[1]!=0xff||
	        co_bdaddr.addr[2]!=0xff||co_bdaddr.addr[3]!=0xff||
	        co_bdaddr.addr[4]!=0xff||co_bdaddr.addr[5]!=0xff )
	{
		memcpy(&co_default_bdaddr,&co_bdaddr,6);
	}
}

//dut模式下
void rw_dut_enter(void)
{
	/*
	 ***************************************************************************
	 * Main loop
	 ***************************************************************************
	 */
	while(1)
	{
		// schedule all pending events开始调度任务
		rwip_schedule();
	}
}

void rw_app_enter(void)
{
#if SYSTEM_SLEEP
	uint8_t sleep_type = 0;
#endif

	while(1)
	{
		//schedule all pending events调度所有挂起事件
		rwip_schedule();
//ANO_DT_Send_Version(0xf1,1, 2, 3, 4, 5);

//		printf("Run Time Calculation ON\r\n");


//		UART_PRINTF("Run Time Calculation ON\r\n");
//		printf("Run Time Calculation ON\r\n");
		usmart_scan();
//		UART_PRINTF("Run Time Calculation OFF\r\n");

//		printf("Run Time Calculation OFF\r\n");
		// Checks for sleep have to be done with interrupt disabled
		//睡眠检查必须在中断被禁用的情况下进行
		GLOBAL_INT_DISABLE();//禁止全局中断

		oad_updating_user_section_pro();

		if(wdt_disable_flag==1)
		{
			wdt_disable();
		}
#if SYSTEM_SLEEP
		// Check if the processor clock can be gated
		//检查处理器时钟是否可以选通
		sleep_type = rwip_sleep();
		if((sleep_type & RW_MCU_DEEP_SLEEP) == RW_MCU_DEEP_SLEEP)
		{
			// 1:idel  0:reduce voltage减压休眠
			if(icu_get_sleep_mode())
			{
				cpu_idle_sleep();//休闲
			}
			else
			{
				cpu_reduce_voltage_sleep();//休眠
			}
		}
		else if((sleep_type & RW_MCU_IDLE_SLEEP) == RW_MCU_IDLE_SLEEP)
		{
			cpu_idle_sleep();//休眠
		}
#endif
		Stack_Integrity_Check();//堆叠完整检查
		GLOBAL_INT_RESTORE();//打开全局中断
	}
}
//初始化系统模式
void sys_mode_init(void)
{
	system_mode |= RW_NO_MODE;
}


/**
 *******************************************************************************
 * @brief RW main function.
 *
 * This function is called right after the booting process has completed.
 *
 * @return status   exit status
 *******************************************************************************
 */
#include "LED.h"
#include "usart.h"
#include "app_fff0.h"              // Battery Application Module Definitions
extern struct rom_env_tag rom_env;
extern void uart_stack_register(void *cb);

void rwip_eif_api_init(void);
void rw_main(void)
{
//	cpu_reduce_voltage_sleep();//休眠
//	cpu_idle_sleep();//休眠
//	while(1);
	/*
	 ***************************************************************************
	 * Platform initialization平台初始化
	 ***************************************************************************
	 */

	// Initialize random process初始化随机模块
	srand(1);

    // 初始化系统运行模式
	sys_mode_init();

	//获取系统睡眠标志
	system_sleep_init();

	// Initialize the exchange memory interface
	emi_init();

	// 初始化时间模块
	timer_init();

	rwip_eif_api_init();

	// 初始化中断控制器
	intc_init();
	Init_RTC();//初始化RTC
//  user_timer_init();
	// Initialize UART component
	 adc_init(1,1);//初始化ADC,软件模式V
#if (UART_DRIVER)
	uart_init(115200);
	//uart_cb_register(uart_rx_handler);//串口注册接收回调函数
	uart_cb_register(USART1_IRQHandler);//串口注册接收回调函数
//    uart_cb_register();//串口注册接收回调函数
	
	//uart_cb_register(app_fff1_send_lvl);//串口注册接收回调函数
#endif

//	Init_LED();
	gpio_config(BlueLedPort, OUTPUT, PULL_NONE);
	gpio_config(RedLedPort, OUTPUT, PULL_NONE);

	gpio_set(BlueLedPort, 0);
	gpio_set(RedLedPort, 0);

  uart_stack_register(uart_printf);

	flash_advance_init();
	bdaddr_env_init();

#if  0//bk encrypt interface test
	code_sanity_check();
#endif

	// Initialize NVDS module
	struct nvds_env_tag env;
	env.flash_read = &flash_read;
	env.flash_write = &flash_write;
	env.flash_erase = &flash_erase;
	nvds_init(env);

	rom_env_init(&rom_env);


	/*
	  ***************************************************************************
	  * RW SW stack initialization
	  ***************************************************************************
	  */
	// 初始化 RW SW 堆栈
	rwip_init(0);

	icu_init();

	flash_init();
	Init_RTC();//初始化RTC
//	gpio_config(0x10, OUTPUT, PULL_NONE);
	gpio_config(BlueLedPort, OUTPUT, PULL_NONE);
//	gpio_set(0x10, 1);

  REG_AHB0_ICU_INT_ENABLE |= (0x01 << 15); //使能蓝牙中断
	REG_AHB0_ICU_IRQ_ENABLE = 0x03;

	// finally start interrupt handling
	GLOBAL_INT_START();//使能全局中断


	/*
	 ***************************************************************************
	 * Choose whitch enter to use
	 ***************************************************************************
	 */
	if((system_mode & RW_DUT_MODE) == RW_DUT_MODE) //dut mode
	{
		UART_PRINTF("DUT Mode Start\r\n");
		rw_dut_enter();
	}
	else if((system_mode & RW_FCC_MODE) == RW_FCC_MODE) //fcc mode
	{
		UART_PRINTF("FCC Mode Start\r\n");
		rw_fcc_enter();
	}
	else if((system_mode & RW_PN9_MODE) == RW_PN9_MODE) //pn9 mode
	{
		//UART_PRINTF("PN9 Mode Start\r\n");
		rw_pn9_enter();
	}
	else //normal mode
	{
		UART_PRINTF("NORMAL Mode Start\r\n");
		rw_app_enter();
	}
}


#if (UART_DRIVER)
//static void uart_rx_handler(uint8_t *buf, uint8_t len)
//{
//	for(uint8_t i=0; i<len; i++)
//	{
//		UART_PRINTF("0x%x ", buf[i]);
//	}
//	uart_printf("\r\n");
//}
#endif

void rwip_eif_api_init(void)
{
	uart_api.read = &uart_read;
	uart_api.write = &uart_write;
	uart_api.flow_on = &uart_flow_on;
	uart_api.flow_off = &uart_flow_off;
}

const struct rwip_eif_api* rwip_eif_get(uint8_t type)
{
	const struct rwip_eif_api* ret = NULL;
	switch(type)
	{
	case RWIP_EIF_AHI:
	{
		ret = &uart_api;
	}
	break;
#if (BLE_EMB_PRESENT) || (BT_EMB_PRESENT)
	case RWIP_EIF_HCIC:
	{
		ret = &uart_api;
	}
	break;
#elif !(BLE_EMB_PRESENT) || !(BT_EMB_PRESENT)
	case RWIP_EIF_HCIH:
	{
		ret = &uart_api;
	}
	break;
#endif
	default:
	{
		ASSERT_INFO(0, type, 0);
	}
	break;
	}
	return ret;
}

static void Stack_Integrity_Check(void)
{
	if ((REG_PL_RD(STACK_BASE_UNUSED)!= BOOT_PATTERN_UNUSED))
	{
		while(1)
		{
			uart_putchar("Stack_Integrity_Check STACK_BASE_UNUSED fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_SVC)!= BOOT_PATTERN_SVC))
	{
		while(1)
		{
			uart_putchar("Stack_Integrity_Check STACK_BASE_SVC fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_FIQ)!= BOOT_PATTERN_FIQ))
	{
		while(1)
		{
			uart_putchar("Stack_Integrity_Check STACK_BASE_FIQ fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_IRQ)!= BOOT_PATTERN_IRQ))
	{
		while(1)
		{
			uart_putchar("Stack_Integrity_Check STACK_BASE_IRQ fail!\r\n");
		}
	}

}


void rom_env_init(struct rom_env_tag *api)
{
	memset(&rom_env,0,sizeof(struct rom_env_tag));
	rom_env.prf_get_id_from_task = prf_get_id_from_task;
	rom_env.prf_get_task_from_id = prf_get_task_from_id;
	rom_env.prf_init = prf_init;
	rom_env.prf_create = prf_create;
	rom_env.prf_cleanup = prf_cleanup;
	rom_env.prf_add_profile = prf_add_profile;
	rom_env.rwble_hl_reset = rwble_hl_reset;
	rom_env.rwip_reset = rwip_reset;
#if SYSTEM_SLEEP
	rom_env.rwip_prevent_sleep_set = rwip_prevent_sleep_set;
	rom_env.rwip_prevent_sleep_clear = rwip_prevent_sleep_clear;
	rom_env.rwip_sleep_lpcycles_2_us = rwip_sleep_lpcycles_2_us;
	rom_env.rwip_us_2_lpcycles = rwip_us_2_lpcycles;
	rom_env.rwip_wakeup_delay_set = rwip_wakeup_delay_set;
#endif
	rom_env.platform_reset = platform_reset;
	rom_env.assert_err = assert_err;
	rom_env.assert_param = assert_param;
	rom_env.Read_Uart_Buf = Read_Uart_Buf;
	rom_env.uart_clear_rxfifo = uart_clear_rxfifo;

}

/// @} DRIVERS
