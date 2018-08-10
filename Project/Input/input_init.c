#include "input_init.h"


TIM_HandleTypeDef Input_LockTimerHandle = { 
    .Instance = INPUT_LOCK_TIM
};

TIM_HandleTypeDef Input_PollTimerHandle = { 
    .Instance = INPUT_POLLING_TIM
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
    // Initialize lock timer.
    Input_LockTimerHandle.Init.Prescaler         = INPUT_LOCK_TIM_PRESCALER;
    Input_LockTimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Input_LockTimerHandle.Init.Period            = MAX_16BIT_TIMER_PERIOD;    // Count 'til max value -> 1 overflow / 16s (with 49999 as prescaler).
    Input_LockTimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    Input_LockTimerHandle.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&Input_LockTimerHandle);
    HAL_TIM_Base_Start(&Input_LockTimerHandle);

    // Initialise polling timer.
    Input_PollTimerHandle.Init.Prescaler         = INPUT_POLLING_TIM_PRESCALER;
    Input_PollTimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Input_PollTimerHandle.Init.Period            = ((INPUT_POLLING_TIM_FREQ / (INPUT_POLLING_TIM_PRESCALER + 1)) / 1000) - 1; // Generate 1000 overflows/s.
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
}
