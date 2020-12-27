#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06
#define CMD0    0       //����λ
#define CMD1    1
#define CMD8    8       //����8 ��SEND_IF_COND
#define CMD9    9       //����9 ����CSD����
#define CMD10   10      //����10����CID����
#define CMD12   12      //����12��ֹͣ���ݴ���
#define CMD16   16      //����16������SectorSize Ӧ����0x00
#define CMD17   17      //����17����sector
#define CMD18   18      //����18����Multi sector
#define CMD23   23      //����23�����ö�sectorд��ǰԤ�Ȳ���N��block
#define CMD24   24      //����24��дsector
#define CMD25   25      //����25��дMulti sector
#define CMD41   41      //����41��Ӧ����0x00
#define CMD55   55      //����55��Ӧ����0x01
#define CMD58   58      //����58����OCR��Ϣ
#define CMD59   59      //����59��ʹ��/��ֹCRC��Ӧ����0x00
//����д���Ӧ������
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD����Ӧ�����
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF
//ALIENTEK ̽����STM32F407������ ʵ��1
//�����ʵ�� -�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com
//������������ӿƼ����޹�˾
//���ߣ�����ԭ�� @ALIENTEK
void SPI1_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    //Configure SPI1 Pins: SCK, MISO and MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //Configure NSS Pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;       //NSS = PA4
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);//�͵�ƽѡͨSD�����ߵ�ƽ����SD��

    SPI_I2S_DeInit(SPI1);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //������
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //8λ���ݳ���
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;         //����Ҫע�⣬һ��Ҫ����Ϊ������������Ч����ΪSD��Ϊ������������Ч
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;        //����Ҫע�⣬һ��Ҫ����ΪSPI_CPHA_2Edge�����ݲ����ڵ�2��ʱ���أ����μ�SD��Э��Ҫ��
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;           //NSS�ź����ⲿ�ܽŹ���
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//SPI�ٶ�Ϊ����
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  //���ݴ���ĵ�һ���ֽ�ΪMSB
    SPI_InitStructure.SPI_CRCPolynomial = 7;            //CRC�Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
}





uint8_t SPIx_ReadWriteByte(uint8_t _ucByte)
{
    uint8_t ch;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);     // �ȴ����ͻ�������
    SPI_I2S_SendData(SPI1, _ucByte);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    // �ȴ����ݽ������
    ch = SPI_I2S_ReceiveData(SPI1);
    return (ch);
}

u8 SD_SPI_ReadWriteByte(u8 data)
{
    return SPIx_ReadWriteByte(data);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

u8  SD_Type = 0; //SD��������

void SPIx_SetSpeed(u8 SpeedSet)
{
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

void SD_SPI_SpeedLow(void)
{
    SPIx_SetSpeed(SPI_BaudRatePrescaler_256);//���õ�����ģʽ
}
//SD������������ʱ��,���Ը�����
void SD_SPI_SpeedHigh(void)
{
    SPIx_SetSpeed(SPI_BaudRatePrescaler_2);//���õ�����ģʽ
}
//SD����ʼ����ʱ��,��Ҫ����


//ȡ��ѡ��,�ͷ�SPI����
void SD_DisSelect(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}

u8 SD_WaitReady(void)
{
    u32 t = 0;
    do
    {
        if (SD_SPI_ReadWriteByte(0XFF) == 0XFF)return 0; //OK
        t++;
    }
    while (t < 0XFFFFFF); //�ȴ�
    return 1;
}

//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
u8 SD_Select(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    if (SD_WaitReady() == 0)return 0; //�ȴ��ɹ�
    SD_DisSelect();
    return 1;//�ȴ�ʧ��
}



//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//    ����,�õ���Ӧֵʧ��
u8 SD_GetResponse(u8 Response)
{
    u16 Count = 0xFFF; //�ȴ�����
    while ((SD_SPI_ReadWriteByte(0XFF) != Response) && Count)Count--; //�ȴ��õ�׼ȷ�Ļ�Ӧ
    if (Count == 0)return MSD_RESPONSE_FAILURE; //�õ���Ӧʧ��
    else return MSD_RESPONSE_NO_ERROR;//��ȷ��Ӧ
}



//��sd����ȡһ�����ݰ�������
//buf:���ݻ�����
//len:Ҫ��ȡ�����ݳ���.
//����ֵ:0,�ɹ�;����,ʧ��;
u8 SD_RecvData(u8 *buf, u16 len)
{
    if (SD_GetResponse(0xFE))return 1; //�ȴ�SD������������ʼ����0xFE
    while (len--) //��ʼ��������
    {
        *buf = SPIx_ReadWriteByte(0xFF);
        buf++;
    }
    //������2��αCRC��dummy CRC��
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);
    return 0;//��ȡ�ɹ�
}



//��sd��д��һ�����ݰ������� 512�ֽ�
//buf:���ݻ�����
//cmd:ָ��
//����ֵ:0,�ɹ�;����,ʧ��;
u8 SD_SendBlock(u8 *buf, u8 cmd)
{
    u16 t;
    if (SD_WaitReady())return 1; //�ȴ�׼��ʧЧ
    SD_SPI_ReadWriteByte(cmd);
    if (cmd != 0XFD) //���ǽ���ָ��
    {
        for (t = 0; t < 512; t++)SPIx_ReadWriteByte(buf[t]); //����ٶ�,���ٺ�������ʱ��
        SD_SPI_ReadWriteByte(0xFF);//����crc
        SD_SPI_ReadWriteByte(0xFF);
        t = SD_SPI_ReadWriteByte(0xFF); //������Ӧ
        if ((t & 0x1F) != 0x05)return 2; //��Ӧ����
    }
    return 0;//д��ɹ�
}



//��SD������һ������
//����: u8 cmd   ����
//      u32 arg  �������
//      u8 crc   crcУ��ֵ
//����ֵ:SD�����ص���Ӧ
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;
    u8 Retry = 0;
    SD_DisSelect();//ȡ���ϴ�Ƭѡ
    if (SD_Select())return 0XFF; //ƬѡʧЧ
    //����
    SD_SPI_ReadWriteByte(cmd | 0x40);//�ֱ�д������
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);
    SD_SPI_ReadWriteByte(crc);
    if (cmd == CMD12)SD_SPI_ReadWriteByte(0xff); //Skip a stuff byte when stop reading
    //�ȴ���Ӧ����ʱ�˳�
    Retry = 0X1F;
    do
    {
        r1 = SD_SPI_ReadWriteByte(0xFF);
    }
    while ((r1 & 0X80) && Retry--);
    //����״ֵ̬
    return r1;
}



//��ȡSD����CID��Ϣ��������������Ϣ
//����: u8 *cid_data(���CID���ڴ棬����16Byte��
//����ֵ:0��NO_ERR
//       1������
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;
    //��CMD10�����CID
    r1 = SD_SendCmd(CMD10, 0, 0x01);
    if (r1 == 0x00)
    {
        r1 = SD_RecvData(cid_data, 16); //����16���ֽڵ�����
    }
    SD_DisSelect();//ȡ��Ƭѡ
    if (r1)return 1;
    else return 0;
}

//��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
//����:u8 *cid_data(���CID���ڴ棬����16Byte��
//����ֵ:0��NO_ERR
//       1������
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;
    r1 = SD_SendCmd(CMD9, 0, 0x01); //��CMD9�����CSD
    if (r1 == 0)
    {
        r1 = SD_RecvData(csd_data, 16); //����16���ֽڵ�����
    }
    SD_DisSelect();//ȡ��Ƭѡ
    if (r1)return 1;
    else return 0;
}


//��ȡSD����������������������
//����ֵ:0�� ȡ��������
//       ����:SD��������(������/512�ֽ�)
//ÿ�������ֽ�����Ϊ512����Ϊ�������512�����ʼ������ͨ��.
u32 SD_GetSectorCount(void)
{
    u8 csd[16];
    u32 Capacity;
    u8 n;
    u16 csize;
    //ȡCSD��Ϣ������ڼ��������0
    if (SD_GetCSD(csd) != 0) return 0;
    //���ΪSDHC�����������淽ʽ����
    if ((csd[0] & 0xC0) == 0x40) //V2.00�Ŀ�
    {
        csize = csd[9] + ((u16)csd[8] << 8) + 1;
        Capacity = (u32)csize << 10;//�õ�������
    }
    else //V1.XX�Ŀ�
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
        Capacity = (u32)csize << (n - 9); //�õ�������
    }
    return Capacity;
}

u8 SD_Idle_Sta(void)
{
    u16 i;
    u8 retry;
    for (i = 0; i < 0xf00; i++); //����ʱ���ȴ�SD���ϵ����
    //�Ȳ���>74�����壬��SD���Լ���ʼ�����
    for (i = 0; i < 10; i++)SPIx_ReadWriteByte(0xFF);
    //-----------------SD����λ��idle��ʼ-----------------
    //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
    //��ʱ��ֱ���˳�
    retry = 0;
    do
    {
        //����CMD0����SD������IDLE״̬
        i = SD_SendCmd(CMD0, 0, 0x95);
        retry++;
    }
    while ((i != 0x01) && (retry < 200));
    //����ѭ���󣬼��ԭ�򣺳�ʼ���ɹ���or ���Գ�ʱ��
    if (retry == 200)return 1; //ʧ��
    return 0;//�ɹ�
}

//��ʼ��SD��
u8 SD_Initialize(void)
{
    u8 r1;      // ���SD���ķ���ֵ
    u16 retry;  // �������г�ʱ����
    u8 buf[4];
    u16 i;


    SPI1_Configuration();
    SD_SPI_SpeedLow();  //���õ�����ģʽ

    delay_ms(1000);   //SPI���߳�ʼ����ʱһ���ٳ�ʼ��SD��

    for (i = 0; i < 10; i++)SD_SPI_ReadWriteByte(0XFF); //��������74������
    retry = 2000;
    do
    {
        r1 = SD_SendCmd(0, 0, 0x95); //����IDLE״̬
    }
    while ((r1 != 0X01) && retry--);
    SD_Type = 0; //Ĭ���޿�
    if (r1 == 0X01)
    {
        if (SD_SendCmd(8, 0x1AA, 0x87) == 1) //SD V2.0
        {
            for (i = 0; i < 4; i++)buf[i] = SD_SPI_ReadWriteByte(0XFF); //Get trailing return value of R7 resp
            if (buf[2] == 0X01 && buf[3] == 0XAA) //���Ƿ�֧��2.7~3.6V
            {
                retry = 0XFFFE;
                do
                {
                    SD_SendCmd(55, 0, 0X01); //����CMD55
                    r1 = SD_SendCmd(41, 0x40000000, 0X01); //����CMD41
                }
                while (r1 && retry--);
                if (retry && SD_SendCmd(58, 0, 0X01) == 0) //����SD2.0���汾��ʼ
                {
                    for (i = 0; i < 4; i++)buf[i] = SD_SPI_ReadWriteByte(0XFF); //�õ�OCRֵ
                    if (buf[0] & 0x40)SD_Type = SD_TYPE_V2HC; //���CCS
                    else SD_Type = SD_TYPE_V2;
                }
            }
        }
        else //SD V1.x/ MMC V3
        {
            SD_SendCmd(55, 0, 0X01);    //����CMD55
            r1 = SD_SendCmd(41, 0, 0X01); //����CMD41
            if (r1 <= 1)
            {
                SD_Type = SD_TYPE_V1;
                retry = 0XFFFE;
                do //�ȴ��˳�IDLEģʽ
                {
                    SD_SendCmd(55, 0, 0X01); //����CMD55
                    r1 = SD_SendCmd(41, 0, 0X01); //����CMD41
                }
                while (r1 && retry--);
            }
            else
            {
                SD_Type = SD_TYPE_MMC; //MMC V3
                retry = 0XFFFE;
                do //�ȴ��˳�IDLEģʽ
                {
                    r1 = SD_SendCmd(1, 0, 0X01); //����CMD1
                }
                while (r1 && retry--);
            }
            if (retry == 0 || SD_SendCmd(16, 512, 0X01) != 0)SD_Type = SD_TYPE_ERR; //����Ŀ�
        }
    }
    SD_DisSelect();//ȡ��Ƭѡ
    SD_SPI_SpeedHigh();//SPI����Ϊ����ģʽ

    if (SD_Type)
        return 0;
    else if (r1)
        return r1;
    return (0xaa);//��������
}



//��SD��
//buf:���ݻ�����
//sector:����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_ReadDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 r1;
    if (SD_Type != SD_TYPE_V2HC)sector <<= 9; //ת��Ϊ�ֽڵ�ַ
    if (cnt == 1)
    {
        r1 = SD_SendCmd(17, sector, 0X01); //������
        if (r1 == 0) //ָ��ͳɹ�
        {
            r1 = SD_RecvData(buf, 512); //����512���ֽ�
        }
    }
    else
    {
        r1 = SD_SendCmd(18, sector, 0X01); //����������
        do
        {
            r1 = SD_RecvData(buf, 512); //����512���ֽ�
            buf += 512;
        }
        while (--cnt && r1 == 0);
        SD_SendCmd(12, 0, 0X01); //����ֹͣ����
    }
    SD_DisSelect();//ȡ��Ƭѡ
    return r1;//
}



//дSD��
//buf:���ݻ�����
//sector:��ʼ����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 r1;
    if (SD_Type != SD_TYPE_V2HC)sector *= 512; //ת��Ϊ�ֽڵ�ַ
    if (cnt == 1)
    {
        r1 = SD_SendCmd(24, sector, 0X01); //������
        if (r1 == 0) //ָ��ͳɹ�
        {
            r1 = SD_SendBlock(buf, 0xFE); //д512���ֽ�
        }
    }
    else
    {
        if (SD_Type != SD_TYPE_MMC)
        {
            SD_SendCmd(55, 0, 0X01);
            SD_SendCmd(23, cnt, 0X01); //����ָ��
        }
        r1 = SD_SendCmd(25, sector, 0X01); //����������
        if (r1 == 0)
        {
            do
            {
                r1 = SD_SendBlock(buf, 0xFC); //����512���ֽ�
                buf += 512;
            }
            while (--cnt && r1 == 0);
            r1 = SD_SendBlock(0, 0xFD); //����512���ֽ�
        }
    }
    SD_DisSelect();//ȡ��Ƭѡ
    return r1;//
}
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
{
    u16 retry;
    u8 r1;
    //����һ�δ���
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    retry = 0;
    do
    {
        r1 = SD_SPI_ReadWriteByte(0xFF);
        retry++;
        if (retry > 4000) //4000�εȴ���û��Ӧ���˳������ɶ��Լ��Σ�
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return 1;
        }
    }
    //�ȴ�SD������������ʼ����0xFE
    while (r1 != 0xFE);
    //����ѭ���󣬿�ʼ��������
    while (len--)
    {
        *data = SD_SPI_ReadWriteByte(0xFF);
        data++;
    }
    //����2��αCRC
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);
    //�����ͷ�����
    if (release == ENABLE)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        SD_SPI_ReadWriteByte(0xFF);
    }
    return 0;
}
u8 SD_ReadSingleBlock(u32 sector, u8 *buffer)
{
    u8 r1;
    //����ģʽ
    SD_SPI_SpeedHigh();
    if (SD_Type != SD_TYPE_V2HC) //�������SDHC��
    {
        sector = sector << 9; //512*sector�����������ı߽�����ַ
    }
    r1 = SD_SendCmd(CMD17, sector, 1);   //����CMD17 ������
    if (r1 != 0x00)   return r1;
    r1 = SD_RecvData(buffer, 512);   //һ������Ϊ512�ֽ�
    if (r1 != 0)
        return r1;   //�����ݳ���
    else
        return 0;      //��ȡ��ȷ������0
}



u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
{
    u8 r1;
    SD_SPI_SpeedHigh();
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    r1 = SD_SendCmd(CMD18, sector, 1);        //���������
    if (r1 != 0x00)    return r1;
    do    //��ʼ��������
    {
        if (SD_ReceiveData(buffer, 512, DISABLE) != 0x00)
        {
            break;
        }
        buffer += 512;
    }
    while (--count);

    SD_SendCmd(CMD12, 0, 1);  //ȫ��������ɣ�����ֹͣ����
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xFF);
    if (count != 0)
        return count;   //���û�д��꣬����ʣ�����
    else
        return 0;
}



u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
{
    u8 r1;
    u16 i;
    u16 retry;
    //����ģʽ
    SD_SPI_SpeedHigh();
    //�������SDHC������sector��ַתΪbyte��ַ
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    //д����ָ��
    r1 = SD_SendCmd(CMD24, sector, 0x00);
    if (r1 != 0x00)
    {
        //Ӧ�����ֱ�ӷ���
        return r1;
    }
    //��ʼ׼�����ݴ���
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //����ʼ����0xFE
    SD_SPI_ReadWriteByte(0xFE);
    //��һ��sector����
    for (i = 0; i < 512; i++)
    {
        SD_SPI_ReadWriteByte(*data++);
    }
    //����2��αCRCУ��
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //�ȴ�SD��Ӧ��
    r1 = SD_SPI_ReadWriteByte(0xff);
    //���Ϊ0x05˵������д��ɹ�
    if ((r1 & 0x1F) != 0x05)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        return r1;
    }
    //�ȴ��������
    retry = 0;
    //���Ա��ʱ�������߱�����
    while (!SD_SPI_ReadWriteByte(0xff))
    {
        retry++;
        if (retry > 65534)     //�����ʱ��û��д����ɣ��˳�����
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return 1;           //д�볬ʱ������1
        }
    }
    //д����ɣ�Ƭѡ��1
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xff);
    return 0;
}

u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count)
{
    u8 r1;
    u16 i;
    SD_SPI_SpeedHigh();
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    if (SD_Type != SD_TYPE_MMC)
    {
        //����ACMD23ָ��ʹ��Ԥ����
        r1 = SD_SendCmd(23, count, 0x01);
    }
    //д���ָ��CMD25
    r1 = SD_SendCmd(CMD25, sector, 0x01);
    //Ӧ����ȷ��ֱ�ӷ���
    if (r1 != 0x00)  return r1;
    //��ʼ׼�����ݴ���
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    //��3����������SD��׼����
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //������N��sectorѭ��д��Ĳ���
    do
    {
        //����ʼ����0xFC�������Ƕ��д��
        SD_SPI_ReadWriteByte(0xFC);
        //��1��sector������
        for (i = 0; i < 512; i++)
        {
            SD_SPI_ReadWriteByte(*data++);
        }
        //��2��αCRC
        SD_SPI_ReadWriteByte(0xff);
        SD_SPI_ReadWriteByte(0xff);
        //�ȴ�SD����Ӧ
        r1 = SD_SPI_ReadWriteByte(0xff);
        //0x05��ʾ����д��ɹ�
        if ((r1 & 0x1F) != 0x05)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return r1;
        }
        //���SD��æ�ź�
        if (SD_WaitReady() == 1)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);   //��ʱ��д��δ��ɣ��˳�
            return 1;
        }
    }
    while (--count);
    //���ʹ����������0xFD
    SD_SPI_ReadWriteByte(0xFD);
    //�ȴ�׼����
    if (SD_WaitReady())
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        return 1;
    }
    //д����ɣ�Ƭѡ��1
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xff);
    //����countֵ�����д�꣬��count=0,����count=δд���sector��
    return count;
}


u8 send_data[1536] = {0};
u8 receive_data[1536] = {0};
int main(void)
{
    uint64_t CardSize = 0;
    u16 i;
    uart_init(115200);
    for (i = 0; i < 1536; i++)
        send_data[i] = 'D';
    switch (SD_Initialize())
    {
    case 0:
        printf("\r\nSD Card Init Success!\r\n");
        break;
    case 1:
        printf("Time Out!\n");
        break;
    case 99:
        printf("No Card!\n");
        break;
    default:
        printf("unknown err\n");
        break;
    }
    CardSize = SD_GetSectorCount();
    CardSize = CardSize * 512 / 1024 / 1024;
    printf("# SD Card Size: %lldMB\r\n", CardSize);
    SD_WriteSingleBlock(30, send_data);
    SD_ReadSingleBlock(30, receive_data);
//  if(Buffercmp(send_data,receive_data,512))
//  {
//      printf("\r\n single read and write success \r\n");
//  //USART1_Puts(receive_data);
//  }
    SD_WriteMultiBlock(50, send_data, 3);
    SD_ReadMultiBlock(50, receive_data, 3);
//  if(Buffercmp(send_data,receive_data,1536))
//  {
//  printf("\r\n    multi read and write success \r\n");
//  //USART1_Puts(receive_data);
//  }
    while (1);
}




