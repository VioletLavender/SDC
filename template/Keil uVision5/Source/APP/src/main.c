#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"     /* FatFs lower layer API */
#include <string.h>
#include <stdio.h>
#include "test.h"


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //设置系统中断优先级分组2
    delay_init(168);                                    //延时初始化
    uart_init(115200);                                  //串口初始化波特率为115200
    SD_Card_init();
    SD_Card_Readbin();
    while (1)
    {
        delay_ms(1000);
    }
}

