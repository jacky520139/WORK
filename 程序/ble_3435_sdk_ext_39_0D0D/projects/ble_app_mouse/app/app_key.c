/** **************************************************************************************
 *
 * @file app_key.c
 *
 * @brief ffe0 Application Module entry point
 *
 * @auth  alen
 *
 * @date  2017.03.30
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "app_key.h"               	 // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "rf.h"
#include "prf_utils.h"
#include "ke_timer.h"
#include "uart.h"
#include "gpio.h"
#include "app_hid.h"
#include "app_sec.h"
#include "audio.h"
#include "BK3435_reg.h"
#include "lld_evt.h"
#include "App_sensor.h"



uint8_t bt_tx_buff[SEND_BUFFER_CNT][SEND_BUFFER_SIZE];
uint8_t tx_buff_len[SEND_BUFFER_CNT];
uint8_t tx_buff_head = 0;
uint8_t tx_buff_tail = 0;
uint8_t tx_buff_count = 0;
uint8_t old_press_flag,key_press_flag,key_rel_flag;
uint8_t stan_key_len,old_key_len;
uint8_t wheel_value,sys_mouse_key;
uint8_t cow[KEYBOARD_MAX_COL_SIZE];
uint8_t row[KEYBOARD_MAX_ROW_SIZE];
uint8_t key_flag[KEYBOARD_MAX_ROW_SIZE][KEYBOARD_MAX_COL_SIZE];
uint8_t stan_key[8],old_key[8];
uint8_t real_key_value[KEYBOARD_MAX_COL_SIZE];
uint8_t key_err_time,key_status;
uint8_t m_key,s_key,c_key,p_key,key_null;
uint8_t sys_media_key[4],sys_power_key[1],sys_standard_key[8];
uint8_t data_upload_status;
uint8_t f1_f12[14] = {0,1,4,45,46,11,12,13,14,9,10,8,6,24};
uint8_t axis_x,axis_y;
const uint8_t media_key[47][2] =
{
    {0x24,0x02},    //WWW back  0
    {0x25,0x02},    //WWW forward 1
    {0x26,0x02},    //WWW Stop  2
    {0x27,0x02},    //WWW Refresh 3
    {0x21,0x02},    //WWW Search 4
    {0x2A,0x02},    //WWW Favorites  5
    {0x23,0x02},    //WWW Home 6
    {0x8A,0x01},    //Mail  7
    {0xE2,0x00},    //Mute 8
    {0xEA,0x00},    //Volume-  9
    {0xE9,0x00},    //Volume+   10
    {0xCD,0x00},    //Play/Pause  11
    {0xB7,0x00},    //Stop  12
    {0xB6,0x00},    //Prev Track 13
    {0xB5,0x00},    //Next Track  14
    {0x83,0x01},    //Media Select  15
    {0x94,0x01},    //My Computer   16
    {0x92,0x01},    //Calculator   17
    {0x09,0x02},    //More Info  18
    {0xB2,0x00},    //Record     19
    {0xB3,0x00},    //Forward     20
    {0xB4,0x00},    //Rewind      21
    {0x8D,0x00},    //Guide        22
    {0x04,0x00},    //<Reserved>   23
    {0x30,0x00},    //Eject(Mac)   24
    {0x07,0x03},    //H7    25
    {0x0A,0x03},    //H10   26lightness+
    {0x0B,0x03},    //H11   27lightness-
    {0xb1,0x01},    //photo 28
    {0xb8,0x00},    //touchkey 29
    {0x14,0x03},    //H20   30
    {0x01,0x03},    //H1   31
    {0x02,0x03},    //H2   32
    {0x03,0x03},    //H3    33
    {0x04,0x03},    //H4    34
    {0x05,0x03},    //H5    35
    {0x06,0x03},    //H6    36
    {0x08,0x03},    //H8    37
    {0x09,0x03},    //H9    38
    {0x0C,0x03},    //H12   39
    {0x0D,0x03},    //H13   40
    {0x0E,0x03},    //H14   41
    {0x0F,0x03},    //H15   42
    {0x10,0x03},    //H16   43
    {0x11,0x03},    //H17   44
    {0x12,0x03},    //H18   45
    {0x13,0x03},    //H19   46
};

uint8_t power_key[3] =
{
    0x01,   //ACPI:Power
    0x02,   //ACPI:Sleep
    0x04,   //ACPI:Wake up
};


#if (BK3435_DEMO_KIT || BK3435_BELON_BOARD)

//按键配置
uint8_t stand_key_map[KEYBOARD_MAX_ROW_SIZE][KEYBOARD_MAX_COL_SIZE]=
{
    {KEY_POWER,			KEY_UP_ARROW,	KEY_NULL},		//KR0
    {KEY_RIGHT_ARROW,	KEY_ENTER,		KEY_LEFT_ARROW},//KR1
    {KEY_NULL,			KEY_DOWN_ARROW,	KEY_NULL},		//KR2
    {KEY_VOLUME_ADD,	KEY_WWW_BACK, 	KEY_VOLUME_SUB},//KR3
    {KEY_SET,			KEY_VOICE, 		KEY_HOME}, 		//KR4
};
#endif
#if BK3435_HBG_BOARD
uint8_t stand_key_map[KEYBOARD_MAX_ROW_SIZE][KEYBOARD_MAX_COL_SIZE]=
{
	{KEY_POWER_U,		KEY_VOICE,		KEY_SET},		//KR0
    {KEY_UP_ARROW,		KEY_LEFT_ARROW,	KEY_ENTER},		//KR1
    {KEY_RIGHT_ARROW,	KEY_DOWN_ARROW,	KEY_ESC},		//KR2
    {KEY_HOME,			KEY_VOL_ADD_U, 	KEY_VOL_SUB_U}, //KR3
};
#endif

#if BK3435_DEMO_KIT
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOD_0,GPIOD_1,GPIOD_2}; //p3.0,p3.1,p3.2
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOB_1,GPIOB_4,GPIOB_5,GPIOB_6,GPIOB_7}; //P1.1,p1.4,p1.5,p1.6,p1.7
#endif
#if BK3435_BELON_BOARD
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOB_0,GPIOD_2,GPIOD_1}; //p1.0,p3.2,p3.1
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOA_3,GPIOA_4,GPIOA_5,GPIOA_6,GPIOA_7}; //p0.3,p0.4,p0.5,p0.6 ,p0.7
#endif
#if BK3435_HBG_BOARD
uint8_t gpio_buff_c[KEYBOARD_MAX_COL_SIZE]= {GPIOB_0,GPIOD_2,GPIOD_3}; //p1.0,p3.2,p3.1
uint8_t gpio_buff_r[KEYBOARD_MAX_ROW_SIZE]= {GPIOA_3,GPIOA_4,GPIOA_5,GPIOA_6}; //p0.3,p0.4,p0.5,p0.6 ,p0.7
#endif

uint8_t mouse_gpio_buff[MOUSE_KEY_LEN]= {Key_L,Key_M,Key_R,Key_P};
uint8_t mouse_int_buff[MOUSE_INT_LEN]= {Key_L,Key_M,Key_R,Key_P,WheelA,WheelB,Sensor_MOTSWK};

volatile uint32_t sys_flag = 0;
app_key_type app_key_state = ALL_KEY_FREE_DELAY;

static unsigned char app_clear_bond_flag = 0;

//声明应用层GPIO中断回调函数
static void app_gpio_int_cb(void);


/*******************************************************************************
    //for 48M clock
    //100=12.5us
    //16=2us
    //8=1us
    //for 32.768 clock
    //8=1.6ms
    //16M clock
    //100=120us
*******************************************************************************/
void delay_us(uint32_t num)
{

    uint32_t i,j;
    for(i=0; i<num; i++)
        for(j=0; j<3; j++)
        {
            ;
        }
}


/* 按键初始化 */
void key_init(void)
{
	//mouse
	gpio_config(Mouse_LedPort, OUTPUT, PULL_NONE);
	gpio_set(Mouse_LedPort, 0);

	//注册IO回调
	gpio_cb_register(app_gpio_int_cb);

    uint8_t i;
	
	//mouse
	gpio_config(WheelA, INPUT, PULL_HIGH);
	gpio_config(WheelB, INPUT, PULL_HIGH);

	for (i = 0; i < MOUSE_KEY_LEN; i++)
    {
		gpio_config(mouse_gpio_buff[i], INPUT, PULL_HIGH);
    }


	key_press_flag = 0;
	key_rel_flag = 0xff;
#if (SYSTEM_SLEEP)
	//mouse
	for(i=0; i<MOUSE_INT_LEN; i++)
	{
		REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		Delay_ms(2);
		REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
	}

#endif
	//This flag is setted to allow system into sleep mode when power on.
	app_key_state = ALL_KEY_FREE_DELAY;


}
/* 滚轴 */
void mouse_wheel_scan(void)
{
    static uint8_t old_pinLevel = 0;
    static uint8_t wheel_tog = 0 ;
    uint8_t pinLevel = 0;
	//uint8_t wheel_buf[2];
	//WA,WB


	if(!(gpio_get_input(WheelB)))
        	pinLevel |= B_0100_0000;
	if(!(gpio_get_input(WheelA)))
        	pinLevel |= B_1000_0000;

    if(pinLevel == B_0100_0000)
    {
    	if(old_pinLevel == B_1100_0000)
         wheel_tog = B_1000_0001 ;                 // 2
    	else if(old_pinLevel == B_0000_0000)
         wheel_tog = B_1000_0010 ;                 // 4
    }
    else if(pinLevel == B_1000_0000)
    {
        if(old_pinLevel == B_1100_0000)
            wheel_tog = B_1000_0011;                // 3
        else if(old_pinLevel == B_0000_0000)
            wheel_tog = B_1000_0000 ;               // 1
    }
    else
    {
        if(wheel_tog & B_1000_0000)
        {
            wheel_tog &= B_0111_1111;

            if(((wheel_tog == 0x00) && (pinLevel==B_1100_0000))||((wheel_tog == 0x01) && (pinLevel==B_0000_0000)))
                    wheel_value++;
            else if(((wheel_tog == 0x02) && (pinLevel==B_1100_0000))||((wheel_tog == 0x03) && (pinLevel==B_0000_0000)))
                    wheel_value--;
        }
    }
    old_pinLevel = pinLevel;
}


/* 按键扫描*/

void mouse_key_scan(void)
{
	uint8_t  i;

	old_press_flag = key_press_flag;
	// 3435:L,R,M,P,N,DPI,PAIR ,now:L,M,R,Pair
	for(i=0;i<MOUSE_KEY_LEN;i++)
	{
	  if(gpio_get_input(mouse_gpio_buff[i])==0)
	  {
	  	sys_flag |= FLAG_KEY_ACTIVE;
		  if( !(key_press_flag&(1<<i)) && (key_rel_flag&(1<<i)))
		  {
			  UART_PRINTF("enter press=%x\r\n",i);
			  key_press_flag |= 1<<i;
			  key_rel_flag	 &= ~(1<<i);
		  }
	  }
	  else if( !(key_rel_flag&(1<<i))&&(key_press_flag&(1<<i)))
	  {
			UART_PRINTF("enter releas\r\n");
		  	key_rel_flag |= 1<<i;
		  	key_press_flag   &= ~(1<<i);
	  }
	}
	//check key change

	key_status &= ~KEY_STATUS_CHANGE;
	if((old_press_flag&0x1f) != (key_press_flag&0x1f))
	{
		UART_PRINTF("2\n");
		key_status |= KEY_STATUS_CHANGE;
		sys_mouse_key = key_press_flag&0x1f;
	}
	
//******************************************
//CPI key
#if 0
if(((old_press_flag&0x20) != (key_press_flag&0x20)) &&(key_press_flag&0x20))
		{
		// cpi keu trigger
		Cpi_Key_flag |= DPI_KEY_EN;
	//	bprintf("true\n");
		}
	if(Cpi_Key_flag&DPI_KEY_EN)
		{
		sys_flag |= FLAG_KEY_ACTIVE;
		if(((Cpi_Key_Cnt>120)&&(sys_flag&FLAG_BT_IN_SNIFF))||((Cpi_Key_Cnt>240)&&(!(sys_flag&FLAG_BT_IN_SNIFF))))
			{
			GPIO_output(config->bt_led_io>>4,config->bt_led_io&0x0f,0);
			bprintf("over\n");
			}
		else
			{
			Cpi_Key_Cnt++;
	//	bprintf("c=%d\n",Cpi_Key_Cnt);
	}
		}

	if(((old_press_flag&0x20) != (key_press_flag&0x20)) &&(!(key_press_flag&0x20)))
		{
	//	bprintf("1\n");
		//cpi key release
		Cpi_Key_flag &= ~DPI_KEY_EN;
	/*	if(((Cpi_Key_Cnt>120)&&(sys_flag&FLAG_BT_IN_SNIFF))||((Cpi_Key_Cnt>240)&&(!(sys_flag&FLAG_BT_IN_SNIFF))))
			{
			//change mode
			//0xff :BT, 0:2.4
			bprintf("change mode\n");
			flash_set_line_mode(1);
			flash_read_data(buf,0x43000,3);
			buf[1] ^= 0xff;
			flash_erase_sector(0x43000);
			flash_write_data(buf,0x43000,3);
			flash_set_line_mode(4);

			Driver_wdt_enable(1000);
			while(1)
				{
				bprintf("wait..\n");
				}

			}
		else*/
			//chang cpi
			{
			bprintf("change cpi\n");
			Cpi_Key_flag |= DPI_CHANGE_EN|DPI_READ;

							sensor_val.cpi ++;
							if( sensor_val.cpi>=3 )
							 sensor_val.cpi=0;

		//	bprintf("cpi_D_S=%x\n",sensor_val.cpi);
			driver_sensor_paw3204_cpi_set_value(sensor_val.cpi);
			led_on_time_tmp=config->bt_connected_led_on_time;
			led_off_time_tmp=config->bt_connected_led_off_time;
			cpi_en = 1;
			led_time_cnt=0;
			cpi_led_cnt =0;

				config->bt_connected_led_on_time = config->bt_connected_led_off_time = 4;
				cpi_cnt = (sensor_val.cpi+1);
			}
		Cpi_Key_Cnt =0;
		}
	if(Cpi_Key_flag & DPI_CHANGE_EN)
		{
		//	bprintf("change cpi1\n");
			//save dpi msg
			flash_set_line_mode(1);
			if(Cpi_Key_flag & DPI_READ)
				{
				flash_read_data(cpibuf,0x43000,3);
				Cpi_Key_flag &= ~DPI_READ;
				Cpi_Key_flag |= DPI_EARSE;
				}
			else
				{
				if(Cpi_Key_flag & DPI_EARSE)
					{
					flash_erase_sector(0x43000);
					Cpi_Key_flag &= ~DPI_EARSE;
					Cpi_Key_flag |= DPI_WRITE;
					}
				else
					{
					cpibuf[2]=sensor_val.cpi;
					flash_write_data(cpibuf,0x43000,3);
					Cpi_Key_flag &= ~(DPI_WRITE|DPI_CHANGE_EN);
					}
				}
			flash_set_line_mode(4);
			}
#endif
	//paried key
	if(((old_press_flag&0x08) != (key_press_flag&0x08)) &&(key_press_flag&0x08))
	{

		app_clear_bond_flag = 0x01;

	}
}



uint8_t xyz_axis_process(uint8_t joystick_xyz)
{
    if(joystick_xyz>0x78&& joystick_xyz<0x88)
    {
        joystick_xyz=0x80;
    }
    else if(joystick_xyz<0x10)
    {
        joystick_xyz=0;
    }
    else if(joystick_xyz<=0x78)
    {
        joystick_xyz=(0x80*(joystick_xyz-0x10))/(float)0x68;
    }
    else if(joystick_xyz>=0xf0)
    {
        joystick_xyz=0xff;
    }
    else
    {
        joystick_xyz=0xff-(0x80*(0xf0-joystick_xyz))/(float)0x68;
    }
    return joystick_xyz;
}

void key_state_reset(void)
{
	tx_buff_count = 0;
	tx_buff_tail = 0;
	tx_buff_head = 0;
}
extern uint8_t key_free_dalay_cnt;


void key_process(void)
{
	//mouse
	UART_PRINTF("3\r\n");

 	mouse_key_scan();
	mouse_wheel_scan();
	driver_sensor_paw3204_check();
	if(((key_status&KEY_STATUS_CHANGE)||((sensor_val.x !=0)||(sensor_val.y !=0))
		||(wheel_value != 0))&&(tx_buff_count<SEND_BUFFER_CNT))
	{
		sys_flag |= FLAG_KEY_ACTIVE;
		key_free_dalay_cnt=0;
		bt_tx_buff[tx_buff_head][0]=sys_mouse_key;
		bt_tx_buff[tx_buff_head][1]=(uint8_t)((sensor_val.x)&0x00ff);
		bt_tx_buff[tx_buff_head][2]=(uint8_t)(((sensor_val.x >> 8)&0x000f)|(((sensor_val.y)<<4)&0x00f0));
		bt_tx_buff[tx_buff_head][3]=(uint8_t)((sensor_val.y>>4)&0x00ff);
		bt_tx_buff[tx_buff_head][4]=wheel_value;

		tx_buff_len[tx_buff_head]=5;
		tx_buff_head++;
		tx_buff_count++;
		sys_mouse_key = 0;
		sensor_val.x = 0;
		sensor_val.y = 0;
		wheel_value = 0;
		tx_buff_head=tx_buff_head%SEND_BUFFER_CNT;

	}

}

void key_wakeup_set(void)
{
    REG_APB5_GPIO_WUATOD_ENABLE = 0x00000000;
	REG_APB5_GPIO_WUATOD_STAT = 0xffffffff;
    REG_AHB0_ICU_INT_ENABLE &= (~(0x01 << 9));

}


/*配置按键中断及休眠*/
void key_wakeup_config(void)
{
    uint8_t i;

	//Clear period timer
	ke_timer_clear(APP_PERIOD_TIMER, TASK_APP);

    //mouse
    for(i=0; i<MOUSE_INT_LEN; i++)
    {
    	REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		Delay_ms(2);
    	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
		REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(mouse_int_buff[i]>>4)+(mouse_int_buff[i]&0x0f));
    }
}



/*检测按键的状态，返回0表示语音按键按下，返回1表示普通按键或者没有按键按下*/
app_key_type key_status_check(void)
{
    static uint8_t key_down_status=0;
	// 如果都没有动，当前是广播模式，那么启动30s的广播超时timer，超时后睡眠
	if(0==(sys_flag & FLAG_KEY_ACTIVE))
	{
	    if(key_down_status==1)
        {
			if(ke_state_get(TASK_APP) == APPM_ADVERTISING)
			{
				//Start the advertising timer
		        ke_timer_set(APP_ADV_TIMEOUT_TIMER, TASK_APP, APP_DFLT_ADV_DURATION);
			}

		    key_down_status=0;
    		//UART_PRINTF("key FREE\r\n");
        }

		return (ALL_KEY_FREE);
	}
	else
	{
		// 如果有按键、滚轮、sensor，那么就会清除所有超时的timer
		if(ke_timer_active(APP_ADV_TIMEOUT_TIMER, TASK_APP))
		{
			ke_timer_clear(APP_ADV_TIMEOUT_TIMER, TASK_APP);
		}
		if(ke_timer_active(APP_DISCONNECT_TIMER, TASK_APP))
		{
			ke_timer_clear(APP_DISCONNECT_TIMER, TASK_APP);
		}
        key_down_status=1;
  //      UART_PRINTF("key DOWN\r\n");
		return (GENERAL_KEY_DOWN);
	}
}



/*发送鼠标键值 */
void hid_send_mousecode(void)
{
	if(tx_buff_count ) //not power key
	{
		if(app_hid_get_send_flag())
			{
			app_hid_set_send_flag(false);
			app_hid_send_report(&bt_tx_buff[tx_buff_tail][0],tx_buff_len[tx_buff_tail]);
			tx_buff_tail++;
			tx_buff_tail %= SEND_BUFFER_CNT;
			tx_buff_count--;
			UART_PRINTF("s\r\n");
			}
	}
	if(app_clear_bond_flag) //power key to clear bond info
	{
		UART_PRINTF("User clear adv mode\r\n");
		app_clear_bond_flag = 0;
		appm_switch_general_adv();
	}

}

static void app_gpio_int_cb(void)
{
	UART_PRINTF("1\r\n");

	key_wakeup_set();
	sys_flag |= FLAG_KEY_ACTIVE;

	//restart adv
	ke_msg_send_basic(APP_ADV_ENABLE_TIMER,TASK_APP,TASK_APP);

	//enable period timer
	if(!ke_timer_active(APP_PERIOD_TIMER, TASK_APP))
	{
		ke_msg_send_basic(APP_PERIOD_TIMER,TASK_APP,TASK_APP);
	}
}

