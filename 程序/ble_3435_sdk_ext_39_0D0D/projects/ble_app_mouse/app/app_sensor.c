#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "app_sensor.h"               	 // health thermometer functions
#include "uart.h"
#include "gpio.h"
#include "app_key.h"
#include "BK3435_reg.h"

SENSOR_VALUE sensor_val;

unsigned char bFirst_Flag = 0;
//#define __SENSOR_TEST__

//#define En_INT	 REG_AHB0_ICU_IRQ_ENABLE |=INT_IRQ_BIT|FIQ_IRQ_BIT;
//#define Dis_INT	 REG_AHB0_ICU_IRQ_ENABLE &=~(INT_IRQ_BIT|FIQ_IRQ_BIT);

//extern 	void driver_sensor_paw3204_send_data(uint8_t addr , uint8_t value);
//extern 	void driver_sensor_paw3204_read_data(uint8_t addr, uint8_t *value, uint8_t len);
extern	void delay_us(unsigned int num);

void driver_sensor_paw3204_send_byte(unsigned char value)
{
    unsigned char i;

    /*OUTPUT  GPIO_PULLdown*/
	gpio_config(Sensor_SDIO, OUTPUT, PULL_NONE);
    for(i=0; i<8; i++)
    {

	gpio_set(Sensor_SCK, 0);
	if(value & 0x80)
       	gpio_set(Sensor_SDIO, 1);
	else
       	gpio_set(Sensor_SDIO, 0);
	gpio_set(Sensor_SCK, 1);
	value <<= 1;
    }
}

unsigned char driver_sensor_paw3204_read_byte(void)
{
    unsigned char i, ret;
//	u_int32 tmp_data;

    ret = 0;

//*INPUT*/
//GPIO_PULLHIGH
	gpio_config(Sensor_SDIO, INPUT, PULL_HIGH);

	for(i=0; i<8; i++)
	{
		gpio_set(Sensor_SCK, 0);
		ret <<= 1;
		gpio_set(Sensor_SCK, 1);

		if(gpio_get_input(Sensor_SDIO))
			ret |= 0x01;
	}

    return ret;
}


/*! \fn void driver_sensor_paw3204_send_data(UINT8 addr , UINT8 value)
    \brief Send a byte data \a value to the address \addr in paw3204.

    \param addr - the address of written
    \param value - the data
    \return void
*/
void driver_sensor_paw3204_send_data(unsigned char addr , unsigned char value)
{
//	Dis_INT;
    driver_sensor_paw3204_send_byte(addr |0x80);
    driver_sensor_paw3204_send_byte(value);
//	En_INT;
}

/*! \fn void driver_sensor_paw3204_read_data(UINT8 addr, UINT8 *value, UINT8 len)
    \brief Read \a len bytes data to the pointer \a value from the address \addr in paw3204.
    \param addr - the address of written
    \param value - the pointer stored the read data
    \param len - the length of data read
    \return void
*/
void driver_sensor_paw3204_read_data(unsigned char addr, unsigned char *value, unsigned char len)
{
    unsigned char i;

    for(i=0; i<len; i++)
    {
    //	Dis_INT;
        driver_sensor_paw3204_send_byte(addr++);
        *value++ = driver_sensor_paw3204_read_byte();
	//	En_INT;
    }
}

void driver_sensor_paw3204_cpi_set_value(unsigned char u_data)
// 0: 600
// 1: 800
// 2 :1000
// 3:1300
// 4:1600

{
    unsigned char buf,tmp_data;
    unsigned char count;
    count = 5;
	switch(u_data)
		{
		case 0:
			tmp_data = 1;
			break;
		case 2:
			tmp_data = 4; //¸ÄÎª1600
			break;
		default:
			tmp_data = 2;
			break;
		}
    while(count--)
    {
        driver_sensor_paw3204_send_data(0x06, tmp_data);
        delay_us(10);
        driver_sensor_paw3204_read_data(0x02, &buf, 1);
        if( (buf&0x07) == tmp_data)
            break;
        delay_us(20);
    }
}


void driver_sensor_paw3204_SPI_init(void)
{

	gpio_config(Sensor_SDIO, INPUT, PULL_HIGH);
	gpio_config(Sensor_SCK, OUTPUT, PULL_NONE);
	gpio_set(Sensor_SCK, 1);
	gpio_config(Sensor_MOTSWK, INPUT, PULL_HIGH);

}

void delay_ms(unsigned int tt)
{
    unsigned int i,j;
    while(tt--)
    {
        for (j=0; j<1000; j++)
        {
            for (i=0; i<12; i++)
            {
                ;
            }
        }
    }
}

void resync(void)
{
	#ifdef __3204__
	gpio_set(Sensor_SCK, 0);
	delay_us(5);
	gpio_set(Sensor_SCK, 1);
	delay_ms(5);//delay 400ms;
	#endif
}


void resync1(void)
{
	#ifdef __3204__
	gpio_set(Sensor_SCK, 0);
	delay_us(5);
	gpio_set(Sensor_SCK, 1);
	delay_ms(140);//delay 400ms;
	#endif
}
#define SENTABLE_LEN 32 //31
const unsigned char register_3204[SENTABLE_LEN][2] =
{
    0x09, 0x5a, //0x5A = Disable, register 0x0A ~ 0x7F can be read/written
    0x0D, 0x0F,
    0x1D, 0xE3,
    0x28, 0xB4,
    0x29, 0x46,
    0x2A, 0x96,
    0x2B, 0x8C,
    0x2C, 0x6E,
    0x2D, 0x64,
    0x38, 0x5F,
    0x39, 0x0F,
    0x3A, 0x32,
    0x3B, 0x47,
    0x42, 0x10,
    0x54, 0x2E,
    0x55, 0xF2,
    0x61, 0xF4,
    0x63, 0x70,
    0x75, 0x52,
    0x76, 0x41,
    0x77, 0xED,
    0x78, 0x23,
    0x79, 0x46,
    0x7A, 0xE5,
    0x7C, 0x48,
    0x7D, 0xD2,
    0x7E, 0x77,
    0x1B, 0x35, //For better tracking on dark surface
    //0x7F, 0x01, //(Switch to Bank1). DO NOT read back!
    //0x0B, 0x00,
    0x7F, 0x00, //(Switch to Bank0)
    0x09, 0x00  // 0x0a-0x7f write protect
};



void driver_sensor_paw3204_Dir_sel(unsigned char u_data)
	//bit5-3 : 0,3,5,6//default 3
{
  //  uint8 buf;
    #ifdef __SENSOR_TEST__
  return;
  #endif
/*	driver_sensor_paw3204_send_data(0x09, 0x5a);// disable Write Protect
	driver_sensor_paw3204_send_data(0x19, ((u_data<<3)|0x04)); //set dir and 12bits
	driver_sensor_paw3204_send_data(0x09, 0);//  enable Write Protect*/
}


unsigned char driver_sensor_paw3204_inital(void)
{
    unsigned char buf[3], i;
    unsigned char count = 0;
  #ifdef __SENSOR_TEST__
  return 1;
  #endif
	// continue 16 bytes,then maybe sync
//	bk3631_printf("cc\r\n");
//	driver_sensor_paw3204_SPI_init();
  UART_PRINTF("sensor init\r\n");

	do
		{
		driver_sensor_paw3204_read_data(0x00, buf, 1);
		count++;
		if ((buf[0] != 0x30)&&(buf[0] != 0x31))
			resync();
		else
		{
			 driver_sensor_paw3204_send_data(0x06, 0x83);                 //just reset,
			 break;
		}
		if(count >= 17)
			return FALSE;
		} while(1);
	UART_PRINTF("1\r\n");
	for(i=0; i<31; i++)  ///12
	    driver_sensor_paw3204_send_data(register_3204[i][0], register_3204[i][1]);
/*//read cpi msg
	flash_set_line_mode(1);
	flash_read_data(buf,0x43000,3);

	if(buf[2]==0xff)
		{

		buf[2] = 1;
		flash_erase_sector(0x43000);
		flash_write_data(buf,0x43000,3);

		}
	flash_set_line_mode(4);
	sensor_val.cpi = buf[2] ;
	driver_sensor_paw3204_cpi_set_value(sensor_val.cpi); //dpi is 1000 default
*/
	//driver_sensor_paw3204_Dir_sel(0);
	return TRUE;

}






void driver_sensor_initial(void)
{
    sensor_val.type = SENSOR_MAX;
	#ifdef __SENSOR_TEST__
	sensor_val.type = SENSOR_PAW3204;
	return;
	#endif
	bFirst_Flag = 0;
	driver_sensor_paw3204_SPI_init();
	resync1();
	 while(!driver_sensor_paw3204_inital())
	 	resync1();
        sensor_val.type = SENSOR_PAW3204;
}

unsigned char dcnt=0;
unsigned char dcnt1=0;
short int dx = 2;
short int dy = 2;

//static uint8 test_pin;
unsigned char driver_sensor_paw3204_check(void)
{
    unsigned char buf[3];
    unsigned char count=0;
    unsigned int deltaX_l, deltaY_l, deltaX_h, deltaY_h;

   // 	return 0;   //can't send data //180224
//#ifdef __SENSOR_TEST__
#if 0

        sensor_val.x = 0;
        sensor_val.y=0;

        switch(dcnt)
        {
            case 0:
        	        sensor_val.x += dx;
        	        sensor_val.y =0;
        	    break;
            case 1:
        	        sensor_val.x = 0;
        	        sensor_val.y += dy;
            break;
            case 2:
        	        sensor_val.x -= dx;
        	        sensor_val.y =0;
        	    break;
            case 3:
        	        sensor_val.x = 0;
        	        sensor_val.y -= dy;
        	    break;
        }
		dcnt1++;
		if(dcnt1>100)
			{
			dcnt1 = 0;
			dcnt++;
			dcnt &=0x03;
			}
        delay_us(600);
        return 1;
        //	#endif
#endif
    deltaX_h = 0;
    deltaY_h = 0;
    sensor_val.x = 0;
    sensor_val.y = 0;

    // count = 0;
    // continue 16 bytes,then maybe sync
    do
    {
    	driver_sensor_paw3204_read_data(0x00, buf, 1);
        count++;
    	if((buf[0] != 0x30)&&(buf[0] != 0x31))
    	{
    		resync();
    	}
    	else
        {
    	 	break;
        }
    	if (count >= 17)
	{
	    driver_sensor_paw3204_inital();
	    return FALSE;
	}
    }while(1);
	if(!gpio_get_input(Sensor_MOTSWK))
    {
    //	  bk3231_printf( "at;");
   		 UART_PRINTF("4\r\n");
    	driver_sensor_paw3204_read_data(0x02, buf , 3);
    	deltaX_l = buf[1];
    	deltaY_l =buf[2];

    	if(buf[0]&0x80)// && buf[1]>=20)
    	{
    		if(deltaX_l & 0x0080)
    			deltaX_h = 0xff00;
    		if(deltaY_l & 0x0080)
    			deltaY_h = 0xff00;
    		if (bFirst_Flag>10)
		{
            		sensor_val.x = deltaX_h | deltaX_l;
            		sensor_val.y = deltaY_h | deltaY_l;
                           #ifdef   __RAPOO__
            		sensor_val.x =  (~sensor_val.x)+1;
                           #else
                           sensor_val.y =  (~sensor_val.y)+1;
                           #endif
		}
    		else
		{
            		bFirst_Flag ++;
            		sensor_val.x = 0;
            		sensor_val.y = 0;
		}

    	//	bk3231_printf( "x:%x,y:%x.\r\n",sensor_val.x,sensor_val.y);
    		return TRUE;
    	}
    	driver_sensor_paw3204_read_data(0x00, buf, 1);
    }

    //	bk3231_printf( "no se;");
    return FALSE;

}

void driver_sensor_paw3204_power_down(void)
{
	#ifdef __SENSOR_TEST__
		return;
		#endif

    driver_sensor_paw3204_send_data(0x06, 0x08); //0x06 Configuration, bit3, PD_enh Power down mode: 0 = Normal operation (Default) 1 = Power down mode
/*    driver_sensor_paw3204_send_data(0x09, 0x5a);
    driver_sensor_paw3204_send_data(0x4b, 0x13);
    driver_sensor_paw3204_send_data(0x09, 0x00);*/
}


void driver_sensor_paw3204_power_up(void)
{
	#ifdef __SENSOR_TEST__
		return;
		#endif

		  driver_sensor_paw3204_send_data(0x06,(0x07 & sensor_val.cpi ));
           driver_sensor_paw3204_cpi_set_value ( sensor_val.cpi );
	//	driver_sensor_paw3204_send_data(0x09, 0x5a);
	//	driver_sensor_paw3204_send_data(0x4b, 0x1B);
	//	driver_sensor_paw3204_send_data(0x09, 0x00);
}

