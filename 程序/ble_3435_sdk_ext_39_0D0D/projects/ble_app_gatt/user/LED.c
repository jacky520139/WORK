#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app.h"                     // Application Definitions
#include "app_task.h"                // application task definitions

#include "co_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "rf.h"
#include "prf_utils.h"
#include "ke_timer.h"
#include "uart.h"
#include "gpio.h"


#include "audio.h"
#include "icu.h"
#include "BK3435_reg.h"
#include "lld_evt.h"
#include "LED.h"

void Init_LED(void)
{
	//≥ı ºªØLED
	gpio_config(BlueLedPort|RedLedPort|0X10, OUTPUT, PULL_NONE);
	gpio_config(BlueLedPort|RedLedPort|0X10, OUTPUT, PULL_NONE);
//	gpio_config(RedLedPort, OUTPUT, PULL_NONE);
	gpio_config(0x05, INPUT, PULL_HIGH);
	gpio_set(BlueLedPort, 1);
	gpio_set(RedLedPort, 1);
	
gpio_set(0X10, 0);
while(1)
{

if(gpio_get_input(0x05))
{	gpio_set(BlueLedPort, 1);
	gpio_set(RedLedPort, 1);
	gpio_set(0X10, 0);}
	else
	{
gpio_set(BlueLedPort, 0);
	gpio_set(RedLedPort, 0);
	gpio_set(0X10, 1);}	
	
	
	}





}






