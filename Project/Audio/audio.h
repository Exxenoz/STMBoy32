#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "audio_config.h"

#define DAC_DHR12R1_ADDRESS 0x40007408

void Audio_Initialize(void);
void Audio_SetAudioBuffer(uint16_t* audioBuffer, uint32_t audioBufferSize);
void DACx_DMA_IRQHandler(void);

#endif
