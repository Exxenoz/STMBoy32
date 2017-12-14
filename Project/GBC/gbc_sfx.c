#include "gbc_sfx.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint32_t GBC_SFX_Ticks = 0;
uint8_t GBC_SFX_Buffer[GBC_SFX_BUFFER_SIZE];

uint32_t GBC_SFX_Channel1Ticks = 0;
uint32_t GBC_SFX_Channel1Length = 0;
uint32_t GBC_SFX_Channel1Position = 0;
uint32_t GBC_SFX_Channel1Frequency = 0;
uint32_t GBC_SFX_Channel1EnvelopeTicks = 0;
uint32_t GBC_SFX_Channel1EnvelopeLength = 0;
uint32_t GBC_SFX_Channel1EnvelopeVolume = 0;
uint32_t GBC_SFX_Channel1SweepTicks = 0;
uint32_t GBC_SFX_Channel1SweepLength = 0;

uint32_t GBC_SFX_Channel2Ticks = 0;
uint32_t GBC_SFX_Channel2Length = 0;
uint32_t GBC_SFX_Channel2Position = 0;
uint32_t GBC_SFX_Channel2Frequency = 0;
uint32_t GBC_SFX_Channel2EnvelopeTicks = 0;
uint32_t GBC_SFX_Channel2EnvelopeLength = 0;
uint32_t GBC_SFX_Channel2EnvelopeVolume = 0;

static const int8_t GBC_SFX_SQUARE_WAVE[4][8] =
{
	{  0, 0,-1, 0, 0, 0, 0, 0 },
	{  0,-1,-1, 0, 0, 0, 0, 0 },
	{ -1,-1,-1,-1, 0, 0, 0, 0 },
	{ -1, 0, 0,-1,-1,-1,-1,-1 }
};

void GBC_SFX_Initialize(void)
{
    GBC_SFX_Ticks = 0;

    GBC_SFX_Channel1Ticks = 0;
    GBC_SFX_Channel1Length = 0;
    GBC_SFX_Channel1Position = 0;
    GBC_SFX_Channel1Frequency = 0;
    GBC_SFX_Channel1EnvelopeTicks = 0;
    GBC_SFX_Channel1EnvelopeLength = 0;
    GBC_SFX_Channel1EnvelopeVolume = 0;
    GBC_SFX_Channel1SweepTicks = 0;
    GBC_SFX_Channel1SweepLength = 0;

    GBC_SFX_Channel2Ticks = 0;
    GBC_SFX_Channel2Length = 0;
    GBC_SFX_Channel2Position = 0;
    GBC_SFX_Channel2Frequency = 0;
    GBC_SFX_Channel2EnvelopeTicks = 0;
    GBC_SFX_Channel2EnvelopeLength = 0;
    GBC_SFX_Channel2EnvelopeVolume = 0;
}

void GBC_SFX_Step(void)
{
    if (!GBC_MMU_Memory.ChannelSoundsEnabled)
    {
        return;
    }

    long f = 0;
    long l = 0;
    long r = 0;
    long s = 0;

    GBC_SFX_Ticks += GBC_CPU_StepTicks;

    for (; GBC_SFX_Ticks >= GBC_SFX_SAMPLE_RATE; GBC_SFX_Ticks -= GBC_SFX_SAMPLE_RATE)
    {
        if (GBC_MMU_Memory.ChannelSound1Enabled)
        {
            // Stop output when length expires
            if (GBC_MMU_Memory.Channel1CounterSelection)
            {
                GBC_SFX_Channel1Ticks += GBC_SFX_SAMPLE_RATE;

                if (GBC_SFX_Channel1Ticks >= GBC_SFX_Channel1Length)
                {
                    GBC_MMU_Memory.ChannelSound1Enabled = 0;
                }
            }

            // Envelope handling
            if (GBC_SFX_Channel1EnvelopeLength)
            {
                GBC_SFX_Channel1EnvelopeTicks += GBC_SFX_SAMPLE_RATE;

                if (GBC_SFX_Channel1EnvelopeTicks >= GBC_SFX_Channel1EnvelopeLength)
                {
                    GBC_SFX_Channel1EnvelopeTicks -= GBC_SFX_Channel1EnvelopeLength;

                    // Envelope Direction (0 = Decrease, 1 = Increase)
                    if (GBC_MMU_Memory.Channel1EnvelopeDirection)
                    {
                        if (GBC_SFX_Channel1EnvelopeVolume < 15)
                        {
                            GBC_SFX_Channel1EnvelopeVolume++;
                        }
                    }
                    else
                    {
                        if (GBC_SFX_Channel1EnvelopeVolume > 0)
                        {
                            GBC_SFX_Channel1EnvelopeVolume--;
                        }
                    }
                }
            }

            // Sweep handling
            if (GBC_SFX_Channel1SweepLength)
            {
                GBC_SFX_Channel1SweepTicks += GBC_SFX_SAMPLE_RATE;

                if (GBC_SFX_Channel1SweepTicks >= GBC_SFX_Channel1SweepLength)
                {
                    GBC_SFX_Channel1SweepTicks -= GBC_SFX_Channel1SweepLength;

                    f = GBC_MMU_Memory.Channel1Frequency;

                    // Sweep Increase / Decrease
                    // 0: Addition    (frequency increases)
                    // 1: Subtraction (frequency decreases)
                    if (GBC_MMU_Memory.Channel1SweepType)
                    {
                        f -= f >> GBC_MMU_Memory.Channel1SweepShift;
                    }
                    else
                    {
                        f += f >> GBC_MMU_Memory.Channel1SweepShift;
                    }

                    if (f > 2047)
                    {
                        GBC_MMU_Memory.ChannelSound1Enabled = 0;
                    }
                    else
                    {
                        GBC_MMU_Memory.Channel1Frequency = f;

                        f = 2048 - f;

                        if ((f << 4) < GBC_SFX_SAMPLE_RATE)
                        {
                            GBC_SFX_Channel1Frequency = 0;
                        }
                        else
                        {
                            GBC_SFX_Channel1Frequency = (GBC_SFX_SAMPLE_RATE << 17) / f;
                        }
                    }
                }
            }

            s = GBC_SFX_SQUARE_WAVE[GBC_MMU_Memory.Channel1WavePatternDuty][(GBC_SFX_Channel1Position >> 18) & 7] & GBC_SFX_Channel1EnvelopeVolume;
            s <<= 2;

            GBC_SFX_Channel1Position += GBC_SFX_Channel1Frequency;

            if (GBC_MMU_Memory.SoundOutputChannel1ToSO1)
            {
                r += s;
            }

            if (GBC_MMU_Memory.SoundOutputChannel1ToSO2)
            {
                l += s;
            }
        }

        if (GBC_MMU_Memory.ChannelSound2Enabled)
        {
            // Stop output when length expires
            if (GBC_MMU_Memory.Channel2CounterSelection)
            {
                GBC_SFX_Channel2Ticks += GBC_SFX_SAMPLE_RATE;

                if (GBC_SFX_Channel2Ticks >= GBC_SFX_Channel2Length)
                {
                    GBC_MMU_Memory.ChannelSound2Enabled = 0;
                }
            }

            // Envelope handling
            if (GBC_SFX_Channel2EnvelopeLength)
            {
                GBC_SFX_Channel2EnvelopeTicks += GBC_SFX_SAMPLE_RATE;

                if (GBC_SFX_Channel2EnvelopeTicks >= GBC_SFX_Channel2EnvelopeLength)
                {
                    GBC_SFX_Channel2EnvelopeTicks -= GBC_SFX_Channel2EnvelopeLength;

                    // Envelope Direction (0 = Decrease, 1 = Increase)
                    if (GBC_MMU_Memory.Channel2EnvelopeDirection)
                    {
                        if (GBC_SFX_Channel2EnvelopeVolume < 15)
                        {
                            GBC_SFX_Channel2EnvelopeVolume++;
                        }
                    }
                    else
                    {
                        if (GBC_SFX_Channel2EnvelopeVolume > 0)
                        {
                            GBC_SFX_Channel2EnvelopeVolume--;
                        }
                    }
                }
            }

            s = GBC_SFX_SQUARE_WAVE[GBC_MMU_Memory.Channel2WavePatternDuty][(GBC_SFX_Channel2Position >> 18) & 7] & GBC_SFX_Channel2EnvelopeVolume;
            s <<= 2;

            GBC_SFX_Channel2Position += GBC_SFX_Channel2Frequency;

            if (GBC_MMU_Memory.SoundOutputChannel2ToSO1)
            {
                r += s;
            }

            if (GBC_MMU_Memory.SoundOutputChannel2ToSO2)
            {
                l += s;
            }
        }

        if (GBC_MMU_Memory.ChannelSound3Enabled)
        {
            
        }

        if (GBC_MMU_Memory.ChannelSound4Enabled)
        {
            
        }
    }
}
