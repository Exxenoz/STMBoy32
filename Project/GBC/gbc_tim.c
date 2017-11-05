#include "gbc_tim.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint32_t GBC_TIM_DividerTicks = 0;
uint32_t GBC_TIM_CounterTicks = 0;
uint32_t GBC_TIM_CounterFrequency = 0;

void GBC_TIM_Initialize(void)
{
    GBC_TIM_DividerTicks = 0;
    GBC_TIM_CounterTicks = 0;
    GBC_TIM_CounterFrequency = 0;
}

void GBC_TIM_Step(void)
{
    GBC_TIM_DividerTicks += GBC_CPU_StepTicks;

    // Timer divider register is incremented at rate of 16384Hz
    while (GBC_TIM_DividerTicks >= 256)
    {
        GBC_TIM_DividerTicks -= 256;

        GBC_MMU_Memory.TimerDivider++;
    }

    if (GBC_MMU_Memory.TimerRunning)
    {
        GBC_TIM_CounterTicks += GBC_CPU_StepTicks;

        switch (GBC_MMU_Memory.TimerSpeed)
        {
            case 0:
                GBC_TIM_CounterFrequency = 1024;    // 4096Hz
                break;
            case 1:
                GBC_TIM_CounterFrequency = 16;      // 262144Hz
                break;
            case 2:
                GBC_TIM_CounterFrequency = 64;      // 65536Hz
                break;
            case 3:
                GBC_TIM_CounterFrequency = 256;     // 16384Hz
                break;
        }

        while (GBC_TIM_CounterTicks >= GBC_TIM_CounterFrequency)
        {
            GBC_TIM_CounterTicks -= GBC_TIM_CounterFrequency;

            if (GBC_MMU_Memory.TimerCounter == 0xFF)
            {
                // When TimerCounter overflows the value of TimerModulo will be used as start value
                GBC_MMU_Memory.TimerCounter = GBC_MMU_Memory.TimerModulo;

                GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_TIMER;
            }
            else
            {
                GBC_MMU_Memory.TimerCounter++;
            }
        }
    }
}
