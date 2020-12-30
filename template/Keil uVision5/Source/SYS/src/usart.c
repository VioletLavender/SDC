/**
  ******************************************************************************
  * @file    main.c
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
#define __USART_C__


/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "usart.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
u8 USART_RX_BUF[USART_REC_LEN];                                         //接收缓冲,最大USART_REC_LEN个字节.
u16 USART_RX_STA = 0;                                                   //接收状态标记
FILE __stdout;


/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


#pragma import(__use_no_semihosting)


struct __FILE                                                           //标准库需要的支持函数
{
    int handle;
};


/**
  * @brief  
  * @param  None
  * @retval None
  */
void _sys_exit(int x)                                                   //定义_sys_exit()以避免使用半主机模式
{
    x = x;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);                                   //循环发送,直到发送完毕
    USART1->DR = (u8) ch;
    return ch;
}


/**
  * @brief  
  * @param  None
  * @retval None
  */
void uart_init(u32 bound)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); 
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 

    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
    USART_Init(USART1, &USART_InitStructure); 

    USART_Cmd(USART1, ENABLE); 
    
}


/******************* (C) COPYRIGHT 2020 *************************END OF FILE***/
