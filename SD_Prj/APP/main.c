#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
/************************************************
 ALIENTEK 探索者STM32F407开发板 实验4
 串口实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司
 作者：正点原子 @ALIENTEK
************************************************/

int main(void)
{

    HAL_Init();                     //初始化HAL库
    Stm32_Clock_Init(336, 8, 2, 7); //设置时钟,168Mhz
    delay_init(168);                //初始化延时函数
    uart_init(115200);              //初始化USART
    LED_Init();                     //初始化LED
	printf("\r\n欢迎使用野火  STM32 F407 开发板。\r\n");	
	
	printf("在开始进行SD卡基本测试前，请给开发板插入32G以内的SD卡\r\n");			
	printf("本程序会对SD卡进行 非文件系统 方式读写，会删除SD卡的文件系统\r\n");		
	printf("实验后可通过电脑格式化或使用SD卡文件系统的例程恢复SD卡文件系统\r\n");		
	printf("\r\n 但sd卡内的原文件不可恢复，实验前务必备份SD卡内的原文件！！！\r\n");		
	printf("\r\n 若已确认，请按开发板的KEY1按键，开始SD卡测试实验....\r\n");	
    while (1)
    {

        LED0 = !LED0; //闪烁LED,提示系统正在运行.
        delay_ms(100);

    }
}

