#include "gbc_apu.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"
#include "audio.h"
#include <string.h>

uint32_t GBC_APU_FrameTicks = 0;
uint32_t GBC_APU_FramePeriod = 4194304 / 512; // 512 Hz
uint32_t GBC_APU_FrameIndex = 0;

uint32_t GBC_APU_Ticks = 0;

uint8_t GBC_APU_Buffer_L[GBC_APU_BUFFER_SIZE];
uint8_t GBC_APU_Buffer_R[GBC_APU_BUFFER_SIZE];

uint32_t GBC_APU_BufferPosition = GBC_APU_BUFFER_SIZE / 2; // Start filling second buffer, so we are always ahead for filling.

uint32_t GBC_APU_Channel1Phase = 0;
int32_t  GBC_APU_Channel1PhaseTicks = 0;
uint32_t GBC_APU_Channel1PhaseFrequency = 0;
uint32_t GBC_APU_Channel1PhasePeriod = 0;
uint32_t GBC_APU_Channel1LastSample = 0;
uint32_t GBC_APU_Channel1LengthCounter = 0;
bool     GBC_APU_Channel1SweepEnabled = false;
uint32_t GBC_APU_Channel1SweepFrequency = 0;
bool     GBC_APU_Channel1SweepDecrease = false;
int32_t  GBC_APU_Channel1SweepLengthCounter = 0;
bool     GBC_APU_Channel1EnvelopeEnabled = false;
uint32_t GBC_APU_Channel1EnvelopeVolume = 0;
int32_t  GBC_APU_Channel1EnvelopeLengthCounter = 0;

uint32_t GBC_APU_Channel2Phase = 0;
int32_t  GBC_APU_Channel2PhaseTicks = 0;
uint32_t GBC_APU_Channel2PhaseFrequency = 0;
uint32_t GBC_APU_Channel2PhasePeriod = 0;
uint32_t GBC_APU_Channel2LastSample = 0;
uint32_t GBC_APU_Channel2LengthCounter = 0;
bool     GBC_APU_Channel2EnvelopeEnabled = false;
uint32_t GBC_APU_Channel2EnvelopeVolume = 0;
int32_t  GBC_APU_Channel2EnvelopeLengthCounter = 0;

uint32_t GBC_APU_Channel3Phase = 0;
int32_t  GBC_APU_Channel3PhaseTicks = 0;
uint32_t GBC_APU_Channel3PhaseFrequency = 0;
int32_t  GBC_APU_Channel3PhasePeriod = 0;
uint32_t GBC_APU_Channel3LastSample = 0;
uint32_t GBC_APU_Channel3LengthCounter = 0;

int32_t  GBC_APU_Channel4Ticks = 0;
uint32_t GBC_APU_Channel4Frequency = 0;
int32_t  GBC_APU_Channel4Period = 0;
uint16_t GBC_APU_Channel4LFSR = 0;
uint32_t GBC_APU_Channel4LastSample = 0;
uint32_t GBC_APU_Channel4LengthCounter = 0;
bool     GBC_APU_Channel4EnvelopeEnabled = false;
uint32_t GBC_APU_Channel4EnvelopeVolume = 0;
int32_t  GBC_APU_Channel4EnvelopeLengthCounter = 0;

static const uint32_t GBC_APU_BaseNoiseFrequencies[8] =
{
    524288 * 2,
    524288,
    524288 / 2,
    524288 / 3,
    524288 / 4,
    524288 / 5,
    524288 / 6,
    524288 / 7
};

#define IS_CHANNEL1_DAC_ENABLED() (GBC_MMU_Memory.IO.Channel1VolumeEnvelope & 0xF8)
#define IS_CHANNEL2_DAC_ENABLED() (GBC_MMU_Memory.IO.Channel2VolumeEnvelope & 0xF8)
#define IS_CHANNEL3_DAC_ENABLED() (GBC_MMU_Memory.IO.Channel3PlaybackEnabled)
#define IS_CHANNEL4_DAC_ENABLED() (GBC_MMU_Memory.IO.Channel4VolumeEnvelope & 0xF8)

#define IS_CHANNEL1_SWEEP_ENABLED() ((GBC_MMU_Memory.IO.Channel1Sweep & 0x77) != 0)

#define IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() (GBC_APU_FrameIndex & 1)
#define IS_APU_IN_SECOND_HALF_OF_LENGTH_PERIOD() (!(GBC_APU_FrameIndex & 1))

#define GET_CHANNEL1_FREQUENCY() ((GBC_MMU_Memory.IO.Channel1FrequencyHI << 8) + GBC_MMU_Memory.IO.Channel1FrequencyLO)
#define GET_CHANNEL2_FREQUENCY() ((GBC_MMU_Memory.IO.Channel2FrequencyHI << 8) + GBC_MMU_Memory.IO.Channel2FrequencyLO)
#define GET_CHANNEL3_FREQUENCY() ((GBC_MMU_Memory.IO.Channel3FrequencyHI << 8) + GBC_MMU_Memory.IO.Channel3FrequencyLO)

#define CALCULATE_CHANNEL1_SWEEP(UPDATE_SWEEP_FREQUENCY)                                                                                                \
{                                                                                                                                                       \
    long sweepFrequency = GBC_APU_Channel1SweepFrequency;                                                                                               \
    GBC_APU_Channel1SweepDecrease = GBC_MMU_Memory.IO.Channel1SweepType ? true : false;                                                                 \
                                                                                                                                                        \
    if (GBC_APU_Channel1SweepDecrease)                                                                                                                  \
    {                                                                                                                                                   \
        sweepFrequency -= GBC_APU_Channel1SweepFrequency >> GBC_MMU_Memory.IO.Channel1SweepShift;                                                       \
    }                                                                                                                                                   \
    else                                                                                                                                                \
    {                                                                                                                                                   \
        sweepFrequency += GBC_APU_Channel1SweepFrequency >> GBC_MMU_Memory.IO.Channel1SweepShift;                                                       \
    }                                                                                                                                                   \
                                                                                                                                                        \
    if (sweepFrequency > 2047)                                                                                                                          \
    {                                                                                                                                                   \
        GBC_MMU_Memory.IO.ChannelSound1Enabled = 0;                                                                                                     \
    }                                                                                                                                                   \
    else if (GBC_MMU_Memory.IO.Channel1SweepShift && UPDATE_SWEEP_FREQUENCY)                                                                            \
    {                                                                                                                                                   \
        GBC_APU_Channel1SweepFrequency = sweepFrequency;                                                                                                \
                                                                                                                                                        \
        GBC_MMU_Memory.IO.Channel1FrequencyLO = sweepFrequency & 0xFF;                                                                                  \
        GBC_MMU_Memory.IO.Channel1FrequencyHI = sweepFrequency >> 8;                                                                                    \
    }                                                                                                                                                   \
}                                                                                                                                                       \

void GBC_APU_InitializeChannel1(void)
{
    GBC_APU_Channel1Phase = 0;
    GBC_APU_Channel1PhaseTicks = 0;
    GBC_APU_Channel1PhaseFrequency = GET_CHANNEL1_FREQUENCY();
    GBC_APU_Channel1PhasePeriod = (2048 - GBC_APU_Channel1PhaseFrequency) * 4;
    GBC_APU_Channel1LastSample = 0;
    GBC_APU_Channel1LengthCounter = 64 - GBC_MMU_Memory.IO.Channel1SoundLengthData;
    GBC_APU_Channel1SweepEnabled = false;
    GBC_APU_Channel1SweepFrequency = 0;
    GBC_APU_Channel1SweepDecrease = false;
    GBC_APU_Channel1SweepLengthCounter = GBC_MMU_Memory.IO.Channel1SweepTime;
    GBC_APU_Channel1EnvelopeEnabled = false;
    GBC_APU_Channel1EnvelopeVolume = GBC_MMU_Memory.IO.Channel1InitialEnvelopeVolume;
    GBC_APU_Channel1EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel1EnvelopeSweepNumber;
}

void GBC_APU_InitializeChannel2(void)
{
    GBC_APU_Channel2Phase = 0;
    GBC_APU_Channel2PhaseTicks = 0;
    GBC_APU_Channel2PhaseFrequency = GET_CHANNEL2_FREQUENCY();
    GBC_APU_Channel2PhasePeriod = (2048 - GBC_APU_Channel2PhaseFrequency) * 4;
    GBC_APU_Channel2LastSample = 0;
    GBC_APU_Channel2LengthCounter = 64 - GBC_MMU_Memory.IO.Channel2SoundLengthData;
    GBC_APU_Channel2EnvelopeEnabled = false;
    GBC_APU_Channel2EnvelopeVolume = GBC_MMU_Memory.IO.Channel2InitialEnvelopeVolume;
    GBC_APU_Channel2EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel2EnvelopeSweepNumber;
}

void GBC_APU_InitializeChannel3(void)
{
    GBC_APU_Channel3Phase = 0;
    GBC_APU_Channel3PhaseTicks = 0;
    GBC_APU_Channel3PhaseFrequency = GET_CHANNEL3_FREQUENCY();
    GBC_APU_Channel3PhasePeriod = (2048 - GBC_APU_Channel3PhaseFrequency) * 2; // (4194304 / (4194304 / (64 * (2048 - f)))) / 32
    GBC_APU_Channel3LastSample = 0;
    GBC_APU_Channel3LengthCounter = 256 - GBC_MMU_Memory.IO.Channel3SoundLength;
}

void GBC_APU_InitializeChannel4(void)
{
    GBC_APU_Channel4Ticks = 0;
    GBC_APU_Channel4Frequency = GBC_APU_BaseNoiseFrequencies[GBC_MMU_Memory.IO.Channel4PolynomialCounterFreqDivRatio] >> (GBC_MMU_Memory.IO.Channel4PolynomialCounterShiftClockFreq + 1);
    GBC_APU_Channel4Period = 4194304 / GBC_APU_Channel4Frequency;
    GBC_APU_Channel4LFSR = 0x7FFF;
    GBC_APU_Channel4LastSample = 0;
    GBC_APU_Channel4LengthCounter = 64 - (GBC_MMU_Memory.IO.Channel4SoundLengthData & 63);
    GBC_APU_Channel4EnvelopeEnabled = false;
    GBC_APU_Channel4EnvelopeVolume = GBC_MMU_Memory.IO.Channel4InitialEnvelopeVolume;
    GBC_APU_Channel4EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel4EnvelopeSweepNumber;
}

void GBC_APU_InitializeChannels(void)
{
    GBC_APU_InitializeChannel1();
    GBC_APU_InitializeChannel2();
    GBC_APU_InitializeChannel3();
    GBC_APU_InitializeChannel4();
}

void GBC_APU_Initialize(void)
{
    GBC_APU_FrameTicks = 0;
    GBC_APU_FramePeriod = 4194304 / 512; // 512 Hz
    GBC_APU_FrameIndex = 0;

    GBC_APU_Ticks = 0;

    memset(&GBC_APU_Buffer_L, 0, sizeof(GBC_APU_Buffer_L));
    memset(&GBC_APU_Buffer_R, 0, sizeof(GBC_APU_Buffer_R));

    GBC_APU_BufferPosition = GBC_APU_BUFFER_SIZE / 2;

    GBC_APU_InitializeChannels();

    Audio_StopOutput();
    Audio_SetOutputBuffer(AUDIO_OUTPUTMODE_GBC, AUDIO_SAMPLE_ALIGNMENT_8B_R,
        GBC_APU_Buffer_L, GBC_APU_Buffer_R, GBC_APU_BUFFER_SIZE);
    Audio_StartOutput(AUDIO_PLAYBACK_HALF1, true);
}

void GBC_APU_Step(void)
{
    if (!GBC_MMU_Memory.IO.ChannelSoundsEnabled)
    {
        return;
    }

    static const uint8_t SquareDutyOffsets[4] = { 1, 1, 3, 7 };
    static const uint8_t SquareDuties[4] = { 1, 2, 4, 6 };
    static const uint8_t WaveVolumeMultipliers[4] = { 0, 4, 2, 1 };

    long l = 0; // Left speaker
    long r = 0; // Right speaker

    GBC_APU_FrameTicks += GBC_CPU_StepTicks;

    if (GBC_APU_FrameTicks >= GBC_APU_FramePeriod)
    {
        GBC_APU_FrameTicks -= GBC_APU_FramePeriod;

        switch (GBC_APU_FrameIndex++)
        {
            case 2:
            case 6: // 128 Hz
            {
                // Channel 1 sweep handling
                if (--GBC_APU_Channel1SweepLengthCounter <= 0)
                {
                    GBC_APU_Channel1SweepLengthCounter = GBC_MMU_Memory.IO.Channel1SweepTime;

                    // Sweep counter treats period 0 as 8
                    if (GBC_APU_Channel1SweepLengthCounter == 0)
                    {
                        GBC_APU_Channel1SweepLengthCounter = 8;
                    }

                    if (GBC_APU_Channel1SweepEnabled &&
                        GBC_MMU_Memory.IO.Channel1SweepTime)
                    {
                        CALCULATE_CHANNEL1_SWEEP(true);
                        CALCULATE_CHANNEL1_SWEEP(false);
                    }
                }

                // NO break!
            }
            case 0:
            case 4: // 256 Hz
            {
                // Stop channel 1 output when length expires
                if (GBC_MMU_Memory.IO.Channel1CounterSelection && GBC_APU_Channel1LengthCounter)
                {
                    if (--GBC_APU_Channel1LengthCounter == 0)
                    {
                        GBC_MMU_Memory.IO.ChannelSound1Enabled = 0;
                    }
                }

                // Stop channel 2 output when length expires
                if (GBC_MMU_Memory.IO.Channel2CounterSelection && GBC_APU_Channel2LengthCounter)
                {
                    if (--GBC_APU_Channel2LengthCounter == 0)
                    {
                        GBC_MMU_Memory.IO.ChannelSound2Enabled = 0;
                    }
                }

                // Stop channel 3 output when length expires
                if (GBC_MMU_Memory.IO.Channel3CounterSelection && GBC_APU_Channel3LengthCounter)
                {
                    if (--GBC_APU_Channel3LengthCounter == 0)
                    {
                        GBC_MMU_Memory.IO.ChannelSound3Enabled = 0;
                    }
                }

                // Stop channel 4 output when length expires
                if (GBC_MMU_Memory.IO.Channel4CounterSelection && GBC_APU_Channel4LengthCounter)
                {
                    if (--GBC_APU_Channel4LengthCounter == 0)
                    {
                        GBC_MMU_Memory.IO.ChannelSound4Enabled = 0;
                    }
                }

                break;
            }
            case 7: // 64 Hz
            {
                GBC_APU_FrameIndex = 0;

                // Channel 1 envelope handling
                if (GBC_APU_Channel1EnvelopeEnabled && --GBC_APU_Channel1EnvelopeLengthCounter <= 0)
                {
                    GBC_APU_Channel1EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel1EnvelopeSweepNumber;

                    if (GBC_APU_Channel1EnvelopeLengthCounter == 0)
                    {
                        GBC_APU_Channel1EnvelopeLengthCounter = 8;
                    }
                    else
                    {
                        // Envelope Direction (0 = Decrease, 1 = Increase)
                        if (GBC_MMU_Memory.IO.Channel1EnvelopeDirection)
                        {
                            if (GBC_APU_Channel1EnvelopeVolume < 15)
                            {
                                GBC_APU_Channel1EnvelopeVolume++;
                            }
                            else
                            {
                                GBC_APU_Channel1EnvelopeEnabled = false;
                            }
                        }
                        else
                        {
                            if (GBC_APU_Channel1EnvelopeVolume > 0)
                            {
                                GBC_APU_Channel1EnvelopeVolume--;
                            }
                            else
                            {
                                GBC_APU_Channel1EnvelopeEnabled = false;
                            }
                        }
                    }
                }

                // Channel 2 envelope handling
                if (GBC_APU_Channel2EnvelopeEnabled && --GBC_APU_Channel2EnvelopeLengthCounter <= 0)
                {
                    GBC_APU_Channel2EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel2EnvelopeSweepNumber;

                    if (GBC_APU_Channel2EnvelopeLengthCounter == 0)
                    {
                        GBC_APU_Channel2EnvelopeLengthCounter = 8;
                    }
                    else
                    {
                        // Envelope Direction (0 = Decrease, 1 = Increase)
                        if (GBC_MMU_Memory.IO.Channel2EnvelopeDirection)
                        {
                            if (GBC_APU_Channel2EnvelopeVolume < 15)
                            {
                                GBC_APU_Channel2EnvelopeVolume++;
                            }
                            else
                            {
                                GBC_APU_Channel2EnvelopeEnabled = false;
                            }
                        }
                        else
                        {
                            if (GBC_APU_Channel2EnvelopeVolume > 0)
                            {
                                GBC_APU_Channel2EnvelopeVolume--;
                            }
                            else
                            {
                                GBC_APU_Channel2EnvelopeEnabled = false;
                            }
                        }
                    }
                }

                // Channel 4 envelope handling
                if (GBC_APU_Channel4EnvelopeEnabled && --GBC_APU_Channel4EnvelopeLengthCounter <= 0)
                {
                    GBC_APU_Channel4EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel4EnvelopeSweepNumber;

                    if (GBC_APU_Channel4EnvelopeLengthCounter == 0)
                    {
                        GBC_APU_Channel4EnvelopeLengthCounter = 8;
                    }
                    else
                    {
                        // Envelope Direction (0 = Decrease, 1 = Increase)
                        if (GBC_MMU_Memory.IO.Channel4EnvelopeDirection)
                        {
                            if (GBC_APU_Channel4EnvelopeVolume < 15)
                            {
                                GBC_APU_Channel4EnvelopeVolume++;
                            }
                            else
                            {
                                GBC_APU_Channel4EnvelopeEnabled = false;
                            }
                        }
                        else
                        {
                            if (GBC_APU_Channel4EnvelopeVolume > 0)
                            {
                                GBC_APU_Channel4EnvelopeVolume--;
                            }
                            else
                            {
                                GBC_APU_Channel4EnvelopeEnabled = false;
                            }
                        }
                    }
                }

                break;
            }
        }
    }

    GBC_APU_Ticks += GBC_CPU_StepTicks;

    GBC_APU_Channel1PhaseTicks += GBC_CPU_StepTicks;
    GBC_APU_Channel2PhaseTicks += GBC_CPU_StepTicks;
    GBC_APU_Channel3PhaseTicks += GBC_CPU_StepTicks;
    GBC_APU_Channel4Ticks      += GBC_CPU_StepTicks;

    // Calculate sample
    if (GBC_APU_Ticks >= GBC_APU_SAMPLE_RATE)
    {
        /***************************************/
        /************** CHANNEL 1 **************/
        /***************************************/

        if (GBC_APU_Channel1PhaseTicks > 0)
        {
            do
            {
                GBC_APU_Channel1PhaseTicks -= GBC_APU_Channel1PhasePeriod;

                ++GBC_APU_Channel1Phase;
                GBC_APU_Channel1Phase &= 7;
            }
            while (GBC_APU_Channel1PhaseTicks > 0);

            if (IS_CHANNEL1_DAC_ENABLED() && GBC_MMU_Memory.IO.ChannelSound1Enabled)
            {
                uint32_t p = (GBC_APU_Channel1Phase + SquareDutyOffsets[GBC_MMU_Memory.IO.Channel1WavePatternDuty]) & 7;

                if (p < SquareDuties[GBC_MMU_Memory.IO.Channel1WavePatternDuty])
                {
                    GBC_APU_Channel1LastSample = GBC_APU_Channel1EnvelopeVolume;
                }
                else
                {
                    GBC_APU_Channel1LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
                }
            }
            else
            {
                GBC_APU_Channel1LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
            }
        }

        /***************************************/
        /************** CHANNEL 2 **************/
        /***************************************/

        if (GBC_APU_Channel2PhaseTicks > 0)
        {
            do
            {
                GBC_APU_Channel2PhaseTicks -= GBC_APU_Channel2PhasePeriod;

                ++GBC_APU_Channel2Phase;
                GBC_APU_Channel2Phase &= 7;
            }
            while (GBC_APU_Channel2PhaseTicks > 0);

            if (IS_CHANNEL2_DAC_ENABLED() && GBC_MMU_Memory.IO.ChannelSound2Enabled)
            {
                uint32_t p = (GBC_APU_Channel2Phase + SquareDutyOffsets[GBC_MMU_Memory.IO.Channel2WavePatternDuty]) & 7;

                if (p < SquareDuties[GBC_MMU_Memory.IO.Channel2WavePatternDuty])
                {
                    GBC_APU_Channel2LastSample = GBC_APU_Channel2EnvelopeVolume;
                }
                else
                {
                    GBC_APU_Channel2LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
                }
            }
            else
            {
                GBC_APU_Channel2LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
            }
        }

        /***************************************/
        /************** CHANNEL 3 **************/
        /***************************************/

        if (GBC_APU_Channel3PhaseTicks > 0)
        {
            do
            {
                GBC_APU_Channel3PhaseTicks -= GBC_APU_Channel3PhasePeriod;

                ++GBC_APU_Channel3Phase;
                GBC_APU_Channel3Phase &= 0x1F;
            }
            while (GBC_APU_Channel3PhaseTicks > 0);

            if (GBC_MMU_Memory.IO.ChannelSound3Enabled)
            {
                // Wave pattern RAM holds 32 4-bit samples that are played back upper 4 bits first.
                GBC_APU_Channel3LastSample = (GBC_MMU_Memory.IO.Channel3WavePatternRAM[GBC_APU_Channel3Phase >> 1] << ((GBC_APU_Channel3Phase << 2) & 4)) & 0xF0;
                GBC_APU_Channel3LastSample = (GBC_APU_Channel3LastSample * WaveVolumeMultipliers[GBC_MMU_Memory.IO.Channel3SelectOutputLevel]) >> 6;
            }
            else
            {
                GBC_APU_Channel3LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
            }
        }

        /***************************************/
        /************** CHANNEL 4 **************/
        /***************************************/

        if (GBC_APU_Channel4Ticks > 0)
        {
            if (GBC_APU_Channel4Ticks > GBC_APU_Channel4Period)
            {
                GBC_APU_Channel4Ticks %= GBC_APU_Channel4Period;
            }

            GBC_APU_Channel4Ticks -= GBC_APU_Channel4Period;

            // If first and second LFSR bits differ
            if ((GBC_APU_Channel4LFSR & 0x1) ^ ((GBC_APU_Channel4LFSR & 0x2) >> 1))
            {
                GBC_APU_Channel4LFSR >>= 1;

                GBC_APU_Channel4LFSR |= 1 << 14;
                if (GBC_MMU_Memory.IO.Channel4PolynomialCounterStepWidth)
                {
                    GBC_APU_Channel4LFSR |= 1 << 6;
                }
            }
            else
            {
                GBC_APU_Channel4LFSR >>= 1;
            }

            if (GBC_MMU_Memory.IO.ChannelSound4Enabled && ~GBC_APU_Channel4LFSR & 0x1)
            {
                GBC_APU_Channel4LastSample = GBC_APU_Channel4EnvelopeVolume;
            }
            else
            {
                GBC_APU_Channel4LastSample = GBC_APU_DAC_OFF_AMPLITUDE;
            }
        }

        /***************************************/
        /*************** MIXING ****************/
        /***************************************/

        if (GBC_MMU_Memory.IO.SoundOutputChannel1ToSO1)
        {
            r += GBC_APU_Channel1LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel1ToSO2)
        {
            l += GBC_APU_Channel1LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel2ToSO1)
        {
            r += GBC_APU_Channel2LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel2ToSO2)
        {
            l += GBC_APU_Channel2LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel3ToSO1)
        {
            r += GBC_APU_Channel3LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel3ToSO2)
        {
            l += GBC_APU_Channel3LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel4ToSO1)
        {
            r += GBC_APU_Channel4LastSample;
        }

        if (GBC_MMU_Memory.IO.SoundOutputChannel4ToSO2)
        {
            l += GBC_APU_Channel4LastSample;
        }

        // ToDo: Implement ChannelControlOutputLevelSO1 and ChannelControlOutputLevelSO2

        if (l > 63)
        {
            l = 64;
        }

		if (r > 63)
        {
            r = 64;
        }

        GBC_APU_Buffer_L[GBC_APU_BufferPosition] = l;
        GBC_APU_Buffer_R[GBC_APU_BufferPosition] = r;

        GBC_APU_BufferPosition++;
        if (GBC_APU_BufferPosition >= GBC_APU_BUFFER_SIZE)
        {
            GBC_APU_BufferPosition = 0;
        }

        l = 0;
        r = 0;

        GBC_APU_Ticks -= GBC_APU_SAMPLE_RATE;
    }
}

void GBC_APU_OnWriteToSoundRegister(uint16_t address, uint8_t value, uint8_t oldValue)
{
    switch (address)
    {
        case 0xFF10:
            if (GBC_APU_Channel1SweepEnabled && // Sweep enabled
                GBC_APU_Channel1SweepDecrease && // Frequency decreases
                !(value & 0x08)) // Frequency increases now
            {
                GBC_MMU_Memory.IO.ChannelSound1Enabled = false;
            }

            break;
        case 0xFF11:
            GBC_APU_Channel1LengthCounter = 64 - GBC_MMU_Memory.IO.Channel1SoundLengthData;
            break;
        case 0xFF12:
            if (!IS_CHANNEL1_DAC_ENABLED())
            {
                GBC_MMU_Memory.IO.ChannelSound1Enabled = false;
            }

            GBC_APU_Channel1EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel1EnvelopeSweepNumber;
            GBC_APU_Channel1EnvelopeVolume = GBC_MMU_Memory.IO.Channel1InitialEnvelopeVolume;
            break;
        case 0xFF13:
            GBC_APU_Channel1PhaseFrequency = GET_CHANNEL1_FREQUENCY();
            GBC_APU_Channel1PhasePeriod = (2048 - GBC_APU_Channel1PhaseFrequency) * 4;
            break;
        case 0xFF14:
            GBC_APU_Channel1PhaseFrequency = GET_CHANNEL1_FREQUENCY();
            GBC_APU_Channel1PhasePeriod = (2048 - GBC_APU_Channel1PhaseFrequency) * 4;

            // Check for counter activation in first half of length period
            if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                !(oldValue & 0x40) && // Counter is disabled
                    (value & 0x40) && // Counter is enabled now
                GBC_APU_Channel1LengthCounter)
            {
                --GBC_APU_Channel1LengthCounter;
            }

            if (GBC_MMU_Memory.IO.Channel1InitialRestart)
            {
                if (IS_CHANNEL1_DAC_ENABLED())
                {
                    GBC_MMU_Memory.IO.ChannelSound1Enabled = true;
                }

                // Trigger should treat 0 counter as maximum
                if (GBC_APU_Channel1LengthCounter == 0)
                {
                    GBC_APU_Channel1LengthCounter = 64;

                    if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                        GBC_MMU_Memory.IO.Channel1CounterSelection)
                    {
                        --GBC_APU_Channel1LengthCounter;
                    }
                }

                GBC_APU_Channel1SweepEnabled = IS_CHANNEL1_SWEEP_ENABLED() ? true : false;
                GBC_APU_Channel1SweepFrequency = GET_CHANNEL1_FREQUENCY();
                GBC_APU_Channel1SweepDecrease = false;
                GBC_APU_Channel1SweepLengthCounter = GBC_MMU_Memory.IO.Channel1SweepTime;

                // Sweep counter treats period 0 as 8
                if (GBC_APU_Channel1SweepLengthCounter == 0)
                {
                    GBC_APU_Channel1SweepLengthCounter = 8;
                }

                // If shift > 0, calculates on trigger
                if (GBC_MMU_Memory.IO.Channel1SweepShift)
                {
                    CALCULATE_CHANNEL1_SWEEP(false);
                }

                GBC_APU_Channel1EnvelopeEnabled = true;
                GBC_APU_Channel1EnvelopeVolume = GBC_MMU_Memory.IO.Channel1InitialEnvelopeVolume;
                GBC_APU_Channel1EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel1EnvelopeSweepNumber;

                if (GBC_APU_Channel1EnvelopeLengthCounter == 0)
                {
                    GBC_APU_Channel1EnvelopeLengthCounter = 8;
                }
            }
            
            if (GBC_APU_Channel1LengthCounter == 0)
            {
                GBC_MMU_Memory.IO.ChannelSound1Enabled = false;
            }

            break;
        case 0xFF16:
            GBC_APU_Channel2LengthCounter = 64 - GBC_MMU_Memory.IO.Channel2SoundLengthData;
            break;
        case 0xFF17:
            if (!IS_CHANNEL2_DAC_ENABLED())
            {
                GBC_MMU_Memory.IO.ChannelSound2Enabled = false;
            }

            GBC_APU_Channel2EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel2EnvelopeSweepNumber;
            GBC_APU_Channel2EnvelopeVolume = GBC_MMU_Memory.IO.Channel2InitialEnvelopeVolume;
            break;
        case 0xFF18:
            GBC_APU_Channel2PhaseFrequency = GET_CHANNEL2_FREQUENCY();
            GBC_APU_Channel2PhasePeriod = (2048 - GBC_APU_Channel2PhaseFrequency) * 4;
            break;
        case 0xFF19:
            GBC_APU_Channel2PhaseFrequency = GET_CHANNEL2_FREQUENCY();
            GBC_APU_Channel2PhasePeriod = (2048 - GBC_APU_Channel2PhaseFrequency) * 4;

            // Check for counter activation in first half of length period
            if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                !(oldValue & 0x40) && // Counter is disabled
                    (value & 0x40) && // Counter is enabled now
                GBC_APU_Channel2LengthCounter)
            {
                --GBC_APU_Channel2LengthCounter;
            }

            if (GBC_MMU_Memory.IO.Channel2InitialRestart)
            {
                if (IS_CHANNEL2_DAC_ENABLED())
                {
                    GBC_MMU_Memory.IO.ChannelSound2Enabled = true;
                }

                // Trigger should treat 0 counter as maximum
                if (GBC_APU_Channel2LengthCounter == 0)
                {
                    GBC_APU_Channel2LengthCounter = 64;

                    if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                        GBC_MMU_Memory.IO.Channel2CounterSelection)
                    {
                        --GBC_APU_Channel2LengthCounter;
                    }
                }

                GBC_APU_Channel2EnvelopeEnabled = true;
                GBC_APU_Channel2EnvelopeVolume = GBC_MMU_Memory.IO.Channel2InitialEnvelopeVolume;
                GBC_APU_Channel2EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel2EnvelopeSweepNumber;

                if (GBC_APU_Channel2EnvelopeLengthCounter == 0)
                {
                    GBC_APU_Channel2EnvelopeLengthCounter = 8;
                }
            }
            
            if (GBC_APU_Channel2LengthCounter == 0)
            {
                GBC_MMU_Memory.IO.ChannelSound2Enabled = false;
            }

            break;
        case 0xFF1A:
            if (!IS_CHANNEL3_DAC_ENABLED())
            {
                GBC_MMU_Memory.IO.ChannelSound3Enabled = false;
            }
            break;
        case 0xFF1B:
            GBC_APU_Channel3LengthCounter = 256 - GBC_MMU_Memory.IO.Channel3SoundLength;
            break;
        case 0xFF1D:
            GBC_APU_Channel3PhaseFrequency = GET_CHANNEL3_FREQUENCY();
            GBC_APU_Channel3PhasePeriod = (2048 - GBC_APU_Channel3PhaseFrequency) * 2; // (4194304 / (4194304 / (64 * (2048 - f)))) / 32
            break;
        case 0xFF1E:
            GBC_APU_Channel3PhaseFrequency = GET_CHANNEL3_FREQUENCY();
            GBC_APU_Channel3PhasePeriod = (2048 - GBC_APU_Channel3PhaseFrequency) * 2; // (4194304 / (4194304 / (64 * (2048 - f)))) / 32

            // Check for counter activation in first half of length period
            if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                !(oldValue & 0x40) && // Counter is disabled
                    (value & 0x40) && // Counter is enabled now
                GBC_APU_Channel3LengthCounter)
            {
                --GBC_APU_Channel3LengthCounter;
            }

            if (GBC_MMU_Memory.IO.Channel3InitialRestart)
            {
                if (IS_CHANNEL3_DAC_ENABLED())
                {
                    int32_t delay = GBC_APU_Channel3PhaseTicks < 0 ? -GBC_APU_Channel3PhaseTicks : 0;

                    // Channel was enabled and we are in DMG mode
                    if (GBC_MMU_Memory.IO.ChannelSound3Enabled && GBC_MMU_IS_DMG_MODE() &&
                        ((uint32_t)(delay - 2)) < 2)
                    {
                        // Wave corruption
                        uint32_t index = ((GBC_APU_Channel3Phase + 1) & 0x1F) >> 1;

                        if (index < 4)
                        {
                            GBC_MMU_Memory.IO.Channel3WavePatternRAM[0] = GBC_MMU_Memory.IO.Channel3WavePatternRAM[index];
                        }
                        else
                        {
                            index &= ~3;

                            for (uint32_t i = 0; i < 4; ++i)
                            {
                                GBC_MMU_Memory.IO.Channel3WavePatternRAM[i] = GBC_MMU_Memory.IO.Channel3WavePatternRAM[index + i];
                            }
                        }
                    }

                    GBC_MMU_Memory.IO.ChannelSound3Enabled = true;
                }
                else
                {
                    GBC_MMU_Memory.IO.ChannelSound3Enabled = false;
                }

                GBC_APU_Channel3Phase = 0;
                GBC_APU_Channel3PhaseTicks = -(GBC_APU_Channel3PhasePeriod + 6); // Start delay

                // Trigger should treat 0 counter as maximum
                if (GBC_APU_Channel3LengthCounter == 0)
                {
                    GBC_APU_Channel3LengthCounter = 256;

                    if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                        GBC_MMU_Memory.IO.Channel3CounterSelection)
                    {
                        --GBC_APU_Channel3LengthCounter;
                    }
                }
            }
            
            if (GBC_APU_Channel3LengthCounter == 0)
            {
                GBC_MMU_Memory.IO.ChannelSound3Enabled = false;
            }

            break;
        case 0xFF20:
            GBC_APU_Channel4LengthCounter = 64 - (GBC_MMU_Memory.IO.Channel4SoundLengthData & 63);
            break;
        case 0xFF21:
            if (!IS_CHANNEL4_DAC_ENABLED())
            {
                GBC_MMU_Memory.IO.ChannelSound4Enabled = false;
            }

            GBC_APU_Channel4EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel4EnvelopeSweepNumber;
            GBC_APU_Channel4EnvelopeVolume = GBC_MMU_Memory.IO.Channel4InitialEnvelopeVolume;
            break;
        case 0xFF22:
            GBC_APU_Channel4Frequency = GBC_APU_BaseNoiseFrequencies[GBC_MMU_Memory.IO.Channel4PolynomialCounterFreqDivRatio] >> (GBC_MMU_Memory.IO.Channel4PolynomialCounterShiftClockFreq + 1);
            GBC_APU_Channel4Period = 4194304 / GBC_APU_Channel4Frequency;
            break;
        case 0xFF23:
            // Check for counter activation in first half of length period
            if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                !(oldValue & 0x40) && // Counter is disabled
                    (value & 0x40) && // Counter is enabled now
                GBC_APU_Channel4LengthCounter)
            {
                --GBC_APU_Channel4LengthCounter;
            }

            if (GBC_MMU_Memory.IO.Channel4InitialRestart)
            {
                if (IS_CHANNEL4_DAC_ENABLED())
                {
                    GBC_MMU_Memory.IO.ChannelSound4Enabled = true;
                }

                // Trigger should treat 0 counter as maximum
                if (GBC_APU_Channel4LengthCounter == 0)
                {
                    GBC_APU_Channel4LengthCounter = 64;

                    if (IS_APU_IN_FIRST_HALF_OF_LENGTH_PERIOD() &&
                        GBC_MMU_Memory.IO.Channel4CounterSelection)
                    {
                        --GBC_APU_Channel4LengthCounter;
                    }
                }

                GBC_APU_Channel4Ticks = -8; // Start delay
                GBC_APU_Channel4LFSR = 0x7FFF;

                GBC_APU_Channel4EnvelopeEnabled = true;
                GBC_APU_Channel4EnvelopeVolume = GBC_MMU_Memory.IO.Channel4InitialEnvelopeVolume;
                GBC_APU_Channel4EnvelopeLengthCounter = GBC_MMU_Memory.IO.Channel4EnvelopeSweepNumber;

                if (GBC_APU_Channel4EnvelopeLengthCounter == 0)
                {
                    GBC_APU_Channel4EnvelopeLengthCounter = 8;
                }
            }
            
            if (GBC_APU_Channel4LengthCounter == 0)
            {
                GBC_MMU_Memory.IO.ChannelSound4Enabled = false;
            }

            break;
    }
}
