#ifndef GBC_APU_H
#define GBC_APU_H

#include "common.h"

#define GBC_APU_SAMPLE_RATE 132     // In CPU ticks
#define GBC_APU_BUFFER_SIZE 532 * 2 // Samples per frame doubled due to double buffering. Must be (70224 / GBC_APU_SAMPLE_RATE) * 2

#define GBC_APU_DAC_OFF_AMPLITUDE 0

extern uint32_t GBC_APU_Channel3Phase; // External memory declaration for MMU access
extern int32_t  GBC_APU_Channel3PhaseTicks; // External memory declaration for MMU access

void GBC_APU_Initialize(void);
void GBC_APU_Step(void);
void GBC_APU_OnWriteToSoundRegister(uint16_t address, uint8_t value, uint8_t oldValue);

#endif
