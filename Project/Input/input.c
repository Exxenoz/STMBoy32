#include "input.h"
#include "lcd.h"
#include "gbc_mmu.h"

TIM_HandleTypeDef Input_LockTimerHandle = { .Instance = INPUT_LOCK_TIM };
TIM_HandleTypeDef Input_PollTimerHandle = { .Instance = INPUT_POLLING_TIM};

Input_Interrupt_Flags_t Input_Interrupt_Flags;

Input_ButtonState_t Input_LastState[8];
Input_ButtonState_t Input_CurrState[8];
uint8_t             Input_Counter[8];

Input_Lock_t Input_Locks[8];
time_t       Input_DynamicLockTimes[8];

const uint32_t Input_Pins[8] =
{
    INPUT_A_PIN,
    INPUT_B_PIN,
    INPUT_SELECT_PIN,
    INPUT_START_PIN,
    INPUT_FADE_RIGHT_PIN,
    INPUT_FADE_LEFT_PIN,
    INPUT_FADE_TOP_PIN,
    INPUT_FADE_BOTTOM_PIN,
};


void Input_InitializePins(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_GPIO_PIN(PORT, PIN)               \
    GPIO_InitObject.Mode  = GPIO_MODE_INPUT;              \
    GPIO_InitObject.Pin   = PIN;                           \
    GPIO_InitObject.Pull  = GPIO_PULLUP;                    \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH;       \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);                    \

    INITIALIZE_GPIO_PIN(INPUT_A_PORT,           INPUT_A_PIN);
    INITIALIZE_GPIO_PIN(INPUT_B_PORT,           INPUT_B_PIN);         
    INITIALIZE_GPIO_PIN(INPUT_START_PORT,       INPUT_START_PIN);
    INITIALIZE_GPIO_PIN(INPUT_SELECT_PORT,      INPUT_SELECT_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_TOP_PORT,    INPUT_FADE_TOP_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_RIGHT_PORT,  INPUT_FADE_RIGHT_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_BOTTOM_PORT, INPUT_FADE_BOTTOM_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_LEFT_PORT,   INPUT_FADE_LEFT_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FRAME_PORT,       INPUT_FRAME_PIN);
}

void Input_InitializeTimers(void)
{
    __TIM3_CLK_ENABLE();
    __TIM4_CLK_ENABLE();

    // Initialize lock timer handle
    Input_LockTimerHandle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;

    // Initialize lock timer
    Input_LockTimerHandle.Init.Prescaler         = 44999;               // Tim4 runs with 90MHz -> scale it to 2kHz
    Input_LockTimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Input_LockTimerHandle.Init.Period            = INPUT_MAX_LOCK_TIME; // Count 'til max even value -> min overflows/s
    Input_LockTimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    Input_LockTimerHandle.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&Input_LockTimerHandle);
    HAL_TIM_Base_Start(&Input_LockTimerHandle);

    // Initialize poll timer handle
    Input_PollTimerHandle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;

    // Initialise polling timer
    Input_PollTimerHandle.Init.Prescaler         = 5999;                // Tim3 runs with 90MHz -> scale it to 15kHz
    Input_PollTimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Input_PollTimerHandle.Init.Period            = 14;                  // Count 'til 14 (+1) -> 1000 overflows/s
    Input_PollTimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    Input_PollTimerHandle.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&Input_PollTimerHandle);
    HAL_TIM_Base_Start_IT(&Input_PollTimerHandle);

    HAL_NVIC_SetPriority(INPUT_POLLING_TIM_NVIC_CHANNEL, INTERRUPT_PRIORITY_2, INTERRUPT_PRIORITY_2);
    HAL_NVIC_EnableIRQ(INPUT_POLLING_TIM_NVIC_CHANNEL);
}

void Input_Initialize() 
{
    Input_InitializePins();
    Input_InitializeTimers();

    Input_Interrupt_Flags.allFlags = 0x00;
    for (int i = 0; i < 8; i++)
    {
        Input_CurrState[i] = INPUT_NOT_PRESSED;
        Input_LastState[i] = INPUT_NOT_PRESSED;
        Input_Counter[i]   = 0;

        Input_Locks[i].ID                   = i;
        Input_Locks[i].LockedFor            = false;
        Input_Locks[i].LockedSince          = false;
        Input_Locks[i].IsLocked             = false;
        Input_Locks[i].WasUnlockedByTimeout = false;

        Input_DynamicLockTimes[i] = INPUT_MAX_DYNAMIC_LOCK_TIME;
    }

    //-----------DEBUG LED----------
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitObject.Pin   = GPIO_PIN_14;
    GPIO_InitObject.Pull  = GPIO_NOPULL;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitObject);
    //------------------------------
}

void Input_UpdateGBCJoypad(void)
{
    // Interrupt-flags have to be inverted because GBGames have inverted logic
    // If SelectFade is active (0) shift interrupt-flags by 4 to get fade flags first
    // Set the upper 4 bit of flags to 1 so the lower 4 bits in Joypad can be set without changing the upper 4
    // Set the lower 4 bit of Joypad to 1 (not pressed) so they can be set accordingly
    if (GBC_MMU_Memory.JoypadInputSelectFade == 0)
    {
        uint8_t flags = (((~Input_Interrupt_Flags.allFlags) >> 4) | 0xF0);
        GBC_MMU_Memory.Joypad |= 0x0F;
        GBC_MMU_Memory.Joypad &= flags;
    }

    // If SelectButtons is active dont shift interrupt-flags, button flags are already first
    if (GBC_MMU_Memory.JoypadInputSelectButtons == 0)
    {
        GBC_MMU_Memory.Joypad |= 0x0F;
        GBC_MMU_Memory.Joypad &= ((~Input_Interrupt_Flags.allFlags) | 0xF0);
    }
}

void Input_UpdateLocks(void)
{
    for (int i = 0; i < 8; i ++)
    {
        if (Input_Locks[i].IsLocked)
        {
            // If lock is enabled calculate for how long it has been locked 
            long timeLocked = INPUT_LOCK_TIM->CNT - Input_Locks[i].LockedSince;

            // If timeLocked is negative timer has overflowed -> add max timer value to correct it
            if (timeLocked < 0) timeLocked += INPUT_MAX_LOCK_TIME;

            // Disable the lock if button was released
            if (!(Input_Interrupt_Flags.allFlags & (Input_Pins[Input_Locks[i].ID] >> 1)))
            {
                Input_Locks[i].IsLocked             = false;
                Input_Locks[i].WasUnlockedByTimeout = false;
            }

            // Disable the lock if enough time has passed
            if (timeLocked > Input_Locks[i].LockedFor)
            {
                Input_Locks[i].IsLocked             = false;
                Input_Locks[i].WasUnlockedByTimeout = true;
            }
        }
    }
}

bool Input_IsLocked(Input_Button_ID_t id)
{
    return Input_Locks[id].IsLocked;
}

// Lock button for lockTime in ms (multiply lockTime by two because timer runs with minimal 2kH)
void Input_Lock(Input_Button_ID_t id, time_t lockTime)
{
    Input_Locks[id].IsLocked    = true;
    Input_Locks[id].LockedFor   = 2 * lockTime;
    Input_Locks[id].LockedSince = INPUT_LOCK_TIM->CNT;
}

void Input_LockAll(time_t lockTime)
{
    for (int i = 0; i < 8; i++)
    {
        Input_Lock(i, lockTime);
    }
}

void Input_LockDynamically(Input_Button_ID_t id)
{
    if (Input_Locks[id].WasUnlockedByTimeout && Input_DynamicLockTimes[id] > INPUT_MIN_DYNAMIC_LOCK_TIME)
    {
        Input_DynamicLockTimes[id] -= INPUT_DYNAMIC_LOCK_TIME_STEP;
    }
    else
    {
        Input_DynamicLockTimes[id] = INPUT_MAX_DYNAMIC_LOCK_TIME;
    }

    Input_Lock(id, Input_DynamicLockTimes[id]);
}

void TIM3_IRQHandler(void)
{
    // What about the if?

    int i;
    for ( i = 0; i < 8; i++)
    {
        // If the Input pin is low button/fade is pressed
        Input_CurrState[i] = ((INPUT_PORT_ALL->IDR & Input_Pins[i]) == 0x00) ? INPUT_PRESSED : INPUT_NOT_PRESSED;

        // If the input state didn't change since 1ms ago increase counter
        // If the input state changed since 1ms ago reset counter
        if (Input_CurrState[i] == Input_LastState[i])
        {
            Input_Counter[i]++;
        }
        else
        {
            Input_Counter[i] = 0;
        }

        // If the input state didn't change for 10 cycles (10ms) consider state as permanent
        if (Input_Counter[i] >= 10)
        {
            // Reset the corresponding Input Bit (not pressed) then set it according to the current state
            Input_Interrupt_Flags.allFlags &= ~(Input_Pins[i] >> 1);
            Input_Interrupt_Flags.allFlags |= ((Input_Pins[i] >> 1) & Input_CurrState[i]);

            Input_Counter[i] = 0;
        }

        Input_LastState[i] = Input_CurrState[i];
    }

    __HAL_TIM_CLEAR_IT(&Input_PollTimerHandle, TIM_IT_UPDATE);
}
