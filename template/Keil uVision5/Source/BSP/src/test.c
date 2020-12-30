/**
  ******************************************************************************
  * @file    BSP.c
  * @author
  * @version V1.00
  * @date    22-Sep-2020
  * @brief   ......
  ******************************************************************************
  * @attention
  *
  * ......
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#define __TEST_C__

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "sdcard.h"
#include "test.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"     /* FatFs lower layer API */
#include <string.h>
#include <stdio.h>
#include "test.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
BYTE work[FF_MAX_SS];   /* Work area (larger is better for processing time) */
FRESULT fr ;            /* FatFs return code */
FATFS sdfs;             /* Filesystem object structure (FATFS) */
FIL fle;                /* File object structure (FIL) */
char wrstr[2048];


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


/**
  * @brief  SD卡挂载函数
  * @param  None
  * @retval None
  */
void SD_Test_Entry(void)
{
    SD_Card_Init();
    SD_F_mount();
    SD_Card_Readbin();
}


/**
  * @brief  SD卡挂载函数
  * @param  None
  * @retval None
  */
void  SD_F_mount(void)
{
    do
    {
        fr = f_mount(&sdfs, "", 1);
        delay_ms(50);
    }

    while (fr != FR_OK); /* Mount the default drive */
}

/**
  * @brief  MCU读取SD卡的bin文件测试函数
  * @param  None
  * @retval None
  */
void SD_Card_Readbin(void)
{
    UINT wr;
    uint16_t length;

    fr = f_open(&fle, "test.bin", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

    fr = f_lseek(&fle, fle.obj.objsize);

    sprintf(wrstr, "\r\n");

    fr = f_write(&fle, wrstr, strlen(wrstr), &wr);

    sprintf(wrstr, "STM32-cp It used to test FatFs Applications\r\n");

    length = strlen(wrstr);

    fr = f_write(&fle, wrstr, length, &wr);

    fr = f_sync(&fle);

    fr = f_close(&fle);

    fr = f_mount(NULL, "0:", 1); /* Unmount the drive */
}


/**
  * @brief  mcu写数据进入SD卡测试函数
  * @param  None
  * @retval None
  */
void SD_Card_Test(void)
{
    UINT wr;
    uint16_t length;

    fr = f_open(&fle, "message.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

    fr = f_lseek(&fle, fle.obj.objsize);

    sprintf(wrstr, "\r\n");

    fr = f_write(&fle, wrstr, strlen(wrstr), &wr);

    sprintf(wrstr, "STM32 cp1 used to test FatFs Applications\r\n");

    length = strlen(wrstr);

    fr = f_write(&fle, wrstr, length, &wr);

    fr = f_sync(&fle);

    fr = f_close(&fle);

    fr = f_mount(NULL, "0:", 1); /* Unmount the drive */
}
