#include "spi.h"
#include "sdcard.h"
#include "test.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"     /* FatFs lower layer API */
#include <string.h>
#include <stdio.h>
#include "test.h"

BYTE work[FF_MAX_SS];   /* Work area (larger is better for processing time) */
FRESULT fr ;            /* FatFs return code */
FATFS sdfs;             /* Filesystem object structure (FATFS) */
FIL fle;                /* File object structure (FIL) */
char wrstr[2048];


void SD_Card_init(void)
{
    while (SD_Init() != SD_RESPONSE_NO_ERROR)
    {
        delay_ms(100);
    }
    do
    {
        fr = f_mount(&sdfs, "", 1);
        delay_ms(50);
    }
    while (fr != FR_OK); /* Mount the default drive */

}

void SD_Card_Readbin(void)
{
    UINT wr;
    uint16_t length;

    fr = f_open(&fle, "test.bin", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    fr = f_lseek(&fle, fle.obj.objsize);
    sprintf(wrstr, "\r\n");
    fr = f_write(&fle, wrstr, strlen(wrstr), &wr);
    sprintf(wrstr, "STM32 It used to test FatFs Applications\r\n");
    length = strlen(wrstr);
    fr = f_write(&fle, wrstr, length, &wr);
    fr = f_sync(&fle);
    fr = f_close(&fle);
    fr = f_mount(NULL, "0:", 1); /* Unmount the drive */
}

void SD_Card_test(void)
{
    UINT wr;
    uint16_t length;

    fr = f_open(&fle, "message.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    fr = f_lseek(&fle, fle.obj.objsize);
    sprintf(wrstr, "\r\n");
    fr = f_write(&fle, wrstr, strlen(wrstr), &wr);
    sprintf(wrstr, "STM32 It used to test FatFs Applications\r\n");
    length = strlen(wrstr);
    fr = f_write(&fle, wrstr, length, &wr);
    fr = f_sync(&fle);
    fr = f_close(&fle);
    fr = f_mount(NULL, "0:", 1); /* Unmount the drive */
}
