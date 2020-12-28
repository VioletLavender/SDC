#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"
#include "sys.h"

#define LEDR PCout(1)
#define LEDG PCout(2)
#define LEDB PCout(3)
#define LEDY PCout(4)

#define setBeep(x) PCout(13) = x

#define LEDR_Toggle() PCout(1)^=1
#define LEDG_Toggle() PCout(2)^=1
#define LEDB_Toggle() PCout(3)^=1
#define LEDY_Toggle() PCout(4)^=1


void LED_Config(void);
void Buzz_Config(void);
#endif


