#ifndef COMMON_H
#define COMMON_H

#pragma anon_unions

// Workaround for Keil syntax highlighting
#ifndef USE_STDPERIPH_DRIVER
    #define USE_STDPERIPH_DRIVER
#endif

#ifndef STM32F429_439xx
    #define STM32F429_439xx
#endif

#include "stm32f4xx_conf.h"
//////////////////////////////////////////

#define NOP __ASM("nop")

// GPIO
#define GPIO_INPUT_MODE  0x00000000
#define GPIO_OUTPUT_MODE 0x55555555

typedef enum bool_e
{
    false,
    true
}
bool;

// Flags
extern bool LCD_READY_FLAG;

// Null pointer define
#define NULL 0

#endif //COMMON_H
