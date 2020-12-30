/**
  ******************************************************************************
  * @file    BSP.h
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
#ifndef __SPI_H
#define __SPI_H


#ifdef __cplusplus
extern "C" {
#endif


#undef EXTERN


#ifdef  __SPI_C__
#define EXTERN
#else
#define EXTERN extern
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/ 	
#define SPI_SDCard_CS_Low()   PBout(12) = 0
#define SPI_SDCard_CS_High()    PBout(12) = 1

void SPI2_SDCard_Init(void);
void SPI2_DMA_Init(void);
void SPI2_SD_SpeedHigh(void);
void SPI2_SD_SpeedLow(void);

uint8_t SPI_SDCard_ReadByte(void);
uint8_t SPI_SDCard_SendByte(uint8_t byte);

void SPI_SD_ReadMultiBytes(uint8_t *buf, uint16_t ndtr);
void SPI_SD_SendMultiBytes(uint8_t *buf, uint16_t ndtr);

uint8_t SD_ReadBlockInDMA(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize);
uint8_t SD_WriteBlockInDMA(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize);
uint8_t SD_ReadMultiBlockInDMA(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
uint8_t SD_WriteMultiBlockInDMA(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);


#ifdef __cplusplus
}
#endif


#endif


/******************* (C) COPYRIGHT 2020 *************************END OF FILE***/
