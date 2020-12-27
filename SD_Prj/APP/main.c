#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
/************************************************
 ALIENTEK ̽����STM32F407������ ʵ��4
 ����ʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

int main(void)
{

    HAL_Init();                     //��ʼ��HAL��
    Stm32_Clock_Init(336, 8, 2, 7); //����ʱ��,168Mhz
    delay_init(168);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
    LED_Init();                     //��ʼ��LED
	printf("\r\n��ӭʹ��Ұ��  STM32 F407 �����塣\r\n");	
	
	printf("�ڿ�ʼ����SD����������ǰ��������������32G���ڵ�SD��\r\n");			
	printf("��������SD������ ���ļ�ϵͳ ��ʽ��д����ɾ��SD�����ļ�ϵͳ\r\n");		
	printf("ʵ����ͨ�����Ը�ʽ����ʹ��SD���ļ�ϵͳ�����ָ̻�SD���ļ�ϵͳ\r\n");		
	printf("\r\n ��sd���ڵ�ԭ�ļ����ɻָ���ʵ��ǰ��ر���SD���ڵ�ԭ�ļ�������\r\n");		
	printf("\r\n ����ȷ�ϣ��밴�������KEY1��������ʼSD������ʵ��....\r\n");	
    while (1)
    {

        LED0 = !LED0; //��˸LED,��ʾϵͳ��������.
        delay_ms(100);

    }
}

