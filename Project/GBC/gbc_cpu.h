#ifndef GBC_CPU_H
#define GBC_CPU_H

#include "common.h"

typedef struct GBC_CPU_Register_s
{
    union
    {
        struct
        {
            uint8_t F; // LSB in AF, Flags
            uint8_t A; // MSB in AF
        };

        uint16_t AF;
    };

    union
    {
        struct
        {
            uint8_t C; // LSB in BC
            uint8_t B; // MSB in BC
        };

        uint16_t BC;
    };

    union
    {
        struct
        {
            uint8_t E; // LSB in DE
            uint8_t D; // MSB in DE
        };

        uint16_t DE;
    };

    union
    {
        struct
        {
            uint8_t L; // LSB in HL
            uint8_t H; // MSB in HL
        };

        uint16_t HL;
    };

    uint16_t PC; // Program counter
    uint16_t SP; // Stack pointer
}
GBC_CPU_Register_t;

enum
{
    GBC_CPU_OPERAND_BYTES_0 = 0,
    GBC_CPU_OPERAND_BYTES_1 = 1,
    GBC_CPU_OPERAND_BYTES_2 = 2,
};

enum
{
    GBC_CPU_TICKS_0  =  0,
    GBC_CPU_TICKS_1  =  1,
    GBC_CPU_TICKS_2  =  2,
    GBC_CPU_TICKS_3  =  3,
    GBC_CPU_TICKS_4  =  4,
    GBC_CPU_TICKS_5  =  5,
    GBC_CPU_TICKS_6  =  6,
    GBC_CPU_TICKS_7  =  7,
    GBC_CPU_TICKS_8  =  8,
    GBC_CPU_TICKS_9  =  9,
    GBC_CPU_TICKS_10 = 10,
    GBC_CPU_TICKS_12 = 12,
    GBC_CPU_TICKS_16 = 16,
    GBC_CPU_TICKS_20 = 20,
    GBC_CPU_TICKS_24 = 24,
};

typedef struct GBC_CPU_Instruction_s
{
    void* Handler;
    uint8_t OperandBytes;
    uint8_t Ticks;
}
GBC_CPU_Instruction_t;

typedef enum GBC_CPU_MemoryAccessDelayState_e
{
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE           = 0,
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT  = 1,
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT2 = 2,
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT  = 3,
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT2 = 4,
    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT3 = 5,
}
GBC_CPU_MemoryAccessDelayState_t;

enum GBC_CPU_Flags_e
{
    // Bits 0-3 should always be zero
    GBC_CPU_FLAGS_CARRY       = 1 << 4,
    GBC_CPU_FLAGS_HALFCARRY   = 1 << 5,
    GBC_CPU_FLAGS_SUBTRACTION = 1 << 6,
    GBC_CPU_FLAGS_ZERO        = 1 << 7,

    GBC_CPU_FLAGS_ALL = GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY |
                        GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO,
};

#define GBC_CPU_FLAGS_SET(FLAGS)   (GBC_CPU_Register.F |=  (FLAGS))
#define GBC_CPU_FLAGS_CLEAR(FLAGS) (GBC_CPU_Register.F &= ~(FLAGS))
#define GBC_CPU_FLAGS_HAS(FLAGS)   (GBC_CPU_Register.F & (FLAGS))
#define GBC_CPU_FLAGS_CLEAR_ALL     GBC_CPU_Register.F = 0

extern GBC_CPU_Register_t GBC_CPU_Register; // External declaration for extended instructions file
extern uint32_t GBC_CPU_Ticks;              // External declaration for GBC file
extern uint32_t GBC_CPU_StepTicks;          // External declaration for GPU file
extern GBC_CPU_MemoryAccessDelayState_t GBC_CPU_MemoryAccessDelayState; // External declaration for CPU Ex file

void GBC_CPU_Initialize(void);
void GBC_CPU_Step(void);

#endif
