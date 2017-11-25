#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "input_config.h"

// Max timer value is 65534 so this time can never be reached
// This effectivly locks the button until it's released (second unlock condition)
#define INPUT_LOCK_UNTIL_RELEASED 65535

typedef long time_t;

typedef enum
{
    INPUT_A_ID,
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

typedef struct
{
    Input_Button_ID_t ID;       // ID of the button the lock is associated with
    time_t LockedSince;         // LockTimer value at the time lock got activated
    time_t LockedFor;           // Time the button should remain locked if not released in ms
    bool IsEnabled;             // State of the lock
}
Input_Lock_t;

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

void Input_UpdateLocks(void);
bool Input_IsLocked(Input_Button_ID_t id);
void Input_Lock(Input_Button_ID_t id, time_t lockTime);
void Input_LockAll(time_t lockTime);

void TIM3_IRQHandler(void);

#endif //INPUT_H
