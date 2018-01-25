#ifndef GBC_APU_H
#define GBC_APU_H

#include "common.h"

#define GBC_APU_BUFFER_SIZE 512
#define GBC_APU_SAMPLE_RATE 131

void GBC_APU_Initialize(void);
void GBC_APU_Step(void);
void GBC_APU_OnWriteToSoundRegister(uint16_t address, uint8_t value);

#endif
