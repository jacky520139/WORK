#ifndef _LED_H
#define _LED_H
  
#define	Get1		0x11
#define	Set_Get1()		REG_APB5_GPIOB_DATA |= 0x01
#define	RES_Get1()		REG_APB5_GPIOB_DATA &= (~0x01)
#define	Get1Toggle()	REG_APB5_GPIOB_DATA ^= 0x01
#define	Get2		0x12
#define	Set_Get2()		REG_APB5_GPIOB_DATA |= 0x12
#define	RES_Get2()		REG_APB5_GPIOB_DATA &= (~0x02)
#define	Get2Toggle()	REG_APB5_GPIOB_DATA ^= 0x02
#define	Get3		0x13
#define	Set_Get3()		REG_APB5_GPIOB_DATA |= 0x14
#define	RES_Get3()		REG_APB5_GPIOB_DATA &= (~0x04)
#define	Get3Toggle()	REG_APB5_GPIOB_DATA ^= 0x04

#define	COM1		0x33
#define	Set_COM1()		REG_APB5_GPIOD_DATA |= 0x08
#define	RES_COM1()		REG_APB5_GPIOD_DATA &= (~0x08)
#define	COM1Toggle()	REG_APB5_GPIOD_DATA ^= 0x08
#define	COM2		0x07
#define	Set_COM2()		REG_APB5_GPIOA_DATA |= 0x80
#define	RES_COM2()		REG_APB5_GPIOA_DATA &= (~0x80)
#define	COM2Toggle()	REG_APB5_GPIOA_DATA ^= 0x80
#define	COM3		0x06
#define	Set_COM3()		REG_APB5_GPIOA_DATA |= 0x40
#define	RES_COM3()		REG_APB5_GPIOA_DATA &= (~0x40)
#define	COM3Toggle()	REG_APB5_GPIOA_DATA ^= 0x40
#define	COM4		0x05
#define	Set_COM4()		REG_APB5_GPIOA_DATA |= 0x20
#define	RES_COM4()		REG_APB5_GPIOA_DATA &= (~0x20)
#define	COM4Toggle()	REG_APB5_GPIOA_DATA ^= 0x20

void Init_LED(void);
void BlueLed(void);


#endif




