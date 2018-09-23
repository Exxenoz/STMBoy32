#include "input.h"
#include "input_init.h"
#include "input_config.h"
#include "gbc_mmu.h"


Input_Interrupt_Flags_t  Input_Interrupt_Flags = { .allFlags = 0x00 };
uint8_t                  Input_Counter[8]      = {0};

Input_ButtonState_t Input_LastState[8] =
{
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
};

Input_ButtonState_t Input_CurrState[8] =
{
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
    INPUT_NOT_PRESSED,
};

const Input_Pins_t Input_Pins[8] =
{
    { INPUT_A_PORT, INPUT_A_PIN },
    { INPUT_B_PORT, INPUT_B_PIN },
    { INPUT_SELECT_PORT, INPUT_SELECT_PIN },
    { INPUT_START_PORT, INPUT_START_PIN },
    { INPUT_FADE_RIGHT_PORT, INPUT_FADE_RIGHT_PIN },
    { INPUT_FADE_LEFT_PORT, INPUT_FADE_LEFT_PIN },
    { INPUT_FADE_TOP_PORT, INPUT_FADE_TOP_PIN },
    { INPUT_FADE_BOTTOM_PORT, INPUT_FADE_BOTTOM_PIN },
};



void Input_UpdateGBCJoypad(void)
{
    // Interrupt-flags have to be inverted because GBGames have inverted logic
    // If SelectFade is active (0) shift interrupt-flags by 4 to get fade flags first
    // Set the upper 4 bit of flags to 1 so the lower 4 bits in Joypad can be set without changing the upper 4
    // Set the lower 4 bit of Joypad to 1 (not pressed) so they can be set accordingly
    if (GBC_MMU_Memory.IO.JoypadInputSelectFade == 0)
    {
        uint8_t flags = (((~Input_Interrupt_Flags.allFlags) >> 4) | 0xF0);
        GBC_MMU_Memory.IO.Joypad |= 0x0F;
        GBC_MMU_Memory.IO.Joypad &= flags;
    }

    // If SelectButtons is active dont shift interrupt-flags, button flags are already first
    if (GBC_MMU_Memory.IO.JoypadInputSelectButtons == 0)
    {
        GBC_MMU_Memory.IO.Joypad |= 0x0F;
        GBC_MMU_Memory.IO.Joypad &= ((~Input_Interrupt_Flags.allFlags) | 0xF0);
    }
}

void TIM2_IRQHandler(void)
{
    for (int i = 0; i < 8; i++)
    {
        // If the Input pin is low button/fade is pressed.
        Input_CurrState[i] = ((Input_Pins[i].Port->IDR & Input_Pins[i].Pin) == 0x00) ? INPUT_PRESSED : INPUT_NOT_PRESSED;

        // If the input state didn't change since 1ms ago increase counter.
        // If the input state changed since 1ms ago reset counter.
        if (Input_CurrState[i] == Input_LastState[i])
        {
            Input_Counter[i]++;
        }
        else
        {
            Input_Counter[i] = 0;
        }

        // If the input state didn't change for x cycles (x ms) consider state as permanent.
        if (Input_Counter[i] >= INPUT_POLLING_CYCLES_UNTIL_CONSIDERED_PRESSED)
        {
            // Reset the corresponding Input Bit (not pressed) then set it according to the current state.
            Input_Interrupt_Flags.allFlags &= ~(Input_Pins[i].Pin >> 1);
            Input_Interrupt_Flags.allFlags |= ((Input_Pins[i].Pin >> 1) & Input_CurrState[i]);

            Input_Counter[i] = 0;
        }

        Input_LastState[i] = Input_CurrState[i];
    }

    __HAL_TIM_CLEAR_IT(&Input_PollTimerHandle, TIM_IT_UPDATE);
}
