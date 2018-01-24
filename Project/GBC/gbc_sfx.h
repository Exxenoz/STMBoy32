#ifndef GBC_SFX_H
#define GBC_SFX_H

#include "common.h"

#define GBC_SFX_BUFFER_SIZE 512
#define GBC_SFX_SAMPLE_RATE 131

void GBC_SFX_Initialize(void);
void GBC_SFX_Step(void);
void GBC_SFX_OnWriteToSoundRegister(uint16_t address, uint8_t value);

#endif
