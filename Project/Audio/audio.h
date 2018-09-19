#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"


#define AUDIO_SET_SD        AUDIO_SD_PORT->BSRRL |= AUDIO_SD_PIN
#define AUDIO_RST_SD        AUDIO_SD_PORT->BSRRH |= AUDIO_SD_PIN


extern volatile bool Audio_IsPlayingOfBufferFinished;



void Audio_Initialize(void);
void Audio_EnablePower(bool enable);
void Audio_SetAudioBuffer(uint16_t* audioBufferL, uint16_t* audioBufferR, uint32_t audioBufferSize);

void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac);
void HAL_DAC_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac);

#endif
