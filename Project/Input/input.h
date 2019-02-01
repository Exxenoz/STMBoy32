#ifndef INPUT_H
#define INPUT_H

#include "common.h"


#define NUMBER_OF_INPUTS                                8
#define INPUT_POLLING_CYCLES_UNTIL_CONSIDERED_PRESSED   10


typedef enum
{
    INPUT_A_ID = 0,
    INPUT_B_ID,
    INPUT_SELECT_ID,
    INPUT_START_ID,
    INPUT_FADE_RIGHT_ID,
    INPUT_FADE_LEFT_ID,
    INPUT_FADE_TOP_ID,
    INPUT_FADE_BOT_ID,
}
Input_Button_ID_t;

typedef enum
{
    INPUT_NOT_PRESSED = 0x0000,
    INPUT_PRESSED     = 0xFFFF
}
Input_ButtonState_t;

#pragma pack(1)
typedef union
{
    struct
    {
        unsigned int ButtonA      : 1;
        unsigned int ButtonB      : 1;
        unsigned int ButtonSelect : 1;
        unsigned int ButtonStart  : 1;
        unsigned int FadeRight    : 1;
        unsigned int FadeLeft     : 1;
        unsigned int FadeTop      : 1;
        unsigned int FadeBot      : 1;
    };

    uint8_t allFlags;
}
Input_Interrupt_Flags_t;

#pragma pack(1)
typedef struct
{
    GPIO_TypeDef* Port;
    uint32_t Pin;
}
Input_Pins_t;


extern Input_Interrupt_Flags_t Input_Interrupt_Flags;
extern const Input_Pins_t      Input_Pins[8];


void Input_UpdateGBCJoypad(void);
void TIM2_IRQHandler(void);

#endif //INPUT_H
