#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "audio_config.h"


extern volatile bool Audio_IsPlayingOfBufferFinished;



void Audio_Initialize(void);
void Audio_EnablePower(bool enable);
void Audio_SetAudioBuffer(uint8_t* audioBufferL, uint8_t* audioBufferR, uint32_t audioBufferSize);

void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac);
void HAL_DAC_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac);

#endif
