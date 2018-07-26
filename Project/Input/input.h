#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "input_config.h"

#define INPUT_FRAME_PORT  GPIOB
#define INPUT_FRAME_PIN   GPIO_PIN_0

// Max even timer period (65534) => 32 767 ms (timer runs with 2kHz)
#define INPUT_MAX_LOCK_TIME       (MAX_16BIT_TIMER_PERIOD - 1)

// Dynamic lock perimeters (used for scrolling a list)
#define INPUT_MIN_DYNAMIC_LOCK_TIME        10
#define INPUT_MAX_DYNAMIC_LOCK_TIME        150
#define INPUT_DYNAMIC_LOCK_TIME_STEP       10

// Locks button until it's released (second unlock condition) because value can never be reached
#define INPUT_LOCK_UNTIL_RELEASED (INPUT_MAX_LOCK_TIME + 1)

typedef uint16_t time_t;

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
    bool IsLocked;              // State of the lock
    bool WasUnlockedByTimeout;  // Indicates if last unlock was triggered by release or timeout
}
Input_Lock_t;

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

extern Input_Interrupt_Flags_t Input_Interrupt_Flags;

void Input_Initialize(void);
void Input_UpdateGBCJoypad(void);

void Input_UpdateLocks(void);
bool Input_IsLocked(Input_Button_ID_t id);
void Input_Lock(Input_Button_ID_t id, time_t lockTime);
void Input_LockAll(time_t lockTime);
void Input_LockDynamically(Input_Button_ID_t id);

void TIM2_IRQHandler(void);

#endif //INPUT_H
