#include "input.h"
#include "lcd.h"
#include "gbc_mmu.h"

uint16_t INPUT_INTERRUPT_FLAGS = 0xFF;

INPUT_ButtonState_t lastState[8]    = {INPUT_NOT_PRESSED};
INPUT_ButtonState_t currentState[8] = {INPUT_NOT_PRESSED};

uint8_t counter = 0x00;

INPUT_InterruptFlags_t flagPositions[8] =
{
    INPUT_INTERRUPT_FLAG_A,
    INPUT_INTERRUPT_FLAG_B,
    INPUT_INTERRUPT_FLAG_SELECT,
    INPUT_INTERRUPT_FLAG_START,
    INPUT_INTERRUPT_FLAG_FADE_RIGHT,
    INPUT_INTERRUPT_FLAG_FADE_LEFT,
    INPUT_INTERRUPT_FLAG_FADE_TOP,
    INPUT_INTERRUPT_FLAG_FADE_BOT,
};


void Input_InitializePins(void)
{
    RCC_AHB1PeriphClockCmd(INPUT_BUS_ALL, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;

    
    #define INITIALIZE_GPIO_PIN(PORT, PIN)                       \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_IN;                   \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;                  \
    GPIO_InitObject.GPIO_Pin   = PIN;                            \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_DOWN;                 \
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

void Input_InitializeTimer(void)
{
    RCC_APB1PeriphClockCmd(INPUT_TIM_BUS, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_BaseObject;
    NVIC_InitTypeDef        NVIC_InitObject;


    TIM_BaseObject.TIM_Prescaler            = 5999;                // Tim3 runs with 90MHz -> scale it to 15kHz
    TIM_BaseObject.TIM_CounterMode          = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period               = 14;                  // Count 'til 14 (starting at 0) -> 1000 overflows/s
    TIM_BaseObject.TIM_ClockDivision        = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter    = 0;
    TIM_TimeBaseInit(INPUT_TIM, &TIM_BaseObject);

    TIM_ClearITPendingBit(INPUT_TIM, TIM_IT_Update);
    TIM_Cmd(INPUT_TIM, ENABLE);

    NVIC_InitObject.NVIC_IRQChannel                   = INPUT_TIM_NVIC_CHANNEL;
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitObject);
    NVIC_EnableIRQ(INPUT_TIM_NVIC_CHANNEL);
}

void Input_Initialize() 
{
    Input_InitializePins();
    Input_InitializeTimer();

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

void Input_HandleButtonState(void)
{
    //-----------DEBUG LED----------
    if (INPUT_INTERRUPT_FLAGS != 0xFF) GPIO_ToggleBits(GPIOB, GPIO_Pin_14);
    //------------------------------
    
    if (GBC_MMU_Memory.JoypadInputSelectButtons)
    {
        GBC_MMU_Memory.Joypad |= 0x0F;                                  // Set lower 4 bits of Joypad to 1 (not pressed)
        GBC_MMU_Memory.Joypad &= (INPUT_INTERRUPT_FLAGS >> 1) | 0xF0;   // Set lower 4 bits of Joypad to button states
    }
    
    if (GBC_MMU_Memory.JoypadInputSelectFade)
    {
        GBC_MMU_Memory.Joypad |= 0x0F;                                  // Set lower 4 bits of Joypad to 1 (not pressed)
        GBC_MMU_Memory.Joypad &= (INPUT_INTERRUPT_FLAGS >> 5) | 0xF0;   // Set lower 4 bits of Joypad to fade states
    }
}

void TIM3_IRQHandler(void)
{       
    for (int i = 0; i < 8; i++)
    {
        // If the Input pin is low button/fade is pressed
        currentState[i] = ((INPUT_PORT_ALL->IDR & flagPositions[i]) == 0x00) ? INPUT_PRESSED : INPUT_NOT_PRESSED;
        
        // If the input state didn't change since 1ms ago increase counter
        if (currentState[i] == lastState[i])
        {
            counter++;
        }
        // If the input state changed since 1ms ago reset counter
        else                                
        {
            counter = 0;
        }

        // If the input state didn't change since 5ms ago consider state as permanent
        if (counter >= 5)
        {
            // Set the corresponding Input Bit (not pressed) then set it according to the current state
            INPUT_INTERRUPT_FLAGS |= flagPositions[i];
            INPUT_INTERRUPT_FLAGS &= (~flagPositions[i] | currentState[i]);
        }
        
        lastState[i] = currentState[i];
    }
}
