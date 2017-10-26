#ifndef GBC_CPU_EX
#define GBC_CPU_EX

#include "common.h"

typedef struct GBC_CPU_EX_Instruction_s
{
    void* Handler;
    uint8_t Ticks;
}
GBC_CPU_EX_Instruction_t;

extern const GBC_CPU_EX_Instruction_t GBC_CPU_EX_Instructions[48]; // External declaration for base instruction 0xCB

#endif
