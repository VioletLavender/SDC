/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "spi.h"		/* Example: Header file of existing MMC/SDC contorl module */


/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	SD_Error result;

	switch (pdrv) {
	case 0 :
		result=SD_Init();
		if(SD_RESPONSE_NO_ERROR==result){
			SD_GetCardInfo(&SDCardInfo);
			return RES_OK;
		}    
		else
		{
			SPI2_SD_SpeedLow();
			SPI_SDCard_SendByte(0xff);//提供额外的8个时钟
			SPI2_SD_SpeedHigh();
			return STA_NOINIT;
		}

	case 1 :
		return STA_NOINIT;

	case 2 :
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
	case 0 :
		// translate the arguments here
	
		if ( count == 1 ){	
			SD_ReadBlock(buff,sector,512);		
		}
		else{        			
			SD_ReadMultiBlocks(buff,sector,512,count);		
		}

		// translate the reslut code here
		
		return RES_OK;

	case 1 :
		// translate the arguments here

		// translate the reslut code here

		return RES_OK;

	case 2 :
		// translate the arguments here

		// translate the reslut code here

		return RES_OK;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
	case 0 :
		// translate the arguments here
	
		if ( count == 1 ){	
			SD_WriteBlock(buff,sector,512);	
		}
		else{        			
			SD_WriteMultiBlocks(buff,sector,512,count);		
		}

		// translate the reslut code here

		return RES_OK;

	case 1 :
		// translate the arguments here

		// translate the reslut code here

		return RES_OK;

	case 2 :
		// translate the arguments here

		// translate the reslut code here

		return RES_OK;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	
	switch(cmd)
	{
		case CTRL_SYNC:
				res = RES_OK; 
				break;	 
		case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512; 
				res = RES_OK;
				break;	 
		case GET_BLOCK_SIZE:
				*(WORD*)buff = SDCardInfo.BlockSize;
				res = RES_OK;
				break;	 
		case GET_SECTOR_COUNT:
				*(DWORD*)buff = SDCardInfo.Capacity >> 9;
				res = RES_OK;
				break;
		default:
				res = RES_PARERR;
				break;
	}
	return res;
}


DWORD get_fattime (void)
{
	return 0;
}	


