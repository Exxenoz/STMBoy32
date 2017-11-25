#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "input_config.h"

typedef enum
{
typedef enum
{
    INPUT_NOT_PRESSED = 0x0000,
    INPUT_PRESSED     = 0xFFFF
}
Input_ButtonState_t;

typedef union
{
    struct
    {
        unsigned int ButtonA      : 1;
        unsigned int ButtonB      : 1;
        unsigned int ButtonStart  : 1;
        unsigned int ButtonSelect : 1;
        unsigned int FadeRight    : 1;
        unsigned int FadeLeft     : 1;
        unsigned int FadeTop      : 1;
        unsigned int FadeBot      : 1;
    };

    uint8_t allFlags;
}
Input_Interrupt_Flags_t;

extern Input_Interrupt_Flags_t Input_Interrupt_Flags;

void Input_Initialize(void);
void Input_UpdateGBCJoypad(void);

void TIM3_IRQHandler(void);

#endif //INPUT_H
