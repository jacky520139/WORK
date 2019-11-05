#ifndef _SENSOR_H__
#define _SENSOR_H__
/*typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef uint8 boolean;*/

//#define __3212__
#define __3204__
#define FALSE	0
#define TRUE	1

// sensor cpi
typedef enum _DIRVER_SENSOR_DIRECTION
{
    DIRECTION_0 = 0x00,
    DIRECTION_90,
    DIRECTION_180,
    DIRECTION_270,
    DIRECTION_MAX
}SENSOR_DIRECTION;

// sensor type
typedef enum _DIRVER_SENSOR_TYPE
{
    SENSOR_A5090 = 0x00,
    SENSOR_A3000,
    SENSOR_PAW3204,
    SENSOR_MAX
}SENSOR_TYPE;


typedef struct _DIRVER_SENSOR_VALUE
{
    unsigned char cpi;
    unsigned char cpi_update;
    unsigned char cpi_gears[2];
    SENSOR_DIRECTION direction;
    SENSOR_TYPE type;
    unsigned int x;
    unsigned int y;
}SENSOR_VALUE;

extern SENSOR_VALUE sensor_val;                 // 9B

extern 	unsigned char bFirst_Flag;

//#define Sensor_Dir	02
//#define Sensor_DirN	037//31
//#define SENSOR_DPI  032//26
extern 	void driver_sensor_initial(void);
extern 	unsigned char driver_sensor_paw3204_check(void);
extern  void driver_sensor_paw3204_power_up(void);
extern 	void driver_sensor_paw3204_power_down(void);
extern  unsigned char driver_sensor_paw3204_inital(void);
extern void driver_sensor_paw3204_cpi_set_value(unsigned char u_data);

#endif
