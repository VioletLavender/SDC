/*
********************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  Miljia
  * @version V0.0.0
  * @date    2020.07.25
  * @brief   Main program body
********************************************************************************
*/


/*--------------------------------- Includes ---------------------------------*/

#include "sys.h"
#include "delay.h"
#include "led.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"		/* FatFs lower layer API */
#include <string.h>
#include <stdio.h>


BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
FRESULT fr ;     /* FatFs return code */
FATFS sdfs;   /* Filesystem object structure (FATFS) */
FIL fle;   /* File object structure (FIL) */
char wrstr[2048]; 

int main(void)
{	
	UINT wr;
	uint16_t length;
	LED_Config();
	Buzz_Config();
	setBeep(1);
	delay_init(168);
	while(SD_Init()!=SD_RESPONSE_NO_ERROR)
	{
		delay_ms(100);
	}
	do
	{
		fr = f_mount(&sdfs, "", 1);
		delay_ms(50);
	}
	while( fr!= FR_OK); /* Mount the default drive */
	
	fr = f_open(&fle,"message.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	fr = f_lseek(&fle,fle.obj.objsize);
	sprintf(wrstr,"\r\n");
	fr = f_write(&fle,wrstr,strlen(wrstr),&wr);
	sprintf(wrstr,"NO.01 It used to test FatFs Applications\r\n\
NO.02 It used to test FatFs Applications\r\n\
NO.03 It used to test FatFs Applications\r\n\
NO.04 It used to test FatFs Applications\r\n\
NO.05 It used to test FatFs Applications\r\n\
NO.06 It used to test FatFs Applications\r\n\
NO.07 It used to test FatFs Applications\r\n\
NO.08 It used to test FatFs Applications\r\n\
NO.09 It used to test FatFs Applications\r\n\
NO.10 It used to test FatFs Applications\r\n\
NO.11 It used to test FatFs Applications\r\n\
NO.12 It used to test FatFs Applications\r\n\
NO.13 It used to test FatFs Applications\r\n\
NO.14 It used to test FatFs Applications\r\n\
NO.15 It used to test FatFs Applications\r\n\
NO.16 It used to test FatFs Applications\r\n\
NO.17 It used to test FatFs Applications\r\n\
NO.18 It used to test FatFs Applications\r\n\
NO.19 It used to test FatFs Applications\r\n\
NO.20 It used to test FatFs Applications\r\n\
NO.21 It used to test FatFs Applications\r\n\
NO.22 It used to test FatFs Applications\r\n\
NO.23 It used to test FatFs Applications\r\n\
NO.24 It used to test FatFs Applications\r\n\
NO.25 It used to test FatFs Applications\r\n\
NO.26 It used to test FatFs Applications\r\n\
NO.27 It used to test FatFs Applications\r\n\
NO.28 It used to test FatFs Applications\r\n\
NO.29 It used to test FatFs Applications\r\n\
NO.30 It used to test FatFs Applications\r\n\
NO.31 It used to test FatFs Applications\r\n\
NO.32 It used to test FatFs Applications\r\n\
NO.33 It used to test FatFs Applications\r\n\
NO.34 It used to test FatFs Applications\r\n\
NO.35 It used to test FatFs Applications\r\n\
NO.36 It used to test FatFs Applications\r\n\
NO.37 It used to test FatFs Applications\r\n\
NO.38 It used to test FatFs Applications\r\n\
NO.39 It used to test FatFs Applications\r\n\
NO.40 It used to test FatFs Applications\r\n");
	length = strlen(wrstr);
	fr = f_write(&fle,wrstr,length,&wr);
	fr = f_sync(&fle);
	fr = f_close(&fle);
	fr = f_mount(NULL,"0:",1); /* Unmount the drive */
	
	setBeep(0);
	while(1)
	{
		LEDR_Toggle();
		LEDG_Toggle();
		LEDB_Toggle();
		LEDY_Toggle();
		delay_ms(250);
	}
}



