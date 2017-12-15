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

// Null pointer define
#define NULL 0

#define INTERRUPT_PRIORITY_2    1                   // 0 is the highest priority, 16 the lowest
#define MAX_16BIT_TIMER_PERIOD  65535

// GPIO Modes
#define GPIO_INPUT_MODE  0x00000000
#define GPIO_OUTPUT_MODE 0x55555555

// Boolean define
typedef enum bool_e
{
    false,
    true
}
bool;

// Custom string functions to ensure null termination
void copyString(char *dest, const char *src, int destSize);
void copyWithoutSuffix(char *dest, const char *src, int destSize);
void appendString(char *dest, const char *src, int destSize);

#endif //COMMON_H
