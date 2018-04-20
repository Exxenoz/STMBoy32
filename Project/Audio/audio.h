#ifndef AUDIO_H
#define AUDIO_H

#include "common.h"
#include "audio_config.h"

void Audio_Initialize(void);
void Audio_SetAudioBuffer(uint16_t* audioBuffer, uint32_t audioBufferSize);
void DACx_DMA_IRQHandler(void);

#endif
