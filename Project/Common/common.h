#ifndef COMMON_H
#define COMMON_H

#pragma anon_unions


#include "stm32h7xx_hal.h"


#define NOP __ASM("nop")

// Null pointer define
#define NULL                         0

#define INTERRUPT_PRIORITY_2         1                   // 0 is the highest priority, 16 the lowest
#define MAX_16BIT_TIMER_PERIOD       0xFFFF

// GPIO Modes
#define GPIO_INPUT_MODE              0x00000000
#define GPIO_OUTPUT_MODE             0x55555555


typedef uint16_t time_t;

// Boolean define
typedef enum bool_e
{
    false,
    true
}
bool;



// Custom string functions to ensure null termination
int  CmpStrings(const void *a, const void *b);
void CopyString(char *dest, const char *src, int destSize);
void CopyChars(char *dest, const char *src, int destSize, int srcSize);
void CopyWithoutSuffix(char *dest, const char *src, int destSize);
void AppendString(char *dest, const char *src, int destSize);
void AppendChars(char *dest, const char *src, int destSize, int srcSize);
int  CountChars(const char *src);

#endif //COMMON_H
