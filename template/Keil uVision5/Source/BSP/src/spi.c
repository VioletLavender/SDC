/******************** (C) COPYRIGHT  *********************************
* @filename :spi_sdcard.c
* @describe :sd driver spi
* SPI1_NSS <----->PB12
* SPI1_SCK <----->PB13
* SPI1_MISO<----->PB14
* SPI1_MOSI<----->PB15
******************************************************************************/

#include "spi.h"
#include "sdcard.h"

uint8_t SPI_Txdata;
uint8_t SPI_Rxdata;

#define SPI2_TX_DMAChannel DMA_Channel_0
#define SPI2_RX_DMAChannel DMA_Channel_0

#define SPI2_TX_DMAStream DMA1_Stream4
#define SPI2_RX_DMAStream DMA1_Stream3

void SPI2_SDCard_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    /*!< Configure SPI_FLASH_SPI pins: CS */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    SPI_SDCard_CS_High();

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                               //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                           //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                             //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                            //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                                   //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;                                //CRCֵ����Ķ���ʽ
    SPI_Init(SPI2, &SPI_InitStructure);                                     //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI2_DMA_Init();
    SPI_Cmd(SPI2, ENABLE);

}


void SPI2_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    DMA_DeInit(SPI2_TX_DMAStream);
    while (DMA_GetCmdStatus(SPI2_TX_DMAStream) != DISABLE);

    /* Configure DMA Initialization Structure */
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Channel = DMA_Channel_0 ;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR)) ;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(SPI2_TX_DMAStream, &DMA_InitStructure);

    DMA_DeInit(SPI2_RX_DMAStream);
    while (DMA_GetCmdStatus(SPI2_RX_DMAStream) != DISABLE);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_Init(SPI2_RX_DMAStream, &DMA_InitStructure);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
}

static void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));    //�ж���Ч��
    SPI2->CR1 &= 0XFFC7;                                               //λ3-5���㣬�������ò�����
    SPI2->CR1 |= SPI_BaudRatePrescaler;                                  //����SPI1�ٶ�
    SPI_Cmd(SPI2, ENABLE);                                             //ʹ��SPI1
}

void SPI2_SD_SpeedHigh(void)
{
    SPI2_SetSpeed(SPI_BaudRatePrescaler_4);  // 10.5Mbps
}

void SPI2_SD_SpeedLow(void)
{
    SPI2_SetSpeed(SPI_BaudRatePrescaler_256);   //164.0625Kbps
}

uint8_t SPI_SDCard_ReadByte(void)
{
    return SPI_SDCard_SendByte(0xff);
}

uint8_t SPI_SDCard_SendByte(uint8_t byte)
{
    uint8_t sendbyte = byte, rcv;
    DMA_InitTypeDef DMA_InitStructure;

    /* Disable DMA SPI RX Stream */
    DMA_Cmd(SPI2_RX_DMAStream, DISABLE);
    while (DMA_GetCmdStatus(SPI2_RX_DMAStream) != DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) & (rcv);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Channel = DMA_Channel_0 ;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR)) ;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(SPI2_RX_DMAStream, &DMA_InitStructure);

    DMA_DeInit(SPI2_TX_DMAStream); //tx
    while (DMA_GetCmdStatus(SPI2_TX_DMAStream) != DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) & (sendbyte);
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init(SPI2_TX_DMAStream, &DMA_InitStructure);

    DMA_ClearFlag(SPI2_RX_DMAStream, DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3);
    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_FEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TCIF4);

    DMA_Cmd(SPI2_RX_DMAStream, ENABLE);
    DMA_Cmd(SPI2_TX_DMAStream, ENABLE);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

    while (!DMA_GetFlagStatus(SPI2_TX_DMAStream, DMA_FLAG_TCIF4));
    while (!DMA_GetFlagStatus(SPI2_RX_DMAStream, DMA_FLAG_TCIF3));

    DMA_Cmd(SPI2_RX_DMAStream, DISABLE);
    DMA_Cmd(SPI2_TX_DMAStream, DISABLE);

    DMA_ClearFlag(SPI2_RX_DMAStream, DMA_FLAG_TCIF3);
    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_TCIF4);
    return rcv;
}

void SPI_SD_ReadMultiBytes(uint8_t *buf, uint16_t ndtr)
{
    uint8_t temp = 0xff;
    DMA_InitTypeDef DMA_InitStructure;

    /* Disable DMA SPI RX Stream */
    DMA_Cmd(SPI2_RX_DMAStream, DISABLE);
    while (DMA_GetCmdStatus(SPI2_RX_DMAStream) != DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_BufferSize = ndtr;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Channel = DMA_Channel_0 ;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR)) ;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(SPI2_RX_DMAStream, &DMA_InitStructure);

    DMA_DeInit(SPI2_TX_DMAStream);
    while (DMA_GetCmdStatus(SPI2_TX_DMAStream) != DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&temp;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_BufferSize = ndtr;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init(SPI2_TX_DMAStream, &DMA_InitStructure);

    DMA_ClearFlag(SPI2_RX_DMAStream, DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3);
    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_FEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TCIF4);

    DMA_Cmd(SPI2_RX_DMAStream, ENABLE);
    DMA_Cmd(SPI2_TX_DMAStream, ENABLE);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

    while (!DMA_GetFlagStatus(SPI2_TX_DMAStream, DMA_FLAG_TCIF4));
    while (!DMA_GetFlagStatus(SPI2_RX_DMAStream, DMA_FLAG_TCIF3));

    DMA_Cmd(SPI2_RX_DMAStream, DISABLE);
    DMA_Cmd(SPI2_TX_DMAStream, DISABLE);

    DMA_ClearFlag(SPI2_RX_DMAStream, DMA_FLAG_TCIF3);
    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_TCIF4);
}

void SPI_SD_SendMultiBytes(uint8_t *buf, uint16_t ndtr)
{
    DMA_InitTypeDef  DMA_InitStructure;
    DMA_DeInit(SPI2_TX_DMAStream); //tx
    while (DMA_GetCmdStatus(SPI2_TX_DMAStream) != DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_BufferSize = ndtr;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Channel = DMA_Channel_0 ;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR)) ;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(SPI2_TX_DMAStream, &DMA_InitStructure);

    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_FEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TCIF4);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

    DMA_Cmd(SPI2_TX_DMAStream, ENABLE);

    while (!DMA_GetFlagStatus(SPI2_TX_DMAStream, DMA_FLAG_TCIF4));
    DMA_Cmd(SPI2_TX_DMAStream, DISABLE);
    DMA_ClearFlag(SPI2_TX_DMAStream, DMA_FLAG_TCIF4);
}
