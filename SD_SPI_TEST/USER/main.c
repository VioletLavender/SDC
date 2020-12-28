#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06
#define CMD0    0       //卡复位
#define CMD1    1
#define CMD8    8       //命令8 ，SEND_IF_COND
#define CMD9    9       //命令9 ，读CSD数据
#define CMD10   10      //命令10，读CID数据
#define CMD12   12      //命令12，停止数据传输
#define CMD16   16      //命令16，设置SectorSize 应返回0x00
#define CMD17   17      //命令17，读sector
#define CMD18   18      //命令18，读Multi sector
#define CMD23   23      //命令23，设置多sector写入前预先擦除N个block
#define CMD24   24      //命令24，写sector
#define CMD25   25      //命令25，写Multi sector
#define CMD41   41      //命令41，应返回0x00
#define CMD55   55      //命令55，应返回0x01
#define CMD58   58      //命令58，读OCR信息
#define CMD59   59      //命令59，使能/禁止CRC，应返回0x00
//数据写入回应字意义
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD卡回应标记字
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF
//ALIENTEK 探索者STM32F407开发板 实验1
//跑马灯实验 -库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com
//广州市星翼电子科技有限公司
//作者：正点原子 @ALIENTEK
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
    GPIO_SetBits(GPIOA, GPIO_Pin_4);//低电平选通SD卡，高电平隔离SD卡

    SPI_I2S_DeInit(SPI1);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //主器件
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //8位数据长度
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;         //这里要注意，一定要配置为上升沿数据有效，因为SD卡为上升沿数据有效
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;        //这里要注意，一定要配置为SPI_CPHA_2Edge（数据捕获于第2个时钟沿），参见SD卡协议要求
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;           //NSS信号由外部管脚管理
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//SPI速度为低速
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  //数据传输的第一个字节为MSB
    SPI_InitStructure.SPI_CRCPolynomial = 7;            //CRC的多项式
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
}





uint8_t SPIx_ReadWriteByte(uint8_t _ucByte)
{
    uint8_t ch;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);     // 等待发送缓冲区空
    SPI_I2S_SendData(SPI1, _ucByte);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    // 等待数据接收完毕
    ch = SPI_I2S_ReceiveData(SPI1);
    return (ch);
}

u8 SD_SPI_ReadWriteByte(u8 data)
{
    return SPIx_ReadWriteByte(data);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

u8  SD_Type = 0; //SD卡的类型

void SPIx_SetSpeed(u8 SpeedSet)
{
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

void SD_SPI_SpeedLow(void)
{
    SPIx_SetSpeed(SPI_BaudRatePrescaler_256);//设置到低速模式
}
//SD卡正常工作的时候,可以高速了
void SD_SPI_SpeedHigh(void)
{
    SPIx_SetSpeed(SPI_BaudRatePrescaler_2);//设置到高速模式
}
//SD卡初始化的时候,需要低速


//取消选择,释放SPI总线
void SD_DisSelect(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
}

u8 SD_WaitReady(void)
{
    u32 t = 0;
    do
    {
        if (SD_SPI_ReadWriteByte(0XFF) == 0XFF)return 0; //OK
        t++;
    }
    while (t < 0XFFFFFF); //等待
    return 1;
}

//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
u8 SD_Select(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    if (SD_WaitReady() == 0)return 0; //等待成功
    SD_DisSelect();
    return 1;//等待失败
}



//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 Response)
{
    u16 Count = 0xFFF; //等待次数
    while ((SD_SPI_ReadWriteByte(0XFF) != Response) && Count)Count--; //等待得到准确的回应
    if (Count == 0)return MSD_RESPONSE_FAILURE; //得到回应失败
    else return MSD_RESPONSE_NO_ERROR;//正确回应
}



//从sd卡读取一个数据包的内容
//buf:数据缓存区
//len:要读取的数据长度.
//返回值:0,成功;其他,失败;
u8 SD_RecvData(u8 *buf, u16 len)
{
    if (SD_GetResponse(0xFE))return 1; //等待SD卡发回数据起始令牌0xFE
    while (len--) //开始接收数据
    {
        *buf = SPIx_ReadWriteByte(0xFF);
        buf++;
    }
    //下面是2个伪CRC（dummy CRC）
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);
    return 0;//读取成功
}



//向sd卡写入一个数据包的内容 512字节
//buf:数据缓存区
//cmd:指令
//返回值:0,成功;其他,失败;
u8 SD_SendBlock(u8 *buf, u8 cmd)
{
    u16 t;
    if (SD_WaitReady())return 1; //等待准备失效
    SD_SPI_ReadWriteByte(cmd);
    if (cmd != 0XFD) //不是结束指令
    {
        for (t = 0; t < 512; t++)SPIx_ReadWriteByte(buf[t]); //提高速度,减少函数传参时间
        SD_SPI_ReadWriteByte(0xFF);//忽略crc
        SD_SPI_ReadWriteByte(0xFF);
        t = SD_SPI_ReadWriteByte(0xFF); //接收响应
        if ((t & 0x1F) != 0x05)return 2; //响应错误
    }
    return 0;//写入成功
}



//向SD卡发送一个命令
//输入: u8 cmd   命令
//      u32 arg  命令参数
//      u8 crc   crc校验值
//返回值:SD卡返回的响应
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;
    u8 Retry = 0;
    SD_DisSelect();//取消上次片选
    if (SD_Select())return 0XFF; //片选失效
    //发送
    SD_SPI_ReadWriteByte(cmd | 0x40);//分别写入命令
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);
    SD_SPI_ReadWriteByte(crc);
    if (cmd == CMD12)SD_SPI_ReadWriteByte(0xff); //Skip a stuff byte when stop reading
    //等待响应，或超时退出
    Retry = 0X1F;
    do
    {
        r1 = SD_SPI_ReadWriteByte(0xFF);
    }
    while ((r1 & 0X80) && Retry--);
    //返回状态值
    return r1;
}



//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）
//返回值:0：NO_ERR
//       1：错误
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;
    //发CMD10命令，读CID
    r1 = SD_SendCmd(CMD10, 0, 0x01);
    if (r1 == 0x00)
    {
        r1 = SD_RecvData(cid_data, 16); //接收16个字节的数据
    }
    SD_DisSelect();//取消片选
    if (r1)return 1;
    else return 0;
}

//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）
//返回值:0：NO_ERR
//       1：错误
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;
    r1 = SD_SendCmd(CMD9, 0, 0x01); //发CMD9命令，读CSD
    if (r1 == 0)
    {
        r1 = SD_RecvData(csd_data, 16); //接收16个字节的数据
    }
    SD_DisSelect();//取消片选
    if (r1)return 1;
    else return 0;
}


//获取SD卡的总扇区数（扇区数）
//返回值:0： 取容量出错
//       其他:SD卡的容量(扇区数/512字节)
//每扇区的字节数必为512，因为如果不是512，则初始化不能通过.
u32 SD_GetSectorCount(void)
{
    u8 csd[16];
    u32 Capacity;
    u8 n;
    u16 csize;
    //取CSD信息，如果期间出错，返回0
    if (SD_GetCSD(csd) != 0) return 0;
    //如果为SDHC卡，按照下面方式计算
    if ((csd[0] & 0xC0) == 0x40) //V2.00的卡
    {
        csize = csd[9] + ((u16)csd[8] << 8) + 1;
        Capacity = (u32)csize << 10;//得到扇区数
    }
    else //V1.XX的卡
    {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
        Capacity = (u32)csize << (n - 9); //得到扇区数
    }
    return Capacity;
}

u8 SD_Idle_Sta(void)
{
    u16 i;
    u8 retry;
    for (i = 0; i < 0xf00; i++); //纯延时，等待SD卡上电完成
    //先产生>74个脉冲，让SD卡自己初始化完成
    for (i = 0; i < 10; i++)SPIx_ReadWriteByte(0xFF);
    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {
        //发送CMD0，让SD卡进入IDLE状态
        i = SD_SendCmd(CMD0, 0, 0x95);
        retry++;
    }
    while ((i != 0x01) && (retry < 200));
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if (retry == 200)return 1; //失败
    return 0;//成功
}

//初始化SD卡
u8 SD_Initialize(void)
{
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buf[4];
    u16 i;


    SPI1_Configuration();
    SD_SPI_SpeedLow();  //设置到低速模式

    delay_ms(1000);   //SPI总线初始化延时一会再初始化SD卡

    for (i = 0; i < 10; i++)SD_SPI_ReadWriteByte(0XFF); //发送最少74个脉冲
    retry = 2000;
    do
    {
        r1 = SD_SendCmd(0, 0, 0x95); //进入IDLE状态
    }
    while ((r1 != 0X01) && retry--);
    SD_Type = 0; //默认无卡
    if (r1 == 0X01)
    {
        if (SD_SendCmd(8, 0x1AA, 0x87) == 1) //SD V2.0
        {
            for (i = 0; i < 4; i++)buf[i] = SD_SPI_ReadWriteByte(0XFF); //Get trailing return value of R7 resp
            if (buf[2] == 0X01 && buf[3] == 0XAA) //卡是否支持2.7~3.6V
            {
                retry = 0XFFFE;
                do
                {
                    SD_SendCmd(55, 0, 0X01); //发送CMD55
                    r1 = SD_SendCmd(41, 0x40000000, 0X01); //发送CMD41
                }
                while (r1 && retry--);
                if (retry && SD_SendCmd(58, 0, 0X01) == 0) //鉴别SD2.0卡版本开始
                {
                    for (i = 0; i < 4; i++)buf[i] = SD_SPI_ReadWriteByte(0XFF); //得到OCR值
                    if (buf[0] & 0x40)SD_Type = SD_TYPE_V2HC; //检查CCS
                    else SD_Type = SD_TYPE_V2;
                }
            }
        }
        else //SD V1.x/ MMC V3
        {
            SD_SendCmd(55, 0, 0X01);    //发送CMD55
            r1 = SD_SendCmd(41, 0, 0X01); //发送CMD41
            if (r1 <= 1)
            {
                SD_Type = SD_TYPE_V1;
                retry = 0XFFFE;
                do //等待退出IDLE模式
                {
                    SD_SendCmd(55, 0, 0X01); //发送CMD55
                    r1 = SD_SendCmd(41, 0, 0X01); //发送CMD41
                }
                while (r1 && retry--);
            }
            else
            {
                SD_Type = SD_TYPE_MMC; //MMC V3
                retry = 0XFFFE;
                do //等待退出IDLE模式
                {
                    r1 = SD_SendCmd(1, 0, 0X01); //发送CMD1
                }
                while (r1 && retry--);
            }
            if (retry == 0 || SD_SendCmd(16, 512, 0X01) != 0)SD_Type = SD_TYPE_ERR; //错误的卡
        }
    }
    SD_DisSelect();//取消片选
    SD_SPI_SpeedHigh();//SPI配置为高速模式

    if (SD_Type)
        return 0;
    else if (r1)
        return r1;
    return (0xaa);//其他错误
}



//读SD卡
//buf:数据缓存区
//sector:扇区
//cnt:扇区数
//返回值:0,ok;其他,失败.
u8 SD_ReadDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 r1;
    if (SD_Type != SD_TYPE_V2HC)sector <<= 9; //转换为字节地址
    if (cnt == 1)
    {
        r1 = SD_SendCmd(17, sector, 0X01); //读命令
        if (r1 == 0) //指令发送成功
        {
            r1 = SD_RecvData(buf, 512); //接收512个字节
        }
    }
    else
    {
        r1 = SD_SendCmd(18, sector, 0X01); //连续读命令
        do
        {
            r1 = SD_RecvData(buf, 512); //接收512个字节
            buf += 512;
        }
        while (--cnt && r1 == 0);
        SD_SendCmd(12, 0, 0X01); //发送停止命令
    }
    SD_DisSelect();//取消片选
    return r1;//
}



//写SD卡
//buf:数据缓存区
//sector:起始扇区
//cnt:扇区数
//返回值:0,ok;其他,失败.
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 r1;
    if (SD_Type != SD_TYPE_V2HC)sector *= 512; //转换为字节地址
    if (cnt == 1)
    {
        r1 = SD_SendCmd(24, sector, 0X01); //读命令
        if (r1 == 0) //指令发送成功
        {
            r1 = SD_SendBlock(buf, 0xFE); //写512个字节
        }
    }
    else
    {
        if (SD_Type != SD_TYPE_MMC)
        {
            SD_SendCmd(55, 0, 0X01);
            SD_SendCmd(23, cnt, 0X01); //发送指令
        }
        r1 = SD_SendCmd(25, sector, 0X01); //连续读命令
        if (r1 == 0)
        {
            do
            {
                r1 = SD_SendBlock(buf, 0xFC); //接收512个字节
                buf += 512;
            }
            while (--cnt && r1 == 0);
            r1 = SD_SendBlock(0, 0xFD); //接收512个字节
        }
    }
    SD_DisSelect();//取消片选
    return r1;//
}
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
{
    u16 retry;
    u8 r1;
    //启动一次传输
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    retry = 0;
    do
    {
        r1 = SD_SPI_ReadWriteByte(0xFF);
        retry++;
        if (retry > 4000) //4000次等待后没有应答，退出报错（可多试几次）
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return 1;
        }
    }
    //等待SD卡发回数据起始令牌0xFE
    while (r1 != 0xFE);
    //跳出循环后，开始接收数据
    while (len--)
    {
        *data = SD_SPI_ReadWriteByte(0xFF);
        data++;
    }
    //发送2个伪CRC
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);
    //按需释放总线
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
    //高速模式
    SD_SPI_SpeedHigh();
    if (SD_Type != SD_TYPE_V2HC) //如果不是SDHC卡
    {
        sector = sector << 9; //512*sector即物理扇区的边界对其地址
    }
    r1 = SD_SendCmd(CMD17, sector, 1);   //发送CMD17 读命令
    if (r1 != 0x00)   return r1;
    r1 = SD_RecvData(buffer, 512);   //一个扇区为512字节
    if (r1 != 0)
        return r1;   //读数据出错
    else
        return 0;      //读取正确，返回0
}



u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
{
    u8 r1;
    SD_SPI_SpeedHigh();
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    r1 = SD_SendCmd(CMD18, sector, 1);        //读多块命令
    if (r1 != 0x00)    return r1;
    do    //开始接收数据
    {
        if (SD_ReceiveData(buffer, 512, DISABLE) != 0x00)
        {
            break;
        }
        buffer += 512;
    }
    while (--count);

    SD_SendCmd(CMD12, 0, 1);  //全部传输完成，发送停止命令
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xFF);
    if (count != 0)
        return count;   //如果没有传完，返回剩余个数
    else
        return 0;
}



u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
{
    u8 r1;
    u16 i;
    u16 retry;
    //高速模式
    SD_SPI_SpeedHigh();
    //如果不是SDHC卡，将sector地址转为byte地址
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    //写扇区指令
    r1 = SD_SendCmd(CMD24, sector, 0x00);
    if (r1 != 0x00)
    {
        //应答错误，直接返回
        return r1;
    }
    //开始准备数据传输
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    //先发3个空数据，等待SD卡准备好
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //放起始令牌0xFE
    SD_SPI_ReadWriteByte(0xFE);
    //发一个sector数据
    for (i = 0; i < 512; i++)
    {
        SD_SPI_ReadWriteByte(*data++);
    }
    //发送2个伪CRC校验
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //等待SD卡应答
    r1 = SD_SPI_ReadWriteByte(0xff);
    //如果为0x05说明数据写入成功
    if ((r1 & 0x1F) != 0x05)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        return r1;
    }
    //等待操作完成
    retry = 0;
    //卡自编程时，数据线被拉低
    while (!SD_SPI_ReadWriteByte(0xff))
    {
        retry++;
        if (retry > 65534)     //如果长时间没有写入完成，退出报错
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return 1;           //写入超时，返回1
        }
    }
    //写入完成，片选置1
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
        //启用ACMD23指令使能预擦除
        r1 = SD_SendCmd(23, count, 0x01);
    }
    //写多块指令CMD25
    r1 = SD_SendCmd(CMD25, sector, 0x01);
    //应答不正确，直接返回
    if (r1 != 0x00)  return r1;
    //开始准备数据传输
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    //放3个空数据让SD卡准备好
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    SD_SPI_ReadWriteByte(0xff);
    //下面是N个sector循环写入的部分
    do
    {
        //放起始令牌0xFC，表明是多块写入
        SD_SPI_ReadWriteByte(0xFC);
        //发1个sector的数据
        for (i = 0; i < 512; i++)
        {
            SD_SPI_ReadWriteByte(*data++);
        }
        //发2个伪CRC
        SD_SPI_ReadWriteByte(0xff);
        SD_SPI_ReadWriteByte(0xff);
        //等待SD卡回应
        r1 = SD_SPI_ReadWriteByte(0xff);
        //0x05表示数据写入成功
        if ((r1 & 0x1F) != 0x05)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);
            return r1;
        }
        //检测SD卡忙信号
        if (SD_WaitReady() == 1)
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_4);   //长时间写入未完成，退出
            return 1;
        }
    }
    while (--count);
    //发送传输结束令牌0xFD
    SD_SPI_ReadWriteByte(0xFD);
    //等待准备好
    if (SD_WaitReady())
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        return 1;
    }
    //写入完成，片选置1
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SD_SPI_ReadWriteByte(0xff);
    //返回count值，如果写完，则count=0,否则count=未写完的sector数
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




