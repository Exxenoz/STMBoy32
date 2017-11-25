#include "input.h"
#include "lcd.h"
#include "gbc_mmu.h"

Input_Interrupt_Flags_t Input_Interrupt_Flags;

Input_ButtonState_t Input_LastState[8];
Input_ButtonState_t Input_CurrState[8];
uint8_t             Input_Counter[8];

Input_Lock_t        Input_Locks[8];

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
    RCC_AHB1PeriphClockCmd(INPUT_BUS_ALL, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;

    
    #define INITIALIZE_GPIO_PIN(PORT, PIN)                       \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_IN;                   \
    GPIO_InitObject.GPIO_Pin   = PIN;                            \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_UP;                   \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;              \
    GPIO_Init(PORT, &GPIO_InitObject);                           \

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
    RCC_APB1PeriphClockCmd(INPUT_POLLING_TIM_BUS, ENABLE);
    RCC_APB1PeriphClockCmd(INPUT_LOCK_TIM_BUS, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_BaseObject;
    NVIC_InitTypeDef        NVIC_InitObject;


    // Initialize lock timer
    TIM_BaseObject.TIM_Prescaler            = 44999;               // Tim4 runs with 90MHz -> scale it to 2kHz
    TIM_BaseObject.TIM_CounterMode          = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period               = INPUT_MAX_LOCK_TIME; // Count 'til max even value -> min overflows/s
    TIM_BaseObject.TIM_ClockDivision        = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter    = 0;
    TIM_TimeBaseInit(INPUT_LOCK_TIM, &TIM_BaseObject);

    TIM_Cmd(INPUT_LOCK_TIM, ENABLE);

    // Initialise polling timer
    TIM_BaseObject.TIM_Prescaler            = 5999;                // Tim3 runs with 90MHz -> scale it to 15kHz
    TIM_BaseObject.TIM_CounterMode          = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period               = 14;                  // Count 'til 14 (+1) -> 1000 overflows/s
    TIM_BaseObject.TIM_ClockDivision        = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter    = 0;
    TIM_TimeBaseInit(INPUT_POLLING_TIM, &TIM_BaseObject);

    TIM_ITConfig(INPUT_POLLING_TIM, TIM_IT_Update, ENABLE);
    TIM_Cmd(INPUT_POLLING_TIM, ENABLE);

    NVIC_InitObject.NVIC_IRQChannel                   = INPUT_POLLING_TIM_NVIC_CHANNEL;
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitObject);
    NVIC_EnableIRQ(INPUT_POLLING_TIM_NVIC_CHANNEL);
    
    NVIC_SetPriority(INPUT_POLLING_TIM_NVIC_CHANNEL, INTERRUPT_PRIORITY_2);
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

        Input_Locks[i].ID        = i;
        Input_Locks[i].IsEnabled = false;
    }

    //-----------DEBUG LED----------
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin   = GPIO_Pin_14;
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitObject);
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
        if (Input_Locks[i].IsEnabled)
        {
            // If lock is enabled calculate for how long it has been locked 
            long timeLocked = INPUT_LOCK_TIM->CNT - Input_Locks[i].LockedSince;

            // If timeLocked is negative timer has overflowed -> add max timer value to correct it
            if (timeLocked < 0) timeLocked += INPUT_MAX_LOCK_TIME;

            // Disable the lock if button was released or enough time has passed
            if (!(Input_Interrupt_Flags.allFlags & (Input_Pins[Input_Locks[i].ID] >> 1)) || timeLocked > Input_Locks[i].LockedFor)
            {
                Input_Locks[i].IsEnabled = false;
            }
        }
    }
}

bool Input_IsLocked(Input_Button_ID_t id)
{
    return Input_Locks[id].IsEnabled;
}

// Lock button for lockTime in ms (multiply lockTime by two because timer runs with minimal 2kH)
void Input_Lock(Input_Button_ID_t id, time_t lockTime)
{
    Input_Locks[id].IsEnabled   = true;
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

void IRQ_HANDLER(INPUT_POLLING_TIM)(void)
{
    // What about the if?

    int i;
    for ( i = 0; i < 8; i++)
    {
        // If the Input pin is low button/fade is pressed
        Input_CurrState[i] = ((INPUT_PORT_ALL->IDR & Input_Pins[i]) == 0x00) ? INPUT_PRESSED : INPUT_NOT_PRESSED;

        // If the input state didn't change since 1ms ago increase counter
        if (Input_CurrState[i] == Input_LastState[i])
        {
            Input_Counter[i]++;
        }
        // If the input state changed since 1ms ago reset counter
        else
        {
            Input_Counter[i] = 0;
        }

        // If the input state didn't change since 10ms ago consider state as permanent
        if (Input_Counter[i] >= 10)
        {
            // Reset the corresponding Input Bit (not pressed) then set it according to the current state
            Input_Interrupt_Flags.allFlags &= ~(Input_Pins[i] >> 1);
            Input_Interrupt_Flags.allFlags |= ((Input_Pins[i] >> 1) & Input_CurrState[i]);

            Input_Counter[i] = 0;
        }

        Input_LastState[i] = Input_CurrState[i];
    }

    TIM_ClearITPendingBit(INPUT_POLLING_TIM, TIM_IT_Update);
}
