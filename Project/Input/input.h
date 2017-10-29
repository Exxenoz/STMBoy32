#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "input_config.h"

typedef enum INPUT_InterruptFlags_e
{
    INPUT_INTERRUPT_FLAG_A           = 0x0002,
    INPUT_INTERRUPT_FLAG_B           = 0x0004,
    INPUT_INTERRUPT_FLAG_SELECT      = 0x0008,
    INPUT_INTERRUPT_FLAG_START       = 0x0010,
    INPUT_INTERRUPT_FLAG_FADE_RIGHT  = 0x0020,
    INPUT_INTERRUPT_FLAG_FADE_LEFT   = 0x0040,
    INPUT_INTERRUPT_FLAG_FADE_TOP    = 0x0080,
    INPUT_INTERRUPT_FLAG_FADE_BOT    = 0x0100
} Input_InterruptFlags_t;

typedef enum INPUT_ButtonState_e
{
    INPUT_NOT_PRESSED = 0xFFFF,
    INPUT_PRESSED     = 0x0000
} Input_ButtonState_t;

extern uint16_t Input_Interrupt_Flags;

void Input_Initialize(void);
void Input_UpdateJoypadState(void);

void TIM3_IRQHandler(void);

#endif //INPUT_H
