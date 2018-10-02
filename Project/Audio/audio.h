#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "audio_config.h"

typedef enum Audio_OutputMode_e
{
    AUDIO_OUTPUTMODE_NONE = 0,
    AUDIO_OUTPUTMODE_GBC = 1,
    AUDIO_OUTPUTMODE_WAV = 2,
}
Audio_OutputMode_t;

typedef enum Audio_SampleAlignment_e
{
    AUDIO_SAMPLE_ALIGNMENT_12B_R = DAC_ALIGN_12B_R,
    AUDIO_SAMPLE_ALIGNMENT_12B_L = DAC_ALIGN_12B_L,
    AUDIO_SAMPLE_ALIGNMENT_8B_R = DAC_ALIGN_8B_R,
}
Audio_SampleAlignment_t;

typedef enum Audio_Playback_e
{
    AUDIO_PLAYBACK_NONE = 0,
    AUDIO_PLAYBACK_FULL = 1,
    AUDIO_PLAYBACK_HALF1 = 2,
    AUDIO_PLAYBACK_HALF2 = 3,
}
Audio_Playback_t;


void Audio_Initialize(void);
void Audio_EnablePower(bool enable);

void Audio_SetOutputBuffer(Audio_OutputMode_t outputMode, Audio_SampleAlignment_t sampleAlignment,
    uint8_t* bufferLeft, uint8_t* bufferRight, uint32_t bufferSize);
void Audio_StartOutput(Audio_Playback_t playback, bool loop);
void Audio_StopOutput(void);
void Audio_Update(void);

void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac);
void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac);

#endif
