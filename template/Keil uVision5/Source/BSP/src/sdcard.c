/**
  ******************************************************************************
  * @file    sdcard.c
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
#define __SDCARD_C__


/* Includes ------------------------------------------------------------------*/

#include "spi.h"
#include "sdcard.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SD_CardInfo      SDCardInfo;
SD_Type SDType = SD_TYPE_ERR;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


//SPI1_NSS <----->PB12
//SPI1_SCK <----->PB13
//SPI1_MISO<----->PB14
//SPI1_MOSI<----->PB15
    
    
/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_Init(void)
{
    uint8_t buf[10] = {0xff};

    SPI2_SDCard_Init();                       
    SPI2_SD_SpeedLow();

    SPI_SDCard_CS_High();

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles */
    SPI_SD_SendMultiBytes(buf, 10);

    if (SD_GoIdleState() == SD_RESPONSE_NO_ERROR) //CMD0 Send OK! SPI Mode Selected
    {
        
        if (SD_Send_CMD8() == SD_RESPONSE_NO_ERROR)
        {
            
            if (SD_Send_ACMD41() == SD_RESPONSE_NO_ERROR)
            {
                SD_Send_CMD58();
                SPI2_SD_SpeedHigh();
                return SD_RESPONSE_NO_ERROR;
            }

            return SD_RESPONSE_FAILURE;
        }
        else return SD_RESPONSE_FAILURE;
    }
    else
    {
        return SD_RESPONSE_FAILURE;//CMD0 Send failure！
    }
}


/**
  * @brief  SD卡片选失能
  * @param  None
  * @retval None
  */
void SD_DisSelect(void)
{
    SPI_SDCard_CS_High();
    SPI_SDCard_SendByte(DUMMY_BYTE);//提供额外的8个时钟
}


/**
  * @brief  SD卡读状态准备就绪
  * @param  None
  * @retval None
  */
uint8_t SD_WaitReady(void)
{
    uint32_t t = 0;

    do
    {
        if (SPI_SDCard_SendByte(DUMMY_BYTE) == 0XFF) return 0; //OK

        t++;
    }
    while (t < 0XFFFFFF);

    return 1;
}


/**
  * @brief  SD卡片选使能
  * @param  None
  * @retval None
  */
uint8_t SD_Select(void)
{
    SPI_SDCard_CS_Low();

    if (SD_WaitReady() == 0)return 0; //OK

    SD_DisSelect();
    return 1;
}


/**
* @brief  SD卡发送CMD8
  * @param  None
  * @retval None
  */
SD_Error SD_Send_CMD8(void)
{
    uint8_t buf[4];

    SD_Select();
    SD_SendCmd(CMD8, 0x1AA, 0x87);

    if (SD_GetResponse(0x01) == 0) //SD V2.0
    {
        SPI_SD_ReadMultiBytes(buf, 4);
        SD_DisSelect();

        if (buf[2] == 0X01 && buf[3] == 0XAA) //support 2.7V-3.6V
            return SD_RESPONSE_NO_ERROR;
        else
            return SD_RESPONSE_FAILURE;
    }

    SD_DisSelect();
    return SD_RESPONSE_FAILURE;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_Send_ACMD41(void)
{
    uint16_t retry = 0XFFFE;
    uint8_t r1;

    do
    {
        SD_DisSelect();
        SD_Select();
        SD_SendCmd(CMD55, 0, 0X01);
        SD_GetResp();
        SD_DisSelect();

        SD_Select();
        SD_SendCmd(CMD41, 0x40000000, 0X01);
        r1 = SD_GetResp();

    }
    while (r1 != 0 && retry--);

    SD_DisSelect();

    if (retry)
    {
        return SD_RESPONSE_NO_ERROR;
    }

    return SD_RESPONSE_FAILURE;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_Send_ACMD23(uint32_t cnt)
{
    uint16_t retry = 0XFFFE;
    uint8_t r1;

    do
    {
        SD_DisSelect();
        SD_Select();
        SD_SendCmd(CMD55, 0, 0X01);
        SD_GetResp();
        SD_DisSelect();

        SD_Select();
        SD_SendCmd(CMD23, cnt, 0X01);
        r1 = SD_GetResp();

    }
    while (r1 != 0 && retry--);

    SD_DisSelect();

    if (retry)
    {
        return SD_RESPONSE_NO_ERROR;
    }

    return SD_RESPONSE_FAILURE;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_Send_CMD58(void)
{
    uint8_t buf[4];

    SD_Select();
    SD_SendCmd(CMD58, 0, 0X01);

    if (SD_GetResponse(0x00) == 0)
    {
        SPI_SD_ReadMultiBytes(buf, 4);

        if (buf[0] & 0x40)
            SDType = SD_TYPE_V2HC; //high capacity > 2GB
        else
            SDType = SD_TYPE_V2SC;//standard capacity < 2GB

        SD_DisSelect();
        return SD_RESPONSE_NO_ERROR;
    }

    SD_DisSelect();
    return SD_RESPONSE_FAILURE;
}



/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_GetCardInfo(SD_CardInfo *CardInfo)
{
    SD_Error status = SD_RESPONSE_FAILURE;
    SD_Type sd_type = CardInfo->Type;

    status = SD_GetCSDRegister(&(CardInfo->CSD));
    status = SD_GetCIDRegister(&(CardInfo->CID));

    if (sd_type == SD_TYPE_V2HC)
    {
        CardInfo->Capacity = ((uint64_t)CardInfo->CSD.DeviceSize + 1) * 512 * 1024;
        CardInfo->BlockSize = 512;
    }
    else if (sd_type == SD_TYPE_V2SC || sd_type == SD_TYPE_V1)
    {
        CardInfo->Capacity = (CardInfo->CSD.DeviceSize + 1);
        CardInfo->Capacity *= (1 << (CardInfo->CSD.DeviceSizeMul + 2));
        CardInfo->BlockSize = 1 << (CardInfo->CSD.RdBlockLen);
        CardInfo->Capacity *= CardInfo->BlockSize;
    }

    return status;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_ReadBlock(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize)
{
    u8 rvalue = SD_RESPONSE_FAILURE;

    if (SDType != SD_TYPE_V2HC) ReadAddr <<= 9;

    /* MSD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
    SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

    if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        /* Now look for the data token to signify the start of the data */

        if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
        {
            SPI_SD_ReadMultiBytes(pBuffer, BlockSize);
        }
    }

    /* Get CRC bytes (not really needed by us, but required by SD) */
    SPI_SDCard_ReadByte();
    SPI_SDCard_ReadByte();

    /* Set response value to success */
    rvalue = SD_RESPONSE_NO_ERROR;

    /* MSD chip select high */
    SPI_SDCard_CS_High();

    /* Send dummy byte: 8 Clock pulses of delay */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    /* Returns the reponse */
    return (SD_Error)rvalue;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
uint8_t SD_RecvData(uint8_t *buf, uint16_t len)
{
    if (SD_GetResponse(SD_START_DATA_MULTIPLE_BLOCK_READ))
        return 1;

    SPI_SD_ReadMultiBytes(buf, len);
    SPI_SDCard_SendByte(0xFF);
    SPI_SDCard_SendByte(0xFF);
    return 0;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_ReadMultiBlocks(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
    u8 rvalue = SD_RESPONSE_FAILURE;
    uint8_t r1;

    if (SDType != SD_TYPE_V2HC) ReadAddr <<= 9;

    SPI_SDCard_CS_Low();

    SD_SendCmd(SD_CMD_READ_MULT_BLOCK, ReadAddr, 0x01);

    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        return  SD_RESPONSE_FAILURE;
    }

    do
    {
        r1 = SD_RecvData(pBuffer, BlockSize);
        pBuffer += BlockSize;
    }
    while (--NumberOfBlocks && r1 == 0);

    SD_SendCmd(SD_CMD_STOP_TRANSMISSION, ReadAddr, 0x01);

    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        rvalue = SD_RESPONSE_FAILURE;
    }
    else
    {
        rvalue = SD_RESPONSE_NO_ERROR;
    }
    
    SPI_SDCard_CS_High();

    SPI_SDCard_SendByte(DUMMY_BYTE);

    return (SD_Error)rvalue;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_WriteBlock(const uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize)
{
    u8 rvalue = SD_RESPONSE_FAILURE;

    if (SDType != SD_TYPE_V2HC) WriteAddr <<= 9;

    /* MSD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD24 (MSD_WRITE_BLOCK) to write multiple block */
    SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);

    /* Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        /* Send a dummy byte */
        SPI_SDCard_SendByte(DUMMY_BYTE);

        /* Send the data token to signify the start of the data */
        SPI_SDCard_SendByte(SD_START_DATA_SINGLE_BLOCK_WRITE);

        /* Write the block data to MSD : write count data by block */
        SPI_SD_SendMultiBytes((uint8_t *)pBuffer, BlockSize);

        /* Put CRC bytes (not really needed by us, but required by MSD) */
        SPI_SDCard_SendByte(DUMMY_BYTE);
        SPI_SDCard_SendByte(DUMMY_BYTE);

        /* Read data response */
        if (SD_GetDataResponse() == SD_DATA_OK)
        {
            rvalue = SD_RESPONSE_NO_ERROR;
        }
    }

    /* MSD chip select high */
    SPI_SDCard_CS_High();

    /* Send dummy byte: 8 Clock pulses of delay */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    /* Returns the reponse */
    return (SD_Error)rvalue;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
uint8_t SD_SendBlock(const uint8_t *buf, uint8_t cmd, uint16_t BlockSize)
{
    uint16_t t;

    if (SD_WaitReady())return 1; //等待准备失效

    SPI_SDCard_SendByte(cmd);

    if (cmd != SD_STOP_DATA_MULTIPLE_BLOCK_WRITE) //不是结束命令
    {
        SPI_SD_SendMultiBytes((uint8_t *)buf, BlockSize);
        SPI_SDCard_SendByte(0xFF);//忽略CRC
        SPI_SDCard_SendByte(0xFF);

        /* Read data response */
        if (SD_GetDataResponse() == SD_DATA_OK)
        {
            t = SD_RESPONSE_NO_ERROR;
        }
    }

    return t;//写入成功！
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_WriteMultiBlocks(const uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
    uint8_t r1;
    u8 rvalue = SD_RESPONSE_FAILURE;

    if (SDType != SD_TYPE_V2HC) WriteAddr <<= 9;

    /* 设置预擦除 */
    if (SDType != SD_TYPE_MMC)
    {
        SD_Send_ACMD23(NumberOfBlocks);
    }

    /* MSD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD25 (MSD_WRITE_BLOCK) to write multiple block */
    SD_SendCmd(SD_CMD_WRITE_MULT_BLOCK, WriteAddr, 0x01);

    /* Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        return SD_RESPONSE_FAILURE;
    }

    do
    {
        r1 = SD_SendBlock(pBuffer, SD_START_DATA_MULTIPLE_BLOCK_WRITE, BlockSize);
        pBuffer += BlockSize;
    }
    while (--NumberOfBlocks && r1 == 0);

    SPI_SDCard_SendByte(SD_STOP_DATA_MULTIPLE_BLOCK_WRITE);

    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
        return SD_RESPONSE_FAILURE;
    else
        rvalue = SD_RESPONSE_NO_ERROR;

    if (SD_GetDataResponse() == SD_DATA_OK)
    {
        rvalue = SD_RESPONSE_NO_ERROR;
    }

    /* MSD chip select high */
    SPI_SDCard_CS_High();

    /* Send dummy byte: 8 Clock pulses of delay */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    /* Returns the reponse */
    return (SD_Error)rvalue;
}



/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_GetCSDRegister(SD_CSD *SD_csd)
{
    uint32_t i = 0;
    SD_Error rvalue = SD_RESPONSE_FAILURE;
    uint8_t CSD_Tab[16];

    /* SD chip select low */
    SPI_SDCard_CS_Low();
    /* Send CMD9 (CSD register) or CMD10(CSD register) */
    SD_SendCmd(SD_CMD_SEND_CSD, 0, 0xFF);

    /* Wait for response in the R1 format (0x00 is no errors) */
    if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
        {
            for (i = 0; i < 16; i++)
            {
                /* Store CSD register value on CSD_Tab */
                CSD_Tab[i] = SPI_SDCard_ReadByte();
            }
        }

        /* Get CRC bytes (not really needed by us, but required by SD) */
        SPI_SDCard_SendByte(DUMMY_BYTE);
        SPI_SDCard_SendByte(DUMMY_BYTE);
        /* Set response value to success */
        rvalue = SD_RESPONSE_NO_ERROR;
    }

    /* SD chip select high */
    SPI_SDCard_CS_High();
    /* Send dummy byte: 8 Clock pulses of delay */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    /* Byte 0 */
    SD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;

    SD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
    SD_csd->Reserved1 = CSD_Tab[0] & 0x03;

    /* Byte 1 */
    SD_csd->TAAC = CSD_Tab[1];

    /* Byte 2 */
    SD_csd->NSAC = CSD_Tab[2];

    /* Byte 3 */
    SD_csd->MaxBusClkFrec = CSD_Tab[3];

    /* Byte 4 */
    SD_csd->CardComdClasses = CSD_Tab[4] << 4;

    /* Byte 5 */
    SD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
    SD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;

    /* Byte 6 */
    SD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
    SD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
    SD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
    SD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
    SD_csd->Reserved2 = 0; /* Reserved */

    SD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;

    /* Byte 7 */
    SD_csd->DeviceSize |= (CSD_Tab[7]) << 2;

    /* Byte 8 */
    SD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;

    SD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
    SD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

    /* Byte 9 */
    SD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
    SD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
    SD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
    /* Byte 10 */
    SD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;

    SD_csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
    SD_csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

    /* Byte 11 */
    SD_csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
    SD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

    /* Byte 12 */
    SD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
    SD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
    SD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
    SD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

    /* Byte 13 */
    SD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
    SD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
    SD_csd->Reserved3 = 0;
    SD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

    /* Byte 14 */
    SD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
    SD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
    SD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
    SD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
    SD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
    SD_csd->ECC = (CSD_Tab[14] & 0x03);

    /* Byte 15 */
    SD_csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
    SD_csd->Reserved4 = 1;

    if (SD_csd->CSDStruct == 1) //V2.0
    {
        SD_csd->DeviceSize = (((u16)CSD_Tab[8] << 8) & 0xff00) | ((u16)CSD_Tab[9] & 0x00ff);
    }

    /* Return the response */
    return rvalue;
}

/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_GetCIDRegister(SD_CID *SD_cid)
{
    uint32_t i = 0;
    SD_Error rvalue = SD_RESPONSE_FAILURE;
    uint8_t CID_Tab[16];

    /* SD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD10 (CID register) */
    SD_SendCmd(SD_CMD_SEND_CID, 0, 0xFF);

    /* Wait for response in the R1 format (0x00 is no errors) */
    if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
        if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
        {
            /* Store CID register value on CID_Tab */
            for (i = 0; i < 16; i++)
            {
                CID_Tab[i] = SPI_SDCard_ReadByte();
            }
        }

        /* Get CRC bytes (not really needed by us, but required by SD) */
        SPI_SDCard_SendByte(DUMMY_BYTE);
        SPI_SDCard_SendByte(DUMMY_BYTE);
        /* Set response value to success */
        rvalue = SD_RESPONSE_NO_ERROR;
    }

    /* SD chip select high */
    SPI_SDCard_CS_High();
    /* Send dummy byte: 8 Clock pulses of delay */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    /* Byte 0 */
    SD_cid->ManufacturerID = CID_Tab[0];

    /* Byte 1 */
    SD_cid->OEM_AppliID = CID_Tab[1] << 8;

    /* Byte 2 */
    SD_cid->OEM_AppliID |= CID_Tab[2];

    /* Byte 3 */
    SD_cid->ProdName1 = CID_Tab[3] << 24;

    /* Byte 4 */
    SD_cid->ProdName1 |= CID_Tab[4] << 16;

    /* Byte 5 */
    SD_cid->ProdName1 |= CID_Tab[5] << 8;

    /* Byte 6 */
    SD_cid->ProdName1 |= CID_Tab[6];

    /* Byte 7 */
    SD_cid->ProdName2 = CID_Tab[7];

    /* Byte 8 */
    SD_cid->ProdRev = CID_Tab[8];

    /* Byte 9 */
    SD_cid->ProdSN = CID_Tab[9] << 24;

    /* Byte 10 */
    SD_cid->ProdSN |= CID_Tab[10] << 16;

    /* Byte 11 */
    SD_cid->ProdSN |= CID_Tab[11] << 8;

    /* Byte 12 */
    SD_cid->ProdSN |= CID_Tab[12];

    /* Byte 13 */
    SD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
    SD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

    /* Byte 14 */
    SD_cid->ManufactDate |= CID_Tab[14];

    /* Byte 15 */
    SD_cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
    SD_cid->Reserved2 = 1;

    /* Return the response */
    return rvalue;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
    uint8_t Frame[6];

    Frame[0] = (Cmd | 0x40); /* Construct byte 1 */

    Frame[1] = (uint8_t)(Arg >> 24); /* Construct byte 2 */

    Frame[2] = (uint8_t)(Arg >> 16); /* Construct byte 3 */

    Frame[3] = (uint8_t)(Arg >> 8); /* Construct byte 4 */

    Frame[4] = (uint8_t)(Arg); /* Construct byte 5 */

    Frame[5] = (Crc); /* Construct CRC: byte 6 */

    SPI_SD_SendMultiBytes(Frame, 6);
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
uint8_t SD_GetDataResponse(void)
{
    uint32_t i = 0;
    uint8_t response, rvalue;

    while (i <= 64)
    {
        /* Read response */
        response = SPI_SDCard_ReadByte();
        /* Mask unused bits */
        response &= 0x1F;

        switch (response)
        {
            case SD_DATA_OK:
            {
                rvalue = SD_DATA_OK;
                break;
            }

            case SD_DATA_CRC_ERROR:
                return SD_DATA_CRC_ERROR;

            case SD_DATA_WRITE_ERROR:
                return SD_DATA_WRITE_ERROR;

            default:
            {
                rvalue = SD_DATA_OTHER_ERROR;
                break;
            }
        }

        /* Exit loop in case of data ok */
        if (rvalue == SD_DATA_OK)
            break;

        /* Increment loop counter */
        i++;
    }

    /* Wait null data */
    while (SPI_SDCard_ReadByte() == 0);

    /* Return response */
    return response;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_GetResponse(uint8_t Response)
{
    uint32_t Count = 0xFFF;

    /* Check if response is got or a timeout is happen */
    while ((SPI_SDCard_ReadByte() != Response) && Count)
    {
        Count--;
    }

    if (Count == 0)
    {
        /* After time out */
        return SD_RESPONSE_FAILURE;
    }
    else
    {
        /* Right response got */
        return SD_RESPONSE_NO_ERROR;
    }
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
uint8_t SD_GetResp(void)
{
    uint8_t temp = 0xff;
    uint32_t Count = 0xFFF;

    do
    {
        temp = SPI_SDCard_ReadByte();
    }
    while ((temp & 0x80) && Count--);

    return temp;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
uint16_t SD_GetStatus(void)
{
    uint16_t Status = 0;

    /* SD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD13 (SD_SEND_STATUS) to get SD status */
    SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF);

    Status = SPI_SDCard_ReadByte();
    Status |= (uint16_t)(SPI_SDCard_ReadByte() << 8);

    /* SD chip select high */
    SPI_SDCard_CS_High();

    /* Send dummy byte 0xFF */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    return Status;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
SD_Error SD_GoIdleState(void)
{
    u8 retry = 5;
    /* SD chip select low */
    SPI_SDCard_CS_Low();

    /* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
    SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);

    /* Wait for In Idle State Response (R1 Format) equal to 0x01 */
    while (SD_GetResponse(SD_IN_IDLE_STATE) == SD_RESPONSE_FAILURE && retry != 0)
    {

        /* SD chip select high */
        SPI_SDCard_CS_High();

        /* Send dummy byte 0xFF */
        SPI_SDCard_SendByte(DUMMY_BYTE);

        delay_ms(20);

        /* SD chip select low */
        SPI_SDCard_CS_Low();

        /* Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
        SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);

        retry--;
        /* No Idle State Response: return response failure */
        //return SD_RESPONSE_FAILURE;
    }

    /* SD chip select high */
    SPI_SDCard_CS_High();

    /* Send dummy byte 0xFF */
    SPI_SDCard_SendByte(DUMMY_BYTE);

    if (retry == 0) return SD_RESPONSE_FAILURE;

    return SD_RESPONSE_NO_ERROR;
}


/**
  * @brief  SD卡初始化函数，等待初始化完成。
  * @param  None
  * @retval None
  */
void SD_Card_Init(void)
{
    while (SD_Init() != SD_RESPONSE_NO_ERROR)
    {
        delay_ms(100);
    }
}


/******************* (C) COPYRIGHT 2020 *************************END OF FILE***/
