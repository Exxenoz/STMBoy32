#include "gbc_cpu.h"
#include "gbc_cpu_ex.h"
#include "gbc_mmu.h"
#include "string.h"

// Uncomment the following debug lines to enable debug functionality
//#define DEBUG_PRINT_INSTRUCTION_CALLS
//#define DEBUG_COUNT_INSTRUCTIONS

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
#include <stdio.h>
#endif

#define GBC_CPU_USE_FUNCTION_PTR

GBC_CPU_Register_t GBC_CPU_Register;                  // Register

uint32_t GBC_CPU_PendingInterrupts = 0;               // Current pending interrupts
uint16_t GBC_CPU_InstructionAddress = 0;              // Current instruction address
uint32_t GBC_CPU_InstructionOpcode = 0;               // Current instruction opcode
uint32_t GBC_CPU_InstructionTicks = 0;                // Current instruction ticks
uint32_t GBC_CPU_StepTicks = 0;                       // Step ticks
int32_t  GBC_CPU_UnhaltTicks = 0;                     // Unhalt ticks
int32_t  GBC_CPU_InterruptMasterEnableDelayTicks = 0; // IME will be enabled after this delay (if != 0)
uint32_t GBC_CPU_InterruptMasterEnable = true;        // Interrupt master
uint32_t GBC_CPU_Halted = false;                      // Halted state
uint32_t GBC_CPU_SpeedModifier = 0;                   // 0 ... normal Speed, 1 ... CGB Speed; needed, because CurrentSpeed flag is set per default in DMG mode

//GBC_CPU_MemoryAccessDelayState_t GBC_CPU_MemoryAccessDelayState =
//    GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE; // Memory access delay state for current instruction

extern const GBC_CPU_Instruction_t GBC_CPU_Instructions[256];

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
typedef struct
{
    long instr_calls[512];
}
InstructionCalls_t;

InstructionCalls_t GBC_CPU_InstructionCalls;
bool GBC_CPU_DebugPrintEnabled = false;
#endif

#ifdef DEBUG_COUNT_INSTRUCTIONS
long GBC_CPU_InstructionsPerStep = 0;
#endif

#define GBC_CPU_PUSH_TO_STACK(VALUE)                                                                                                               \
{                                                                                                                                                  \
    GBC_CPU_Register.SP -= 2;                                                                                                                      \
                                                                                                                                                   \
    switch (GBC_CPU_Register.SP & 0xF000)                                                                                                          \
    {                                                                                                                                              \
        case 0xC000: /* WRAM Bank 0 */                                                                                                             \
            GBC_MMU_Memory.WRAMBank0[GBC_CPU_Register.SP + 0 - 0xC000] = ((VALUE) & 0xFF);        /* Low  */                                       \
            GBC_MMU_Memory.WRAMBank0[GBC_CPU_Register.SP + 1 - 0xC000] = ((VALUE) & 0xFF00) >> 8; /* High */                                       \
            break;                                                                                                                                 \
        case 0xD000: /* WRAM Bank X */                                                                                                             \
            switch (GBC_MMU_Memory.WRAMBankID)                                                                                                     \
            {                                                                                                                                      \
                case 0:                                                                                                                            \
                case 1:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 2:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank2[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank2[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 3:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank3[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank3[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 4:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank4[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank4[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 5:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank5[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank5[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 6:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank6[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank6[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                case 7:                                                                                                                            \
                    GBC_MMU_Memory.WRAMBank7[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank7[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
                default:                                                                                                                           \
                    GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 0 - 0xD000] = ((VALUE) & 0xFF);        /* Low  */                               \
                    GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 1 - 0xD000] = ((VALUE) & 0xFF00) >> 8; /* High */                               \
                    break;                                                                                                                         \
            }                                                                                                                                      \
            break;                                                                                                                                 \
        case 0xF000: /* HRAM */                                                                                                                    \
            GBC_MMU_Memory.HRAM[GBC_CPU_Register.SP + 0 - 0xFF80] = ((VALUE) & 0xFF);        /* Low  */                                            \
            GBC_MMU_Memory.HRAM[GBC_CPU_Register.SP + 1 - 0xFF80] = ((VALUE) & 0xFF00) >> 8; /* High */                                            \
            break;                                                                                                                                 \
        default:                                                                                                                                   \
            GBC_MMU_WriteShort(GBC_CPU_Register.SP, (VALUE));                                                                                      \
            break;                                                                                                                                 \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_POP_FROM_STACK(VALUE)                                                                                                              \
{                                                                                                                                                  \
    switch (GBC_CPU_Register.SP & 0xF000)                                                                                                          \
    {                                                                                                                                              \
        case 0xC000: /* WRAM Bank 0 */                                                                                                             \
            VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank0[GBC_CPU_Register.SP + 1 - 0xC000]) << 8) |                                                \
                GBC_MMU_Memory.WRAMBank0[GBC_CPU_Register.SP - 0xC000];                                                                            \
            break;                                                                                                                                 \
        case 0xD000: /* WRAM Bank X */                                                                                                             \
            switch (GBC_MMU_Memory.WRAMBankID)                                                                                                     \
            {                                                                                                                                      \
                case 0:                                                                                                                            \
                case 1:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 2:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank2[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank2[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 3:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank3[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank3[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 4:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank4[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank4[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 5:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank5[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank5[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 6:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank6[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank6[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                case 7:                                                                                                                            \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank7[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank7[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
                default:                                                                                                                           \
                    VALUE = (((uint16_t)GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP + 1 - 0xD000]) << 8) |                                        \
                        GBC_MMU_Memory.WRAMBank1[GBC_CPU_Register.SP - 0xD000];                                                                    \
                    break;                                                                                                                         \
            }                                                                                                                                      \
            break;                                                                                                                                 \
        case 0xF000: /* HRAM */                                                                                                                    \
            VALUE = (((uint16_t)GBC_MMU_Memory.HRAM[GBC_CPU_Register.SP + 1 - 0xFF80]) << 8) | GBC_MMU_Memory.HRAM[GBC_CPU_Register.SP - 0xFF80];  \
            break;                                                                                                                                 \
        default:                                                                                                                                   \
            VALUE = GBC_MMU_ReadShort(GBC_CPU_Register.SP);                                                                                        \
            break;                                                                                                                                 \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_Register.SP += 2;                                                                                                                      \
}                                                                                                                                                  \

#define GBC_CPU_INC(VALUE)                                                                                                                         \
{                                                                                                                                                  \
    VALUE++;                                                                                                                                       \
                                                                                                                                                   \
    /* Carry flag not affected */                                                                                                                  \
                                                                                                                                                   \
    if ((VALUE & 0x0F) == 0x00)                                                                                                                    \
    {                                                                                                                                              \
        if (VALUE)                                                                                                                                 \
        {                                                                                                                                          \
            GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | GBC_CPU_FLAGS_HALFCARRY;                                             \
        }                                                                                                                                          \
        else                                                                                                                                       \
        {                                                                                                                                          \
            GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | (GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_ZERO);                      \
        }                                                                                                                                          \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_Register.F &= ~(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);                                         \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_DEC(VALUE)                                                                                                                         \
{                                                                                                                                                  \
    VALUE--;                                                                                                                                       \
                                                                                                                                                   \
    /* Carry flag not affected */                                                                                                                  \
                                                                                                                                                   \
    if ((VALUE & 0x0F) == 0x0F)                                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | (GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        if (VALUE)                                                                                                                                 \
        {                                                                                                                                          \
            GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | GBC_CPU_FLAGS_SUBTRACTION;                                           \
        }                                                                                                                                          \
        else                                                                                                                                       \
        {                                                                                                                                          \
            GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | (GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);                    \
        }                                                                                                                                          \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_ADD(A, B)                                                                                                                          \
{                                                                                                                                                  \
    uint32_t result = (A) + (B);                                                                                                                   \
                                                                                                                                                   \
    if (result & 0x100)                                                                                                                            \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    if ((((A) & 0x0F) + ((B) & 0x0F)) & 0x10)                                                                                                      \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                \
                                                                                                                                                   \
    result &= 0xFF;                                                                                                                                \
                                                                                                                                                   \
    if (result)                                                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);                                                                                                     \
    }                                                                                                                                              \
                                                                                                                                                   \
    A = result;                                                                                                                                    \
}                                                                                                                                                  \

#define GBC_CPU_ADC(A, B)                                                                                                                          \
{                                                                                                                                                  \
    uint8_t carry = (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);                                                                              \
    uint32_t result = (A) + (B) + carry;                                                                                                           \
                                                                                                                                                   \
    if (result & 0x100)                                                                                                                            \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    if ((((A) & 0x0F) + ((B) & 0x0F) + carry) & 0x10)                                                                                              \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                \
                                                                                                                                                   \
    result &= 0xFF;                                                                                                                                \
                                                                                                                                                   \
    if (result)                                                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);                                                                                                     \
    }                                                                                                                                              \
                                                                                                                                                   \
    A = result;                                                                                                                                    \
}                                                                                                                                                  \

#define GBC_CPU_SUB(A, B)                                                                                                                          \
{                                                                                                                                                  \
    uint8_t result = (A) - (B);                                                                                                                    \
                                                                                                                                                   \
    if ((B) > (A))                                                                                                                                 \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    if (((B) & 0x0F) > ((A) & 0x0F))                                                                                                               \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                  \
                                                                                                                                                   \
    if (result)                                                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);                                                                                                     \
    }                                                                                                                                              \
                                                                                                                                                   \
    A = result;                                                                                                                                    \
}                                                                                                                                                  \

#define GBC_CPU_SBC(A, B)                                                                                                                          \
{                                                                                                                                                  \
    int8_t carry = (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);                                                                               \
    int32_t result = (A) - (B) - carry;                                                                                                            \
                                                                                                                                                   \
    if (result < 0)                                                                                                                                \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    if ((((A) & 0x0F) - ((B) & 0x0F) - carry) < 0)                                                                                                 \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                  \
                                                                                                                                                   \
    result &= 0xFF;                                                                                                                                \
                                                                                                                                                   \
    if (result)                                                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);                                                                                                     \
    }                                                                                                                                              \
                                                                                                                                                   \
    A = result;                                                                                                                                    \
}                                                                                                                                                  \

#define GBC_CPU_ADD2(A, B)                                                                                                                         \
{                                                                                                                                                  \
    uint32_t result = A + B;                                                                                                                       \
                                                                                                                                                   \
    if (result > 0xFFFF)                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    result &= 0xFFFF;                                                                                                                              \
                                                                                                                                                   \
    if (((A & 0x0FFF) + (B & 0x0FFF)) > 0x0FFF)                                                                                                    \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                \
                                                                                                                                                   \
    /* Zero flag not affected */                                                                                                                   \
                                                                                                                                                   \
    A = result;                                                                                                                                    \
}                                                                                                                                                  \

#define GBC_CPU_AND(A, B)                                                                                                                          \
{                                                                                                                                                  \
    A = (A) & (B);                                                                                                                                 \
                                                                                                                                                   \
    if (A)                                                                                                                                         \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_HALFCARRY;                                                                                              \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_ZERO;                                                                         \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_OR(A, B)                                                                                                                           \
{                                                                                                                                                  \
    A = (A) | (B);                                                                                                                                 \
                                                                                                                                                   \
    if (A)                                                                                                                                         \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_ZERO;                                                                                                   \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_XOR(A, B)                                                                                                                          \
{                                                                                                                                                  \
    A = (A) ^ (B);                                                                                                                                 \
                                                                                                                                                   \
    if (A)                                                                                                                                         \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;                                                                                                   \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_Register.F = GBC_CPU_FLAGS_ZERO;                                                                                                   \
    }                                                                                                                                              \
}                                                                                                                                                  \

#define GBC_CPU_COMPARE(A, B)                                                                                                                      \
{                                                                                                                                                  \
    if ((A) < (B))                                                                                                                                 \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);                                                                                                    \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);                                                                                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
    if (((A) & 0xF) < ((B) & 0xF))                                                                                                                 \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);                                                                                                \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);                                                                                              \
    }                                                                                                                                              \
                                                                                                                                                   \
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);                                                                                                  \
                                                                                                                                                   \
    if ((A) == (B))                                                                                                                                \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);                                                                                                     \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);                                                                                                   \
    }                                                                                                                                              \
}                                                                                                                                                  \

void GBC_CPU_NOP()                      // 0x00 - No operation
{
    // Do nothing
}

void GBC_CPU_LD_BC_XX(uint16_t operand) // 0x01 - Load 16-bit immediate into BC
{
    GBC_CPU_Register.BC = operand;
}

void GBC_CPU_LD_BCP_A()                 // 0x02 - Save A to address pointed by BC
{
    GBC_MMU_WriteByte(GBC_CPU_Register.BC, GBC_CPU_Register.A);
}

void GBC_CPU_INC_BC()                   // 0x03 - Increment 16-bit BC
{
    GBC_CPU_Register.BC++;

    // Flags not affected
}

void GBC_CPU_INC_B()                    // 0x04 - Increment B
{
    GBC_CPU_INC(GBC_CPU_Register.B);
}

void GBC_CPU_DEC_B()                    // 0x05 - Decrement B
{
    GBC_CPU_DEC(GBC_CPU_Register.B);
}

void GBC_CPU_LD_B_X(uint8_t operand)    // 0x06 - Load 8-bit immediate into B
{
    GBC_CPU_Register.B = operand;
}

void GBC_CPU_RLC_A()                    // 0x07 - Rotate A left with carry
{
    if (GBC_CPU_Register.A & 0x80)
    {
        GBC_CPU_Register.A <<= 1;
        GBC_CPU_Register.A  |= 1;

        GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
    }
    else
    {
        GBC_CPU_Register.A <<= 1;

        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
    }
}

void GBC_CPU_LD_XXP_SP(uint16_t operand)// 0x08 - Save SP to given address
{
    GBC_MMU_WriteShort(operand, GBC_CPU_Register.SP);
}

void GBC_CPU_ADD_HL_BC()                // 0x09 - Add 16-bit BC to HL
{
    GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.BC);
}

void GBC_CPU_LD_A_BCP()                 // 0x0A - Load A from address pointed to by BC
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.BC);
}

void GBC_CPU_DEC_BC()                   // 0x0B - Decrement 16-bit BC
{
    GBC_CPU_Register.BC--;

    // No flags affected
}

void GBC_CPU_INC_C()                    // 0x0C - Increment C
{
    GBC_CPU_INC(GBC_CPU_Register.C);
}

void GBC_CPU_DEC_C()                    // 0x0D - Decrement C
{
    GBC_CPU_DEC(GBC_CPU_Register.C);
}

void GBC_CPU_LD_C_X(uint8_t operand)    // 0x0E - Load 8-bit immediate into C
{
    GBC_CPU_Register.C = operand;
}

void GBC_CPU_RRC_A()                    // 0x0F - Rotate A right with carry
{
    if (GBC_CPU_Register.A & 0x1)
    {
        GBC_CPU_Register.A >>= 1;
        GBC_CPU_Register.A  |= 0x80;

        GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
    }
    else
    {
        GBC_CPU_Register.A >>= 1;

        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
    }
}

void GBC_CPU_STOP()                     // 0x10 - Stop processor
{
    if (GBC_MMU_IS_CGB_MODE())
    {
        if (GBC_MMU_Memory.PrepareSpeedSwitch)
        {
            GBC_MMU_Memory.PrepareSpeedSwitch = 0;
            GBC_MMU_Memory.CurrentSpeed = !GBC_MMU_Memory.CurrentSpeed;
            GBC_CPU_SpeedModifier = GBC_CPU_SpeedModifier ? 0 : 1;
        }
    }
}

void GBC_CPU_LD_DE_XX(uint16_t operand) // 0x11 - Load 16-bit immediate into DE
{
    GBC_CPU_Register.DE = operand;
}

void GBC_CPU_LD_DEP_A()                 // 0x12 - Save A to address pointed by DE
{
    GBC_MMU_WriteByte(GBC_CPU_Register.DE, GBC_CPU_Register.A);
}

void GBC_CPU_INC_DE()                   // 0x13 - Increment 16-bit DE
{
    GBC_CPU_Register.DE++;

    // Flags not affected
}

void GBC_CPU_INC_D()                    // 0x14 - Increment D
{
    GBC_CPU_INC(GBC_CPU_Register.D);
}

void GBC_CPU_DEC_D()                    // 0x15 - Decrement D
{
    GBC_CPU_DEC(GBC_CPU_Register.D);
}

void GBC_CPU_LD_D_X(uint8_t operand)    // 0x16 - Load 8-bit immediate into D
{
    GBC_CPU_Register.D = operand;
}

void GBC_CPU_RL_A()                     // 0x17 - Rotate A left
{
    uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0;

    if (GBC_CPU_Register.A & 0x80)
    {
        GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
    }
    else
    {
        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
    }

    GBC_CPU_Register.A <<= 1;
    GBC_CPU_Register.A  |= carry;
}

void GBC_CPU_JR_X(uint8_t operand)      // 0x18 - Relative jump by signed immediate
{
    int8_t value = operand;
    GBC_CPU_Register.PC += value;
}

void GBC_CPU_ADD_HL_DE()                // 0x19 - Add 16-bit DE to HL
{
    GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.DE);
}

void GBC_CPU_LD_A_DEP()                 // 0x1A - Load A from address pointed to by DE
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.DE);
}

void GBC_CPU_DEC_DE()                   // 0x1B - Decrement 16-bit DE
{
    GBC_CPU_Register.DE--;

    // No flags affected
}

void GBC_CPU_INC_E()                    // 0x1C - Increment E
{
    GBC_CPU_INC(GBC_CPU_Register.E);
}

void GBC_CPU_DEC_E()                    // 0x1D - Decrement E
{
    GBC_CPU_DEC(GBC_CPU_Register.E);
}

void GBC_CPU_LD_E_X(uint8_t operand)    // 0x1E - Load 8-bit immediate into E
{
    GBC_CPU_Register.E = operand;
}

void GBC_CPU_RR_A()                     // 0x1F - Rotate A right
{
    uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 0x80 : 0;

    if (GBC_CPU_Register.A & 0x1)
    {
        GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
    }
    else
    {
        GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
    }

    GBC_CPU_Register.A >>= 0x1;
    GBC_CPU_Register.A  |= carry;
}

void GBC_CPU_JR_NZ_X(uint8_t operand)   // 0x20 - Relative jump by signed immediate if last result was not zero
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        int8_t value = operand;
        GBC_CPU_Register.PC += value;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_LD_HL_XX(uint16_t operand) // 0x21 - Load 16-bit immediate into HL
{
    GBC_CPU_Register.HL = operand;
}

void GBC_CPU_LDI_HLP_A()                // 0x22 - Save A to address pointed by HL, and increment HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL++, GBC_CPU_Register.A);
}

void GBC_CPU_INC_HL()                   // 0x23 - Increment 16-bit HL
{
    GBC_CPU_Register.HL++;

    // Flags not affected
}

void GBC_CPU_INC_H()                    // 0x24 - Increment H
{
    GBC_CPU_INC(GBC_CPU_Register.H);
}

void GBC_CPU_DEC_H()                    // 0x25 - Decrement H
{
    GBC_CPU_DEC(GBC_CPU_Register.H);
}

void GBC_CPU_LD_H_X(uint8_t operand)    // 0x26 - Load 8-bit immediate into H
{
    GBC_CPU_Register.H = operand;
}

void GBC_CPU_DA_A()                     // 0x27 - Adjust A for BCD (Binary Coded Decimal) addition
{
    int32_t value = GBC_CPU_Register.A;

    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_SUBTRACTION))
    {
        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY))
        {
            value -= 0x06;

            if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
            {
                value &= 0xFF;
            }
        }

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value -= 0x60;
        }
    }
    else
    {
        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY) || (value & 0xF) > 9)
        {
            value += 0x06;
        }

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) || value > 0x9F)
        {
            value += 0x60;
        }
    }

    if (value & 0x100)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    //else
    {
        // Do not clear carry flag
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);

    // Subtraction flag not affected

    value &= 0xFF;

    if (value)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    GBC_CPU_Register.A = value;
}

void GBC_CPU_JR_Z_X(uint8_t operand)    // 0x28 - Relative jump by signed immediate if last result was zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        int8_t value = operand;
        GBC_CPU_Register.PC += value;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_ADD_HL_HL()                // 0x29 - Add 16-bit HL to HL
{
    GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.HL);
}

void GBC_CPU_LDI_A_HLP()                // 0x2A - Load A from address pointed to by HL, and increment HL
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL++);
}

void GBC_CPU_DEC_HL()                   // 0x2B - Decrement 16-bit HL
{
    GBC_CPU_Register.HL--;

    // No flags affected
}

void GBC_CPU_INC_L()                    // 0x2C - Increment L
{
    GBC_CPU_INC(GBC_CPU_Register.L);
}

void GBC_CPU_DEC_L()                    // 0x2D - Decrement L
{
    GBC_CPU_DEC(GBC_CPU_Register.L);
}

void GBC_CPU_LD_L_X(uint8_t operand)    // 0x2E - Load 8-bit immediate into L
{
    GBC_CPU_Register.L = operand;
}

void GBC_CPU_CPL()                      // 0x2F - Complement (logical NOT) on A (flip all bits)
{
    GBC_CPU_Register.A = ~GBC_CPU_Register.A;

    // Carry flag not affected
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
    // Zero flag not affected
}

void GBC_CPU_JR_NC_X(uint8_t operand)   // 0x30 - Relative jump by signed immediate if last result caused no carry
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        int8_t value = operand;
        GBC_CPU_Register.PC += value;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_LD_SP_XX(uint16_t operand) // 0x31 - Load 16-bit immediate into SP
{
    GBC_CPU_Register.SP = operand;
}

void GBC_CPU_LDD_HLP_A()                // 0x32 - Save A to address pointed by HL, and decrement HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL--, GBC_CPU_Register.A);
}

void GBC_CPU_INC_SP()                   // 0x33 - Increment 16-bit SP
{
    GBC_CPU_Register.SP++;

    // Flags not affected
}

void GBC_CPU_INC_HLP()                  // 0x34 - Increment value pointed by HL
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_INC(value)

    GBC_MMU_WriteByte(GBC_CPU_Register.HL, value);
}

void GBC_CPU_DEC_HLP()                  // 0x35 - Decrement value pointed by HL
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_DEC(value);

    GBC_MMU_WriteByte(GBC_CPU_Register.HL, value);
}

void GBC_CPU_LD_HLP_X(uint8_t operand)  // 0x36 - Load 8-bit immediate into address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, operand);
}

void GBC_CPU_SCF()                      // 0x37 - Set carry flag
{
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
    // Zero flag not affected
}

void GBC_CPU_JR_C_X(uint8_t operand)    // 0x38 - Relative jump by signed immediate if last result caused carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        int8_t value = operand;
        GBC_CPU_Register.PC += value;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_ADD_HL_SP()                // 0x39 - Add 16-bit SP to HL
{
    GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.SP);
}

void GBC_CPU_LDD_A_HLP()                // 0x3A - Load A from address pointed to by HL, and decrement HL
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL--);
}

void GBC_CPU_DEC_SP()                   // 0x3B - Decrement 16-bit SP
{
    GBC_CPU_Register.SP--;

    // No flags affected
}

void GBC_CPU_INC_A()                    // 0x3C - Increment A
{
    GBC_CPU_INC(GBC_CPU_Register.A);
}

void GBC_CPU_DEC_A()                    // 0x3D - Decrement A
{
    GBC_CPU_DEC(GBC_CPU_Register.A);
}

void GBC_CPU_LD_A_X(uint8_t operand)    // 0x3E - Load 8-bit immediate into A
{
    GBC_CPU_Register.A = operand;
}

void GBC_CPU_CCF()                      // 0x3F - Complement carry flag
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
    }

    // Zero flag not affected
}

void GBC_CPU_LD_B_C()                   // 0x41 - Copy C to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.C;
}

void GBC_CPU_LD_B_D()                   // 0x42 - Copy D to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.D;
}

void GBC_CPU_LD_B_E()                   // 0x43 - Copy E to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.E;
}

void GBC_CPU_LD_B_H()                   // 0x44 - Copy H to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.H;
}

void GBC_CPU_LD_B_L()                   // 0x45 - Copy L to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.L;
}

void GBC_CPU_LD_B_HLP()                 // 0x46 - Copy value pointed by HL to B
{
    GBC_CPU_Register.B = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_B_A()                   // 0x47 - Copy A to B
{
    GBC_CPU_Register.B = GBC_CPU_Register.A;
}

void GBC_CPU_LD_C_B()                   // 0x48 - Copy B to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.B;
}

void GBC_CPU_LD_C_D()                   // 0x4A - Copy D to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.D;
}

void GBC_CPU_LD_C_E()                   // 0x4B - Copy E to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.E;
}

void GBC_CPU_LD_C_H()                   // 0x4C - Copy H to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.H;
}

void GBC_CPU_LD_C_L()                   // 0x4D - Copy L to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.L;
}

void GBC_CPU_LD_C_HLP()                 // 0x4E - Copy value pointed by HL to C
{
    GBC_CPU_Register.C = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_C_A()                   // 0x4F - Copy A to C
{
    GBC_CPU_Register.C = GBC_CPU_Register.A;
}

void GBC_CPU_LD_D_B()                   // 0x50 - Copy B to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.B;
}

void GBC_CPU_LD_D_C()                   // 0x51 - Copy C to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.C;
}

void GBC_CPU_LD_D_E()                   // 0x53 - Copy E to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.E;
}

void GBC_CPU_LD_D_H()                   // 0x54 - Copy H to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.H;
}

void GBC_CPU_LD_D_L()                   // 0x55 - Copy L to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.L;
}

void GBC_CPU_LD_D_HLP()                 // 0x56 - Copy value pointed by HL to D
{
    GBC_CPU_Register.D = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_D_A()                   // 0x57 - Copy A to D
{
    GBC_CPU_Register.D = GBC_CPU_Register.A;
}

void GBC_CPU_LD_E_B()                   // 0x58 - Copy B to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.B;
}

void GBC_CPU_LD_E_C()                   // 0x59 - Copy C to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.C;
}

void GBC_CPU_LD_E_D()                   // 0x5A - Copy D to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.D;
}

void GBC_CPU_LD_E_H()                   // 0x5C - Copy H to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.H;
}

void GBC_CPU_LD_E_L()                   // 0x5D - Copy L to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.L;
}

void GBC_CPU_LD_E_HLP()                 // 0x5E - Copy value pointed by HL to E
{
    GBC_CPU_Register.E = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_E_A()                   // 0x5F - Copy A to E
{
    GBC_CPU_Register.E = GBC_CPU_Register.A;
}

void GBC_CPU_LD_H_B()                   // 0x60 - Copy B to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.B;
}

void GBC_CPU_LD_H_C()                   // 0x61 - Copy C to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.C;
}

void GBC_CPU_LD_H_D()                   // 0x62 - Copy D to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.D;
}

void GBC_CPU_LD_H_E()                   // 0x63 - Copy E to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.E;
}

void GBC_CPU_LD_H_L()                   // 0x65 - Copy L to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.L;
}

void GBC_CPU_LD_H_HLP()                 // 0x66 - Copy value pointed by HL to H
{
    GBC_CPU_Register.H = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_H_A()                   // 0x67 - Copy A to H
{
    GBC_CPU_Register.H = GBC_CPU_Register.A;
}

void GBC_CPU_LD_L_B()                   // 0x68 - Copy B to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.B;
}

void GBC_CPU_LD_L_C()                   // 0x69 - Copy C to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.C;
}

void GBC_CPU_LD_L_D()                   // 0x6A - Copy D to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.D;
}

void GBC_CPU_LD_L_E()                   // 0x6B - Copy E to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.E;
}

void GBC_CPU_LD_L_H()                   // 0x6C - Copy H to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.H;
}

void GBC_CPU_LD_L_HLP()                 // 0x6E - Copy value pointed by HL to L
{
    GBC_CPU_Register.L = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_LD_L_A()                   // 0x6F - Copy A to L
{
    GBC_CPU_Register.L = GBC_CPU_Register.A;
}

void GBC_CPU_LD_HLP_B()                 // 0x70 - Copy B to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.B);
}

void GBC_CPU_LD_HLP_C()                 // 0x71 - Copy C to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.C);
}

void GBC_CPU_LD_HLP_D()                 // 0x72 - Copy D to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.D);
}

void GBC_CPU_LD_HLP_E()                 // 0x73 - Copy E to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.E);
}

void GBC_CPU_LD_HLP_H()                 // 0x74 - Copy H to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.H);
}

void GBC_CPU_LD_HLP_L()                 // 0x75 - Copy L to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.L);
}

void GBC_CPU_HALT()                     // 0x76 - Halt processor
{
    if (GBC_CPU_InterruptMasterEnableDelayTicks)
    {
        // Serve pending interrupts before HALT
        GBC_CPU_InterruptMasterEnableDelayTicks = 0;
        GBC_CPU_InterruptMasterEnable = true;
        GBC_CPU_Register.PC--;
    }
    else
    {
        GBC_CPU_Halted = true;
    }
}

void GBC_CPU_LD_HLP_A()                 // 0x77 - Copy A to address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.A);
}

void GBC_CPU_LD_A_B()                   // 0x78 - Copy B to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.B;
}

void GBC_CPU_LD_A_C()                   // 0x79 - Copy C to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.C;
}

void GBC_CPU_LD_A_D()                   // 0x7A - Copy D to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.D;
}

void GBC_CPU_LD_A_E()                   // 0x7B - Copy E to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.E;
}

void GBC_CPU_LD_A_H()                   // 0x7C - Copy H to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.H;
}

void GBC_CPU_LD_A_L()                   // 0x7D - Copy L to A
{
    GBC_CPU_Register.A = GBC_CPU_Register.L;
}

void GBC_CPU_LD_A_HLP()                 // 0x7E - Copy value pointed by HL to A
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL);
}

void GBC_CPU_ADD_A_B()                  // 0x80 - Add B to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_ADD_A_C()                  // 0x81 - Add C to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_ADD_A_D()                  // 0x82 - Add D to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_ADD_A_E()                  // 0x83 - Add E to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_ADD_A_H()                  // 0x84 - Add H to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_ADD_A_L()                  // 0x85 - Add L to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_ADD_A_HLP()                // 0x86 - Add value pointed by HL to A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_ADD(GBC_CPU_Register.A, value);
}

void GBC_CPU_ADD_A_A()                  // 0x87 - Add A to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_ADC_A_B()                  // 0x88 - Add B and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_ADC_A_C()                  // 0x89 - Add C and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_ADC_A_D()                  // 0x8A - Add D and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_ADC_A_E()                  // 0x8B - Add E and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_ADC_A_H()                  // 0x8C - Add H and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_ADC_A_L()                  // 0x8D - Add L and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_ADC_A_HLP()                // 0x8E - Add value pointed by HL and carry flag to A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_ADC(GBC_CPU_Register.A, value);
}

void GBC_CPU_ADC_A_A()                  // 0x8F - Add A and carry flag to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_SUB_A_B()                  // 0x90 - Subtract B from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_SUB_A_C()                  // 0x91 - Subtract C from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_SUB_A_D()                  // 0x92 - Subtract D from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_SUB_A_E()                  // 0x93 - Subtract E from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_SUB_A_H()                  // 0x94 - Subtract H from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_SUB_A_L()                  // 0x95 - Subtract L from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_SUB_A_HLP()                // 0x96 - Subtract value pointed by HL from A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_SUB(GBC_CPU_Register.A, value);
}

void GBC_CPU_SUB_A_A()                  // 0x97 - Subtract A from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_SBC_A_B()                  // 0x98 - Subtract B and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_SBC_A_C()                  // 0x99 - Subtract C and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_SBC_A_D()                  // 0x9A - Subtract D and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_SBC_A_E()                  // 0x9B - Subtract E and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_SBC_A_H()                  // 0x9C - Subtract H and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_SBC_A_L()                  // 0x9D - Subtract L and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_SBC_A_HLP()                // 0x9E - Subtract value pointed by HL and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_SBC_A_A()                  // 0x9F - Subtract A and carry flag from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_AND_A_B()                  // 0xA0 - Logical AND B against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_AND_A_C()                  // 0xA1 - Logical AND C against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_AND_A_D()                  // 0xA2 - Logical AND D against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_AND_A_E()                  // 0xA3 - Logical AND E against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_AND_A_H()                  // 0xA4 - Logical AND H against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_AND_A_L()                  // 0xA5 - Logical AND L against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_AND_A_HLP()                // 0xA6 - Logical AND value pointed by HL against A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_AND(GBC_CPU_Register.A, value);
}

void GBC_CPU_AND_A_A()                  // 0xA7 - Logical AND A against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_XOR_A_B()                  // 0xA8 - Logical XOR B against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_XOR_A_C()                  // 0xA9 - Logical XOR C against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_XOR_A_D()                  // 0xAA - Logical XOR D against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_XOR_A_E()                  // 0xAB - Logical XOR E against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_XOR_A_H()                  // 0xAD - Logical XOR H against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_XOR_A_L()                  // 0xAD - Logical XOR L against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_XOR_A_HLP()                // 0xAE - Logical XOR value pointed by HL against A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_XOR(GBC_CPU_Register.A, value);
}

void GBC_CPU_XOR_A_A()                  // 0xAF - Logical XOR A against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_OR_A_B()                   // 0xB0 - Logical OR B against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_OR_A_C()                   // 0xB1 - Logical OR C against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_OR_A_D()                   // 0xB2 - Logical OR D against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_OR_A_E()                   // 0xB3 - Logical OR E against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_OR_A_H()                   // 0xB4 - Logical OR H against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_OR_A_L()                   // 0xB5 - Logical OR L against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_OR_A_HLP()                 // 0xB6 - Logical OR value pointed by HL against A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_OR(GBC_CPU_Register.A, value);
}

void GBC_CPU_OR_A_A()                   // 0xB7 - Logical OR A against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_CP_A_B()                   // 0xB8 - Compare B against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_CP_A_C()                   // 0xB9 - Compare C against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_CP_A_D()                   // 0xBA - Compare D against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_CP_A_E()                   // 0xBB - Compare E against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_CP_A_H()                   // 0xBC - Compare H against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_CP_A_L()                   // 0xBD - Compare L against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_CP_A_HLP()                 // 0xBE - Compare value pointed by HL against A
{
    uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

    GBC_CPU_COMPARE(GBC_CPU_Register.A, value);
}

void GBC_CPU_CP_A_A()                   // 0xBF - Compare A against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_RET_NZ()                   // 0xC0 - Return if last result was not zero
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_POP_BC()                   // 0xC1 - Pop 16-bit value from stack into BC
{
    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.BC);
}

void GBC_CPU_JP_NZ_XX(uint16_t operand) // 0xC2 - Absolute jump to 16-bit location if last result was not zero
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_JP_XX(uint16_t operand)    // 0xC3 - Absolute jump to 16-bit location
{
    GBC_CPU_Register.PC = operand;
}

void GBC_CPU_CALL_NZ_XX(uint16_t operand)   // 0xC4 - Call routine at 16-bit location if last result was not zero
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_PUSH_BC()                  // 0xC5 - Push 16-bit BC onto stack
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.BC);
}

void GBC_CPU_ADD_A_X(uint8_t operand)   // 0xC6 - Add 8-bit immediate to A
{
    GBC_CPU_ADD(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_0H()                   // 0xC7 - Call routine at address 0000h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0;
}

void GBC_CPU_RET_Z()                    // 0xC8 - Return if last result was zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_RET()                      // 0xC9 - Return to calling routine
{
    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
}

void GBC_CPU_JP_Z_XX(uint16_t operand)  // 0xCA - Absolute jump to 16-bit location if last result was zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_EXT_OPS(uint8_t operand)   // 0xCB - Extended operations (two-byte instruction code)
{
    GBC_CPU_EX_Instruction_t instruction = GBC_CPU_EX_Instructions[operand];    // Get extended instruction

    ((void (*)(void))instruction.Handler)();                                    // Execute extended instruction

    GBC_CPU_InstructionTicks += instruction.Ticks;                              // Add extended instruction ticks
}

void GBC_CPU_CALL_Z_XX(uint16_t operand)// 0xCC - Call routine at 16-bit location if last result was zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_CALL_XX(uint16_t operand)  // 0xCD - Call routine at 16-bit location
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = operand;
}

void GBC_CPU_ADC_A_X(uint8_t operand)   // 0xCE - Add 8-bit immediate and carry to A
{
    GBC_CPU_ADC(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_8H()                   // 0xCF - Call routine at address 0008h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x8;
}

void GBC_CPU_RET_NC()                   // 0xD0 - Return if last result caused no carry
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_POP_DE()                   // 0xD1 - Pop 16-bit value from stack into DE
{
    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.DE);
}

void GBC_CPU_JP_NC_XX(uint16_t operand) // 0xD2 - Absolute jump to 16-bit location if last result caused no carry
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_CALL_NC_XX(uint16_t operand)// 0xD4 - Call routine at 16-bit location if last result caused no carry
{
    if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_PUSH_DE()                  // 0xD5 - Push 16-bit DE onto stack
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.DE);
}

void GBC_CPU_SUB_A_X(uint8_t operand)   // 0xD6 - Subtract 8-bit immediate from A
{
    GBC_CPU_SUB(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_10H()                  // 0xD7 - Call routine at address 0010h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x10;
}

void GBC_CPU_RET_C()                    // 0xD8 - Return if last result caused carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_RETI()                     // 0xD9 - Enable interrupts and return to calling routine
{
    GBC_CPU_InterruptMasterEnable = true;

    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
}

void GBC_CPU_JP_C_XX(uint16_t operand)  // 0xDA - Absolute jump to 16-bit location if last result caused carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 4;
    }
}

void GBC_CPU_CALL_C_XX(uint16_t operand)// 0xDC - Call routine at 16-bit location if last result caused carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
        GBC_CPU_Register.PC = operand;
        GBC_CPU_InstructionTicks += 12;
    }
}

void GBC_CPU_SBC_A_X(uint8_t operand)   // 0xDE - Subtract 8-bit immediate and carry from A
{
    GBC_CPU_SBC(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_18H()                  // 0xDF - Call routine at address 0018h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x18;
}

void GBC_CPU_LDH_XP_A(uint8_t operand)  // 0xE0 - Save A at address pointed to by (FF00h + 8-bit immediate)
{
    GBC_MMU_WriteByte(0xFF00 + operand, GBC_CPU_Register.A);
}

void GBC_CPU_POP_HL()                   // 0xE1 - Pop 16-bit value from stack into HL
{
    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.HL);
}

void GBC_CPU_LDH_CP_A()                 // 0xE2 - Save A at address pointed to by (FF00h + C)
{
    GBC_MMU_WriteByte(0xFF00 + GBC_CPU_Register.C, GBC_CPU_Register.A);
}

void GBC_CPU_PUSH_HL()                  // 0xE5 - Push 16-bit HL onto stack
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.HL);
}

void GBC_CPU_AND_A_X(uint8_t operand)   // 0xE6 - Logical AND 8-bit value immediate against A
{
    GBC_CPU_AND(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_20H()                  // 0xE7 - Call routine at address 0020h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x20;
}

void GBC_CPU_ADD_SP_X(uint8_t operand)  // 0xE8 - Add signed 8-bit immediate to SP
{
    int8_t value = operand;
    int32_t result = GBC_CPU_Register.SP + value;

    if ((GBC_CPU_Register.SP & 0xFF) + (operand & 0xFF) > 0xFF)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    if ((GBC_CPU_Register.SP & 0x0F) + (value & 0x0F) > 0x0F)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    // GBC_CPU_FLAGS_ZERO is cleared according to documentation
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);

    result &= 0xFFFF;

    GBC_CPU_Register.SP = result;
}

void GBC_CPU_JP_HL()                    // 0xE9 - Jump to HL
{
    GBC_CPU_Register.PC = GBC_CPU_Register.HL;
}

void GBC_CPU_LD_XXP_A(uint16_t operand) // 0xEA - Save A at given 16-bit address
{
    GBC_MMU_WriteByte(operand, GBC_CPU_Register.A);
}

void GBC_CPU_XOR_A_X(uint8_t operand)   // 0xEE - Logical XOR 8-bit value immediate against A
{
    GBC_CPU_XOR(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_28H()                  // 0xEF - Call routine at address 0028h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x28;
}

void GBC_CPU_LDH_A_XP(uint8_t operand)  // 0xF0 - Load A from address pointed to by (FF00h + 8-bit immediate)
{
    /*if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
    {
        GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
        return;
    }*/

    GBC_CPU_Register.A = GBC_MMU_ReadByte(0xFF00 + operand);
}

void GBC_CPU_POP_AF()                   // 0xF1 - Pop 16-bit value from stack into AF
{
    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.AF);
    GBC_CPU_Register.AF = GBC_CPU_Register.AF & 0xFFF0;
}

void GBC_CPU_LDH_A_CP()                 // 0xF2 - Load A from address pointed to by (FF00h + C)
{
    GBC_CPU_Register.A = GBC_MMU_ReadByte(0xFF00 + GBC_CPU_Register.C);
}

void GBC_CPU_DI()                       // 0xF3 - Disable Interrupts
{
    GBC_CPU_InterruptMasterEnableDelayTicks = 0;
    GBC_CPU_InterruptMasterEnable = false;
}

void GBC_CPU_PUSH_AF()                  // 0xF5 - Push 16-bit AF onto stack
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.AF);
}

void GBC_CPU_OR_A_X(uint8_t operand)    // 0xF6 - Logical OR 8-bit value immediate against A
{
    GBC_CPU_OR(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_30H()                  // 0xF7 - Call routine at address 0030h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x30;
}

void GBC_CPU_LDHL_SP_X(uint8_t operand) // 0xF8 - Add signed 8-bit immediate to SP and save result in HL
{
    int8_t value = operand;
    int32_t result = GBC_CPU_Register.SP + value;

    if ((GBC_CPU_Register.SP & 0xFF) + (operand & 0xFF) > 0xFF)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    if ((GBC_CPU_Register.SP & 0x0F) + (value & 0x0F) > 0x0F)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    // GBC_CPU_FLAGS_ZERO is cleared according to documentation
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);

    result &= 0xFFFF;

    GBC_CPU_Register.HL = result;
}

void GBC_CPU_LD_SP_HL()                 // 0xF9 - Copy HL to SP
{
    GBC_CPU_Register.SP = GBC_CPU_Register.HL;
}

void GBC_CPU_LD_A_XXP(uint16_t operand) // 0xFA - Load A from given 16-bit address
{
    /*if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
    {
        GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
        return;
    }*/

    GBC_CPU_Register.A = GBC_MMU_ReadByte(operand);
}

void GBC_CPU_EI()                       // 0xFB - Enable Interrupts
{
    GBC_CPU_InterruptMasterEnableDelayTicks = GBC_CPU_Instructions[0xFB].Ticks + 1;
}

void GBC_CPU_CP_A_X(uint8_t operand)    // 0xFE - Compare 8-bit value immediate against A
{
    GBC_CPU_COMPARE(GBC_CPU_Register.A, operand);
}

void GBC_CPU_RST_38H()                  // 0xFF - Call routine at address 0038h
{
    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x38;
}

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_Instruction_t GBC_CPU_Instructions[256] =
{
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x00 - No operation
    { GBC_CPU_LD_BC_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0x01 - Load 16-bit immediate into BC
    { GBC_CPU_LD_BCP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x02 - Save A to address pointed by BC
    { GBC_CPU_INC_BC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x03 - Increment 16-bit BC
    { GBC_CPU_INC_B,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x04 - Increment B
    { GBC_CPU_DEC_B,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x05 - Decrement B
    { GBC_CPU_LD_B_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x06 - Load 8-bit immediate into B
    { GBC_CPU_RLC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x07 - Rotate A left with carry
    { GBC_CPU_LD_XXP_SP, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_20 }, // 0x08 - Save SP to given address
    { GBC_CPU_ADD_HL_BC, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x09 - Add 16-bit BC to HL
    { GBC_CPU_LD_A_BCP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x0A - Load A from address pointed to by BC
    { GBC_CPU_DEC_BC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x0B - Decrement 16-bit BC
    { GBC_CPU_INC_C,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0C - Increment C
    { GBC_CPU_DEC_C,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0D - Decrement C
    { GBC_CPU_LD_C_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x0E - Load 8-bit immediate into C
    { GBC_CPU_RRC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0F - Rotate A right with carry
    { GBC_CPU_STOP,      GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0x10 - Stop processor
    { GBC_CPU_LD_DE_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0x11 - Load 16-bit immediate into DE
    { GBC_CPU_LD_DEP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x12 - Save A to address pointed by DE
    { GBC_CPU_INC_DE,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x13 - Increment 16-bit DE
    { GBC_CPU_INC_D,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x14 - Increment D
    { GBC_CPU_DEC_D,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x15 - Decrement D
    { GBC_CPU_LD_D_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x16 - Load 8-bit immediate into D
    { GBC_CPU_RL_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x17 - Rotate A left
    { GBC_CPU_JR_X,      GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_12 }, // 0x18 - Relative jump by signed immediate
    { GBC_CPU_ADD_HL_DE, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x19 - Add 16-bit DE to HL
    { GBC_CPU_LD_A_DEP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x1A - Load A from address pointed to by DE
    { GBC_CPU_DEC_DE,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x1B - Decrement 16-bit DE
    { GBC_CPU_INC_E,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1C - Increment E
    { GBC_CPU_DEC_E,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1D - Decrement E
    { GBC_CPU_LD_E_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x1E - Load 8-bit immediate into E
    { GBC_CPU_RR_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1F - Rotate A right
    { GBC_CPU_JR_NZ_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x20 - Relative jump by signed immediate if last result was not zero
    { GBC_CPU_LD_HL_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0x21 - Load 16-bit immediate into HL
    { GBC_CPU_LDI_HLP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x22 - Save A to address pointed by HL, and increment HL
    { GBC_CPU_INC_HL,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x23 - Increment 16-bit HL
    { GBC_CPU_INC_H,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x24 - Increment H
    { GBC_CPU_DEC_H,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x25 - Decrement H
    { GBC_CPU_LD_H_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x26 - Load 8-bit immediate into H
    { GBC_CPU_DA_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x27 - Adjust A for BCD (Binary Coded Decimal) addition
    { GBC_CPU_JR_Z_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x28 - Relative jump by signed immediate if last result was zero
    { GBC_CPU_ADD_HL_HL, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x29 - Add 16-bit HL to HL
    { GBC_CPU_LDI_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x2A - Load A from address pointed to by HL, and increment HL
    { GBC_CPU_DEC_HL,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x2B - Decrement 16-bit HL
    { GBC_CPU_INC_L,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x2C - Increment L
    { GBC_CPU_DEC_L,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x2D - Decrement L
    { GBC_CPU_LD_L_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x2E - Load 8-bit immediate into L
    { GBC_CPU_CPL,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x2F - Complement (logical NOT) on A (flip all bits)
    { GBC_CPU_JR_NC_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x30 - Relative jump by signed immediate if last result caused no carry
    { GBC_CPU_LD_SP_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0x31 - Load 16-bit immediate into SP
    { GBC_CPU_LDD_HLP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x32 - Save A to address pointed by HL, and decrement HL
    { GBC_CPU_INC_SP,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x33 - Increment 16-bit SP
    { GBC_CPU_INC_HLP,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0x34 - Increment value pointed by HL
    { GBC_CPU_DEC_HLP,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0x35 - Decrement value pointed by HL
    { GBC_CPU_LD_HLP_X,  GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_12 }, // 0x36 - Load 8-bit immediate into address pointed by HL
    { GBC_CPU_SCF,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x37 - Set carry flag
    { GBC_CPU_JR_C_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x38 - Relative jump by signed immediate if last result caused carry
    { GBC_CPU_ADD_HL_SP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x39 - Add 16-bit SP to HL
    { GBC_CPU_LDD_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x3A - Load A from address pointed to by HL, and decrement HL
    { GBC_CPU_DEC_SP,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x3B - Decrement 16-bit SP
    { GBC_CPU_INC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x3C - Increment A
    { GBC_CPU_DEC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x3D - Decrement A
    { GBC_CPU_LD_A_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0x3E - Load 8-bit immediate into A
    { GBC_CPU_CCF,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x3F - Complement carry flag
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x40 - Copy B to B
    { GBC_CPU_LD_B_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x41 - Copy C to B
    { GBC_CPU_LD_B_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x42 - Copy D to B
    { GBC_CPU_LD_B_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x43 - Copy E to B
    { GBC_CPU_LD_B_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x44 - Copy H to B
    { GBC_CPU_LD_B_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x45 - Copy L to B
    { GBC_CPU_LD_B_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x46 - Copy value pointed by HL to B
    { GBC_CPU_LD_B_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x47 - Copy A to B
    { GBC_CPU_LD_C_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x48 - Copy B to C
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x49 - Copy C to C
    { GBC_CPU_LD_C_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4A - Copy D to C
    { GBC_CPU_LD_C_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4B - Copy E to C
    { GBC_CPU_LD_C_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4C - Copy H to C
    { GBC_CPU_LD_C_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4D - Copy L to C
    { GBC_CPU_LD_C_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x4E - Copy value pointed by HL to C
    { GBC_CPU_LD_C_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4F - Copy A to C
    { GBC_CPU_LD_D_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x50 - Copy B to D
    { GBC_CPU_LD_D_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x51 - Copy C to D
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x52 - Copy D to D
    { GBC_CPU_LD_D_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x53 - Copy E to D
    { GBC_CPU_LD_D_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x54 - Copy H to D
    { GBC_CPU_LD_D_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x55 - Copy L to D
    { GBC_CPU_LD_D_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x56 - Copy value pointed by HL to D
    { GBC_CPU_LD_D_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x57 - Copy A to D
    { GBC_CPU_LD_E_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x58 - Copy B to E
    { GBC_CPU_LD_E_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x59 - Copy C to E
    { GBC_CPU_LD_E_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5A - Copy D to E
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5B - Copy E to E
    { GBC_CPU_LD_E_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5C - Copy H to E
    { GBC_CPU_LD_E_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5D - Copy L to E
    { GBC_CPU_LD_E_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x5E - Copy value pointed by HL to E
    { GBC_CPU_LD_E_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5F - Copy A to E
    { GBC_CPU_LD_H_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x60 - Copy B to H
    { GBC_CPU_LD_H_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x61 - Copy C to H
    { GBC_CPU_LD_H_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x62 - Copy D to H
    { GBC_CPU_LD_H_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x63 - Copy E to H
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x64 - Copy H to H
    { GBC_CPU_LD_H_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x65 - Copy L to H
    { GBC_CPU_LD_H_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x66 - Copy value pointed by HL to H
    { GBC_CPU_LD_H_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x67 - Copy A to H
    { GBC_CPU_LD_L_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x68 - Copy B to L
    { GBC_CPU_LD_L_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x69 - Copy C to L
    { GBC_CPU_LD_L_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6A - Copy D to L
    { GBC_CPU_LD_L_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6B - Copy E to L
    { GBC_CPU_LD_L_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6C - Copy H to L
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6D - Copy L to L
    { GBC_CPU_LD_L_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x6E - Copy value pointed by HL to L
    { GBC_CPU_LD_L_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6F - Copy A to L
    { GBC_CPU_LD_HLP_B,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x70 - Copy B to address pointed by HL
    { GBC_CPU_LD_HLP_C,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x71 - Copy C to address pointed by HL
    { GBC_CPU_LD_HLP_D,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x72 - Copy D to address pointed by HL
    { GBC_CPU_LD_HLP_E,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x73 - Copy E to address pointed by HL
    { GBC_CPU_LD_HLP_H,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x74 - Copy H to address pointed by HL
    { GBC_CPU_LD_HLP_L,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x75 - Copy L to address pointed by HL
    { GBC_CPU_HALT,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0x76 - Halt processor
    { GBC_CPU_LD_HLP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x77 - Copy A to address pointed by HL
    { GBC_CPU_LD_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x78 - Copy B to A
    { GBC_CPU_LD_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x79 - Copy C to A
    { GBC_CPU_LD_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7A - Copy D to A
    { GBC_CPU_LD_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7B - Copy E to A
    { GBC_CPU_LD_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7C - Copy H to A
    { GBC_CPU_LD_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7D - Copy L to A
    { GBC_CPU_LD_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x7E - Copy value pointed by HL to A
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7F - Copy A to A
    { GBC_CPU_ADD_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x80 - Add B to A
    { GBC_CPU_ADD_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x81 - Add C to A
    { GBC_CPU_ADD_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x82 - Add D to A
    { GBC_CPU_ADD_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x83 - Add E to A
    { GBC_CPU_ADD_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x84 - Add H to A
    { GBC_CPU_ADD_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x85 - Add L to A
    { GBC_CPU_ADD_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x86 - Add value pointed by HL to A
    { GBC_CPU_ADD_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x87 - Add A to A
    { GBC_CPU_ADC_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x88 - Add B and carry flag to A
    { GBC_CPU_ADC_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x89 - Add C and carry flag to A
    { GBC_CPU_ADC_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8A - Add D and carry flag to A
    { GBC_CPU_ADC_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8B - Add E and carry flag to A
    { GBC_CPU_ADC_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8C - Add H and carry flag to A
    { GBC_CPU_ADC_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8D - Add L and carry flag to A
    { GBC_CPU_ADC_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x8E - Add value pointed by HL and carry flag to A
    { GBC_CPU_ADC_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8F - Add A and carry flag to A
    { GBC_CPU_SUB_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x90 - Subtract B from A
    { GBC_CPU_SUB_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x91 - Subtract C from A
    { GBC_CPU_SUB_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x92 - Subtract D from A
    { GBC_CPU_SUB_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x93 - Subtract E from A
    { GBC_CPU_SUB_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x94 - Subtract H from A
    { GBC_CPU_SUB_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x95 - Subtract L from A
    { GBC_CPU_SUB_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x96 - Subtract value pointed by HL from A
    { GBC_CPU_SUB_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x97 - Subtract A from A
    { GBC_CPU_SBC_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x98 - Subtract B and carry flag from A
    { GBC_CPU_SBC_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x99 - Subtract C and carry flag from A
    { GBC_CPU_SBC_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9A - Subtract D and carry flag from A
    { GBC_CPU_SBC_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9B - Subtract E and carry flag from A
    { GBC_CPU_SBC_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9C - Subtract H and carry flag from A
    { GBC_CPU_SBC_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9D - Subtract L and carry flag from A
    { GBC_CPU_SBC_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0x9E - Subtract value pointed by HL and carry flag from A
    { GBC_CPU_SBC_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9F - Subtract A and carry flag from A
    { GBC_CPU_AND_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA0 - Logical AND B against A
    { GBC_CPU_AND_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA1 - Logical AND C against A
    { GBC_CPU_AND_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA2 - Logical AND D against A
    { GBC_CPU_AND_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA3 - Logical AND E against A
    { GBC_CPU_AND_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA4 - Logical AND H against A
    { GBC_CPU_AND_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA5 - Logical AND L against A
    { GBC_CPU_AND_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xA6 - Logical AND value pointed by HL against A
    { GBC_CPU_AND_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA7 - Logical AND A against A
    { GBC_CPU_XOR_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA8 - Logical XOR B against A
    { GBC_CPU_XOR_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA9 - Logical XOR C against A
    { GBC_CPU_XOR_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAA - Logical XOR D against A
    { GBC_CPU_XOR_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAB - Logical XOR E against A
    { GBC_CPU_XOR_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAC - Logical XOR H against A
    { GBC_CPU_XOR_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAD - Logical XOR L against A
    { GBC_CPU_XOR_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xAE - Logical XOR value pointed by HL against A
    { GBC_CPU_XOR_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAF - Logical XOR A against A
    { GBC_CPU_OR_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB0 - Logical OR B against A
    { GBC_CPU_OR_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB1 - Logical OR C against A
    { GBC_CPU_OR_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB2 - Logical OR D against A
    { GBC_CPU_OR_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB3 - Logical OR E against A
    { GBC_CPU_OR_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB4 - Logical OR H against A
    { GBC_CPU_OR_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB5 - Logical OR L against A
    { GBC_CPU_OR_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xB6 - Logical OR value pointed by HL against A
    { GBC_CPU_OR_A_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB7 - Logical OR A against A
    { GBC_CPU_CP_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB8 - Compare B against A
    { GBC_CPU_CP_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB9 - Compare C against A
    { GBC_CPU_CP_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBA - Compare D against A
    { GBC_CPU_CP_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBB - Compare E against A
    { GBC_CPU_CP_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBC - Compare H against A
    { GBC_CPU_CP_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBD - Compare L against A
    { GBC_CPU_CP_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xBE - Compare value pointed by HL against A
    { GBC_CPU_CP_A_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBF - Compare A against A
    { GBC_CPU_RET_NZ,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xC0 - Return if last result was not zero
    { GBC_CPU_POP_BC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0xC1 - Pop 16-bit value from stack into BC
    { GBC_CPU_JP_NZ_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xC2 - Absolute jump to 16-bit location if last result was not zero
    { GBC_CPU_JP_XX,     GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_16 }, // 0xC3 - Absolute jump to 16-bit location
    { GBC_CPU_CALL_NZ_XX,GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xC4 - Call routine at 16-bit location if last result was not zero
    { GBC_CPU_PUSH_BC,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xC5 - Push 16-bit BC onto stack
    { GBC_CPU_ADD_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xC6 - Add 8-bit immediate to A
    { GBC_CPU_RST_0H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xC7 - Call routine at address 0000h
    { GBC_CPU_RET_Z,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xC8 - Return if last result was zero
    { GBC_CPU_RET,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xC9 - Return to calling routine
    { GBC_CPU_JP_Z_XX,   GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xCA - Absolute jump to 16-bit location if last result was zero
    { GBC_CPU_EXT_OPS,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0xCB - Extended operations (two-byte instruction code)
    { GBC_CPU_CALL_Z_XX, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xCC - Call routine at 16-bit location if last result was zero
    { GBC_CPU_CALL_XX,   GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_24 }, // 0xCD - Call routine at 16-bit location
    { GBC_CPU_ADC_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xCE - Add 8-bit immediate and carry to A
    { GBC_CPU_RST_8H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xCF - Call routine at address 0008h
    { GBC_CPU_RET_NC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xD0 - Return if last result caused no carry
    { GBC_CPU_POP_DE,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0xD1 - Pop 16-bit value from stack into DE
    { GBC_CPU_JP_NC_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xD2 - Absolute jump to 16-bit location if last result caused no carry
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xD3 - Operation removed in this CPU
    { GBC_CPU_CALL_NC_XX,GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xD4 - Call routine at 16-bit location if last result caused no carry
    { GBC_CPU_PUSH_DE,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xD5 - Push 16-bit DE onto stack
    { GBC_CPU_SUB_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xD6 - Subtract 8-bit immediate from A
    { GBC_CPU_RST_10H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xD7 - Call routine at address 0010h
    { GBC_CPU_RET_C,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xD8 - Return if last result caused carry
    { GBC_CPU_RETI,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xD9 - Enable interrupts and return to calling routine
    { GBC_CPU_JP_C_XX,   GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xDA - Absolute jump to 16-bit location if last result caused carry
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xDB - Operation removed in this CPU
    { GBC_CPU_CALL_C_XX, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_12 }, // 0xDC - Call routine at 16-bit location if last result caused carry
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xDD - Operation removed in this CPU
    { GBC_CPU_SBC_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xDE - Subtract 8-bit immediate and carry from A
    { GBC_CPU_RST_18H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xDF - Call routine at address 0018h
    { GBC_CPU_LDH_XP_A,  GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_12 }, // 0xE0 - Save A at address pointed to by (FF00h + 8-bit immediate)
    { GBC_CPU_POP_HL,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0xE1 - Pop 16-bit value from stack into HL
    { GBC_CPU_LDH_CP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xE2 - Save A at address pointed to by (FF00h + C)
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xE3 - Operation removed in this CPU
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xE4 - Operation removed in this CPU
    { GBC_CPU_PUSH_HL,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xE5 - Push 16-bit HL onto stack
    { GBC_CPU_AND_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xE6 - Logical AND 8-bit value immediate against A
    { GBC_CPU_RST_20H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xE7 - Call routine at address 0020h
    { GBC_CPU_ADD_SP_X,  GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_16 }, // 0xE8 - Add signed 8-bit immediate to SP
    { GBC_CPU_JP_HL,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xE9 - Jump to HL
    { GBC_CPU_LD_XXP_A,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_16 }, // 0xEA - Save A at given 16-bit address
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xEB - Operation removed in this CPU
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xEC - Operation removed in this CPU
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xED - Operation removed in this CPU
    { GBC_CPU_XOR_A_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xEE - Logical XOR 8-bit value immediate against A
    { GBC_CPU_RST_28H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xEF - Call routine at address 0028h
    { GBC_CPU_LDH_A_XP,  GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_12 }, // 0xF0 - Load A from address pointed to by (FF00h + 8-bit immediate)
    { GBC_CPU_POP_AF,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_12 }, // 0xF1 - Pop 16-bit value from stack into AF
    { GBC_CPU_LDH_A_CP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xF2 - Load A from address pointed to by (FF00h + C)
    { GBC_CPU_DI,        GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xF3 - Disable Interrupts
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xF4 - Operation removed in this CPU
    { GBC_CPU_PUSH_AF,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xF5 - Push 16-bit AF onto stack
    { GBC_CPU_OR_A_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xF6 - Logical OR 8-bit value immediate against A
    { GBC_CPU_RST_30H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xF7 - Call routine at address 0030h
    { GBC_CPU_LDHL_SP_X, GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_12 }, // 0xF8 - Add signed 8-bit immediate to SP and save result in HL
    { GBC_CPU_LD_SP_HL,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_8  }, // 0xF9 - Copy HL to SP
    { GBC_CPU_LD_A_XXP,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_16 }, // 0xFA - Load A from given 16-bit address
    { GBC_CPU_EI,        GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xFB - Enable Interrupts
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xFC - Operation removed in this CPU
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_0  }, // 0xFD - Operation removed in this CPU
    { GBC_CPU_CP_A_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_8  }, // 0xFE - Compare 8-bit value immediate against A
    { GBC_CPU_RST_38H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_16 }, // 0xFF - Call routine at address 0038h
};

void GBC_CPU_Initialize()
{
    memset(&GBC_CPU_Register, 0, sizeof(GBC_CPU_Register_t));

    // Initial value in register A is used for GBC detection
    if (GBC_MMU_IS_CGB_MODE())
    {
        GBC_CPU_Register.AF = 0x11B0;
    }
    else
    {
        GBC_CPU_Register.AF = 0x01B0;
    }

    GBC_CPU_Register.BC = 0x0013;
    GBC_CPU_Register.DE = 0x00D8;
    GBC_CPU_Register.HL = 0x014D;

    GBC_CPU_Register.PC = 0x0100;
    GBC_CPU_Register.SP = 0xFFFE;

    GBC_CPU_PendingInterrupts = 0;
    GBC_CPU_InstructionAddress = 0;
    GBC_CPU_InstructionOpcode = 0;
    GBC_CPU_InstructionTicks = 0;
    GBC_CPU_StepTicks = 0;
    GBC_CPU_UnhaltTicks = 0;
    GBC_CPU_InterruptMasterEnableDelayTicks = 0;
    GBC_CPU_InterruptMasterEnable = true;
    GBC_CPU_Halted = false;
    GBC_CPU_SpeedModifier = 0;
    //GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE;
}

void GBC_CPU_RST_40H()      // Start VBlank Handler
{
    GBC_CPU_InterruptMasterEnable = false;

    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x40;

    GBC_CPU_InstructionTicks += 20;
}

void GBC_CPU_RST_48H()      // Start LCD-Stat Handler
{
    GBC_CPU_InterruptMasterEnable = false;

    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x48;

    GBC_CPU_InstructionTicks += 20;
}

void GBC_CPU_RST_50H()      // Start Timer Handler
{
    GBC_CPU_InterruptMasterEnable = false;

    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x50;

    GBC_CPU_InstructionTicks += 20;
}

void GBC_CPU_RST_58H()      // Start Serial Handler
{
    GBC_CPU_InterruptMasterEnable = false;

    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x58;

    GBC_CPU_InstructionTicks += 20;
}

void GBC_CPU_RST_60H()      // Start Joypad Handler
{
    GBC_CPU_InterruptMasterEnable = false;

    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
    GBC_CPU_Register.PC = 0x60;

    GBC_CPU_InstructionTicks += 20;
}

void GBC_CPU_Step()
{
    GBC_CPU_PendingInterrupts = GBC_MMU_Memory.InterruptEnable & GBC_MMU_Memory.InterruptFlags;
    GBC_CPU_InstructionTicks = 0;
    GBC_CPU_StepTicks = 0;

    if (GBC_CPU_Halted)
    {
        GBC_CPU_StepTicks += 4;

        if (GBC_CPU_UnhaltTicks != 0)
        {
            GBC_CPU_UnhaltTicks -= 4;

            if (GBC_CPU_UnhaltTicks <= 0)
            {
                GBC_CPU_UnhaltTicks = 0;
                GBC_CPU_Halted = false;
            }
        }
        else if (GBC_CPU_PendingInterrupts)
        {
            GBC_CPU_UnhaltTicks = 12;
        }
    }
    else // not in HALT mode
    {
        // Interrupt handling
        if (GBC_CPU_InterruptMasterEnable && GBC_CPU_PendingInterrupts /*&& GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE*/)
        {
            if (GBC_CPU_PendingInterrupts & GBC_MMU_INTERRUPT_FLAGS_VBLANK)
            {
                GBC_MMU_Memory.InterruptFlags &= ~GBC_MMU_INTERRUPT_FLAGS_VBLANK;

                GBC_CPU_InterruptMasterEnable = false;
                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x40;
                GBC_CPU_InstructionTicks += 20;
            }
            else if (GBC_CPU_PendingInterrupts & GBC_MMU_INTERRUPT_FLAGS_LCD_STAT)
            {
                GBC_MMU_Memory.InterruptFlags &= ~GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;

                GBC_CPU_InterruptMasterEnable = false;
                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x48;
                GBC_CPU_InstructionTicks += 20;
            }
            else if (GBC_CPU_PendingInterrupts & GBC_MMU_INTERRUPT_FLAGS_TIMER)
            {
                GBC_MMU_Memory.InterruptFlags &= ~GBC_MMU_INTERRUPT_FLAGS_TIMER;

                GBC_CPU_InterruptMasterEnable = false;
                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x50;
                GBC_CPU_InstructionTicks += 20;
            }
            else if (GBC_CPU_PendingInterrupts & GBC_MMU_INTERRUPT_FLAGS_SERIAL)
            {
                GBC_MMU_Memory.InterruptFlags &= ~GBC_MMU_INTERRUPT_FLAGS_SERIAL;

                GBC_CPU_InterruptMasterEnable = false;
                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x58;
                GBC_CPU_InstructionTicks += 20;
            }
            else if (GBC_CPU_PendingInterrupts & GBC_MMU_INTERRUPT_FLAGS_JOYPAD)
            {
                GBC_MMU_Memory.InterruptFlags &= ~GBC_MMU_INTERRUPT_FLAGS_JOYPAD;

                GBC_CPU_InterruptMasterEnable = false;
                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x60;
                GBC_CPU_InstructionTicks += 20;
            }
        }

        // Save start position of current instruction
        GBC_CPU_InstructionAddress = GBC_CPU_Register.PC;

        GBC_CPU_InstructionOpcode  = GBC_MMU_ReadByte(GBC_CPU_InstructionAddress);

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
        GBC_CPU_InstructionCalls.instr_calls[GBC_CPU_InstructionOpcode]++;
#endif

#ifdef DEBUG_COUNT_INSTRUCTIONS
        GBC_CPU_InstructionsPerStep++;
#endif

        // Increment program counter
        GBC_CPU_Register.PC++;

#ifdef GBC_CPU_USE_FUNCTION_PTR
        // Get instruction information from instruction array
        GBC_CPU_Instruction_t instruction = GBC_CPU_Instructions[GBC_CPU_InstructionOpcode];

        switch (instruction.OperandBytes)
        {
            case GBC_CPU_OPERAND_BYTES_0:
            {
                ((void (*)(void))instruction.Handler)();

                break;
            }
            case GBC_CPU_OPERAND_BYTES_1:
            {

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC);

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
                if (GBC_CPU_InstructionOpcode == 0xCB)
                {
                    GBC_CPU_InstructionCalls.instr_calls[256 + operand]++;
                }
#endif

                GBC_CPU_Register.PC++;

                ((void (*)(uint8_t))instruction.Handler)(operand);

                break;
            }
            case GBC_CPU_OPERAND_BYTES_2:
            {
                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                ((void (*)(uint16_t))instruction.Handler)(operand);

                break;
            }
        }

        GBC_CPU_InstructionTicks += instruction.Ticks;
#else
        // Instruction handling
        switch (GBC_CPU_InstructionOpcode)
        {
            case 0x00: // NOP
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x01: // Load 16-bit immediate into BC
            {
                GBC_CPU_InstructionTicks += 12;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                GBC_CPU_Register.BC = operand;

                break;
            }
            case 0x02: // Save A to address pointed by BC
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.BC, GBC_CPU_Register.A);

                break;
            }
            case 0x03: // Increment 16-bit BC
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.BC++;

                // Flags not affected

                break;
            }
            case 0x04: // Increment B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.B);

                break;
            }
            case 0x05: // Decrement B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B--;

                // Carry flag not affected

                if ((GBC_CPU_Register.B & 0x0F) == 0x0F)
                {
                    GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | (GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
                }
                else
                {
                    if (GBC_CPU_Register.B)
                    {
                        GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | GBC_CPU_FLAGS_SUBTRACTION;
                    }
                    else
                    {
                        GBC_CPU_Register.F = (GBC_CPU_Register.F & GBC_CPU_FLAGS_CARRY) | (GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);
                    }
                }

                break;
            }
            case 0x06: // Load 8-bit immediate into B
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.B = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x07: // Rotate A left with carry
            {
                GBC_CPU_InstructionTicks += 4;

                if (GBC_CPU_Register.A & 0x80)
                {
                    GBC_CPU_Register.A <<= 1;
                    GBC_CPU_Register.A  |= 1;

                    GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
                }
                else
                {
                    GBC_CPU_Register.A <<= 1;

                    GBC_CPU_Register.F = 0;
                }

                break;
            }
            case 0x08: // Save SP to given address
            {
                GBC_CPU_InstructionTicks += 20;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                GBC_MMU_WriteShort(operand, GBC_CPU_Register.SP);

                break;
            }
            case 0x09: // Add 16-bit BC to HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.BC);

                break;
            }
            case 0x0A: // Load A from address pointed to by BC
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.BC);

                break;
            }
            case 0x0B: // Decrement 16-bit BC
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.BC--;

                // No flags affected

                break;
            }
            case 0x0C: // Increment C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.C);

                break;
            }
            case 0x0D: // Decrement C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.C);

                break;
            }
            case 0x0E: // Load 8-bit immediate into C
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.C = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x0F: // Rotate A right with carry
            {
                GBC_CPU_InstructionTicks += 4;

                if (GBC_CPU_Register.A & 0x1)
                {
                    GBC_CPU_Register.A >>= 1;
                    GBC_CPU_Register.A  |= 0x80;

                    GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
                }
                else
                {
                    GBC_CPU_Register.A >>= 1;

                    GBC_CPU_Register.F = 0;
                }

                break;
            }
            case 0x10: // Stop processor
            {
                // No instruction ticks

                if (GBC_MMU_IS_CGB_MODE())
                {
                    if (GBC_MMU_Memory.PrepareSpeedSwitch)
                    {
                        GBC_MMU_Memory.PrepareSpeedSwitch = 0;
                        GBC_MMU_Memory.CurrentSpeed = !GBC_MMU_Memory.CurrentSpeed;
                        GBC_CPU_SpeedModifier = GBC_CPU_SpeedModifier ? 0 : 1;
                    }
                }

                break;
            }
            case 0x11: // Load 16-bit immediate into DE
            {
                GBC_CPU_InstructionTicks += 12;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                GBC_CPU_Register.DE = operand;

                break;
            }
            case 0x12: // Save A to address pointed by DE
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.DE, GBC_CPU_Register.A);

                break;
            }
            case 0x13: // Increment 16-bit DE
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.DE++;

                // Flags not affected

                break;
            }
            case 0x14: // Increment D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.D);

                break;
            }
            case 0x15: // Decrement D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.D);

                break;
            }
            case 0x16: // Load 8-bit immediate into D
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.D = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x17: // Rotate A left
            {
                GBC_CPU_InstructionTicks += 4;

                uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0;

                if (GBC_CPU_Register.A & 0x80)
                {
                    GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
                }
                else
                {
                    GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
                }

                GBC_CPU_Register.A <<= 1;
                GBC_CPU_Register.A  |= carry;

                break;
            }
            case 0x18: // Relative jump by signed immediate
            {
                GBC_CPU_InstructionTicks += 12;

                int8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_Register.PC += operand;

                break;
            }
            case 0x19: // Add 16-bit DE to HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.DE);

                break;
            }
            case 0x1A: // Load A from address pointed to by DE
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.DE);

                break;
            }
            case 0x1B: // Decrement 16-bit DE
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.DE--;

                // No flags affected

                break;
            }
            case 0x1C: // Increment E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.E);

                break;
            }
            case 0x1D: // Decrement E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.E);

                break;
            }
            case 0x1E: // Load 8-bit immediate into E
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.E = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x1F: // Rotate A right
            {
                GBC_CPU_InstructionTicks += 4;

                uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 0x80 : 0;

                if (GBC_CPU_Register.A & 0x1)
                {
                    GBC_CPU_Register.F = GBC_CPU_FLAGS_CARRY;
                }
                else
                {
                    GBC_CPU_Register.F = GBC_CPU_FLAGS_NONE;
                }

                GBC_CPU_Register.A >>= 0x1;
                GBC_CPU_Register.A  |= carry;

                break;
            }
            case 0x20: // Relative jump by signed immediate if last result was not zero
            {
                int8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += operand;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                break;
            }
            case 0x21: // Load 16-bit immediate into HL
            {
                GBC_CPU_InstructionTicks += 12;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                GBC_CPU_Register.HL = operand;

                break;
            }
            case 0x22: // Save A to address pointed by HL, and increment HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL++, GBC_CPU_Register.A);

                break;
            }
            case 0x23: // Increment 16-bit HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.HL++;

                // Flags not affected

                break;
            }
            case 0x24: // Increment H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.H);

                break;
            }
            case 0x25: // Decrement H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.H);

                break;
            }
            case 0x26: // Load 8-bit immediate into H
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.H = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x27: // Adjust A for BCD (Binary Coded Decimal) addition
            {
                GBC_CPU_InstructionTicks += 4;

                int32_t value = GBC_CPU_Register.A;

                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_SUBTRACTION))
                {
                    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY))
                    {
                        value -= 0x06;

                        if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                        {
                            value &= 0xFF;
                        }
                    }

                    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                    {
                        value -= 0x60;
                    }
                }
                else
                {
                    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY) || (value & 0xF) > 9)
                    {
                        value += 0x06;
                    }

                    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) || value > 0x9F)
                    {
                        value += 0x60;
                    }
                }

                if (value & 0x100)
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
                }
                //else
                {
                    // Do not clear carry flag
                }

                GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);

                // Subtraction flag not affected

                value &= 0xFF;

                if (value)
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
                }
                else
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
                }

                GBC_CPU_Register.A = value;

                break;
            }
            case 0x28: // Relative jump by signed immediate if last result was zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 12;

                    int8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.PC);

                    GBC_CPU_Register.PC += value;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                GBC_CPU_Register.PC++;

                break;
            }
            case 0x29: // Add 16-bit HL to HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.HL);

                break;
            }
            case 0x2A: // Load A from address pointed to by HL, and increment HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL++);

                break;
            }
            case 0x2B: // Decrement 16-bit HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.HL--;

                // No flags affected

                break;
            }
            case 0x2C: // Increment L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.L);

                break;
            }
            case 0x2D: // Decrement L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.L);

                break;
            }
            case 0x2E: // Load 8-bit immediate into L
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.L = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x2F: // Complement (logical NOT) on A (flip all bits)
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = ~GBC_CPU_Register.A;

                // Carry flag not affected
                GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
                // Zero flag not affected

                break;
            }
            case 0x30: // Relative jump by signed immediate if last result caused no carry
            {
                if (!GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 12;

                    int8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.PC);

                    GBC_CPU_Register.PC += value;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                GBC_CPU_Register.PC++;

                break;
            }
            case 0x31: // Load 16-bit immediate into SP
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_CPU_Register.SP = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                GBC_CPU_Register.PC += 2;

                break;
            }
            case 0x32: // Save A to address pointed by HL, and decrement HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL--, GBC_CPU_Register.A);

                break;
            }
            case 0x33: // Increment 16-bit SP
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.SP++;

                // Flags not affected

                break;
            }
            case 0x34: // Increment value pointed by HL
            {
                GBC_CPU_InstructionTicks += 12;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_INC(value)

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, value);
                
                break;
            }
            case 0x35: // Decrement value pointed by HL
            {
                GBC_CPU_InstructionTicks += 12;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_DEC(value);

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, value);

                break;
            }
            case 0x36: // Load 8-bit immediate into address pointed by HL
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_MMU_ReadByte(GBC_CPU_Register.PC++));

                break;
            }
            case 0x37: // Set carry flag
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
                GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
                // Zero flag not affected

                break;
            }
            case 0x38: // Relative jump by signed immediate if last result caused carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 12;

                    int8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.PC);

                    GBC_CPU_Register.PC += value;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                GBC_CPU_Register.PC++;

                break;
            }
            case 0x39: // Add 16-bit SP to HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_ADD2(GBC_CPU_Register.HL, GBC_CPU_Register.SP);

                break;
            }
            case 0x3A: // Load A from address pointed to by HL, and decrement HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL--);

                break;
            }
            case 0x3B: // Decrement 16-bit SP
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.SP--;

                // No flags affected

                break;
            }
            case 0x3C: // Increment A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_INC(GBC_CPU_Register.A);

                break;
            }
            case 0x3D: // Decrement A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_DEC(GBC_CPU_Register.A);

                break;
            }
            case 0x3E: // Load 8-bit immediate into A
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                break;
            }
            case 0x3F: // Complement carry flag
            {
                GBC_CPU_InstructionTicks += 4;

                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
                }
                else
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
                }

                // Zero flag not affected

                break;
            }
            case 0x40: // Copy B to B
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x41: // Copy C to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.C;

                break;
            }
            case 0x42: // Copy D to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.D;

                break;
            }
            case 0x43: // Copy E to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.E;

                break;
            }
            case 0x44: // Copy H to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.H;

                break;
            }
            case 0x45: // Copy L to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.L;

                break;
            }
            case 0x46: // Copy value pointed by HL to B
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.B = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x47: // Copy A to B
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.B = GBC_CPU_Register.A;

                break;
            }
            case 0x48: // Copy B to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.B;

                break;
            }
            case 0x49: // Copy C to C
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x4A: // Copy D to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.D;

                break;
            }
            case 0x4B: // Copy E to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.E;

                break;
            }
            case 0x4C: // Copy H to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.H;

                break;
            }
            case 0x4D: // Copy L to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.L;

                break;
            }
            case 0x4E: // Copy value pointed by HL to C
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.C = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x4F: // Copy A to C
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.C = GBC_CPU_Register.A;

                break;
            }
            case 0x50: // Copy B to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.B;

                break;
            }
            case 0x51: // Copy C to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.C;

                break;
            }
            case 0x52: // Copy D to D
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x53: // Copy E to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.E;

                break;
            }
            case 0x54: // Copy H to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.H;

                break;
            }
            case 0x55: // Copy L to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.L;

                break;
            }
            case 0x56: // Copy value pointed by HL to D
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.D = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x57: // Copy A to D
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.D = GBC_CPU_Register.A;

                break;
            }
            case 0x58: // Copy B to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.B;

                break;
            }
            case 0x59: // Copy C to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.C;

                break;
            }
            case 0x5A: // Copy D to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.D;

                break;
            }
            case 0x5B: // Copy E to E
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x5C: // Copy H to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.H;

                break;
            }
            case 0x5D: // Copy L to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.L;

                break;
            }
            case 0x5E: // Copy value pointed by HL to E
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.E = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x5F: // Copy A to E
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.E = GBC_CPU_Register.A;

                break;
            }
            case 0x60: // Copy B to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.B;

                break;
            }
            case 0x61: // Copy C to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.C;

                break;
            }
            case 0x62: // Copy D to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.D;

                break;
            }
            case 0x63: // Copy E to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.E;

                break;
            }
            case 0x64: // Copy H to H
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x65: // Copy L to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.L;

                break;
            }
            case 0x66: // Copy value pointed by HL to H
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.H = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x67: // Copy A to H
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.H = GBC_CPU_Register.A;

                break;
            }
            case 0x68: // Copy B to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.B;

                break;
            }
            case 0x69: // Copy C to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.C;

                break;
            }
            case 0x6A: // Copy D to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.D;

                break;
            }
            case 0x6B: // Copy E to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.E;

                break;
            }
            case 0x6C: // Copy H to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.H;

                break;
            }
            case 0x6D: // Copy L to L
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x6E: // Copy value pointed by HL to L
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.L = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x6F: // Copy A to L
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.L = GBC_CPU_Register.A;

                break;
            }
            case 0x70: // Copy B to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.B);

                break;
            }
            case 0x71: // Copy C to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.C);

                break;
            }
            case 0x72: // Copy D to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.D);

                break;
            }
            case 0x73: // Copy E to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.E);

                break;
            }
            case 0x74: // Copy H to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.H);

                break;
            }
            case 0x75: // Copy L to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.L);

                break;
            }
            case 0x76: // Halt processor
            {
                // No instruction ticks

                if (GBC_CPU_InterruptMasterEnableDelayTicks)
                {
                    // Serve pending interrupts before HALT
                    GBC_CPU_InterruptMasterEnableDelayTicks = 0;
                    GBC_CPU_InterruptMasterEnable = true;
                    GBC_CPU_Register.PC--;
                }
                else
                {
                    GBC_CPU_Halted = true;
                }

                break;
            }
            case 0x77: // Copy A to address pointed by HL
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_Register.A);

                break;
            }
            case 0x78: // Copy B to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.B;

                break;
            }
            case 0x79: // Copy C to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.C;

                break;
            }
            case 0x7A: // Copy D to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.D;

                break;
            }
            case 0x7B: // Copy E to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.E;

                break;
            }
            case 0x7C: // Copy H to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.H;

                break;
            }
            case 0x7D: // Copy L to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.A = GBC_CPU_Register.L;

                break;
            }
            case 0x7E: // Copy value pointed by HL to A
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                break;
            }
            case 0x7F: // Copy A to A
            {
                GBC_CPU_InstructionTicks += 4;

                // Do nothing

                break;
            }
            case 0x80: // Add B to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0x81: // Add C to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0x82: // Add D to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0x83: // Add E to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0x84: // Add H to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0x85: // Add L to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0x86: // Add value pointed by HL to A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_ADD(GBC_CPU_Register.A, value);

                break;
            }
            case 0x87: // Add A to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADD(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0x88: // Add B and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0x89: // Add C and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0x8A: // Add D and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0x8B: // Add E and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0x8C: // Add H and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0x8D: // Add L and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0x8E: // Add value pointed by HL and carry flag to A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_ADC(GBC_CPU_Register.A, value);

                break;
            }
            case 0x8F: // Add A and carry flag to A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_ADC(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0x90: // Subtract B from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0x91: // Subtract C from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0x92: // Subtract D from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0x93: // Subtract E from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0x94: // Subtract H from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0x95: // Subtract L from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0x96: // Subtract value pointed by HL from A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_SUB(GBC_CPU_Register.A, value);

                break;
            }
            case 0x97: // Subtract A from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SUB(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0x98: // Subtract B and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0x99: // Subtract C and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0x9A: // Subtract D and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0x9B: // Subtract E and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0x9C: // Subtract H and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0x9D: // Subtract L and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0x9E: // Subtract value pointed by HL and carry flag from A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_SBC(GBC_CPU_Register.A, value);

                break;
            }
            case 0x9F: // Subtract A and carry flag from A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_SBC(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0xA0: // Logical AND B against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0xA1: // Logical AND C against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0xA2: // Logical AND D against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0xA3: // Logical AND E against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0xA4: // Logical AND H against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0xA5: // Logical AND L against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0xA6: // Logical AND value pointed by HL against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_AND(GBC_CPU_Register.A, value);

                break;
            }
            case 0xA7: // Logical AND A against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_AND(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0xA8: // Logical XOR B against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0xA9: // Logical XOR C against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0xAA: // Logical XOR D against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0xAB: // Logical XOR E against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0xAC: // Logical XOR H against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0xAD: // Logical XOR L against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0xAE: // Logical XOR value pointed by HL against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_XOR(GBC_CPU_Register.A, value);

                break;
            }
            case 0xAF: // Logical XOR A against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_XOR(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0xB0: // Logical OR B against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0xB1: // Logical OR C against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0xB2: // Logical OR D against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0xB3: // Logical OR E against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0xB4: // Logical OR H against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0xB5: // Logical OR L against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0xB6: // Logical OR value pointed by HL against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_OR(GBC_CPU_Register.A, value);

                break;
            }
            case 0xB7: // Logical OR A against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_OR(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0xB8: // Compare B against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.B);

                break;
            }
            case 0xB9: // Compare C against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.C);

                break;
            }
            case 0xBA: // Compare D against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.D);

                break;
            }
            case 0xBB: // Compare E against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.E);

                break;
            }
            case 0xBC: // Compare H against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.H);

                break;
            }
            case 0xBD: // Compare L against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.L);

                break;
            }
            case 0xBE: // Compare value pointed by HL against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t value = GBC_MMU_ReadByte(GBC_CPU_Register.HL);

                GBC_CPU_COMPARE(GBC_CPU_Register.A, value);

                break;
            }
            case 0xBF: // Compare A against A
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_COMPARE(GBC_CPU_Register.A, GBC_CPU_Register.A);

                break;
            }
            case 0xC0: // Return if last result was not zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 8;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 20;
                    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
                }

                break;
            }
            case 0xC1: // Pop 16-bit value from stack into BC
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.BC);

                break;
            }
            case 0xC2: // Absolute jump to 16-bit location if last result was not zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 16;
                    GBC_CPU_Register.PC = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                }

                break;
            }
            case 0xC3: // Absolute jump to 16-bit location
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_Register.PC = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                break;
            }
            case 0xC4: // Call routine at 16-bit location if last result was not zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 24;

                    uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC += 2;

                    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC = operand;
                }

                break;
            }
            case 0xC5: // Push 16-bit BC onto stack
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.BC);

                break;
            }
            case 0xC6: // Add 8-bit immediate to A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_ADD(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xC7: // Call routine at address 0000h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0;

                break;
            }
            case 0xC8: // Return if last result was zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 20;
                    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                break;
            }
            case 0xC9: // Return to calling routine
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);

                break;
            }
            case 0xCA: // Absolute jump to 16-bit location if last result was zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 16;
                    GBC_CPU_Register.PC = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                }
                else
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }

                break;
            }
            case 0xCB: // Extended operations (two-byte instruction code)
            {
                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_EX_Instruction_t instruction = GBC_CPU_EX_Instructions[operand];    // Get extended instruction

                ((void (*)(void))instruction.Handler)();                                    // Execute extended instruction

                GBC_CPU_InstructionTicks += instruction.Ticks;                              // Add extended instruction ticks

                break;
            }
            case 0xCC: // Call routine at 16-bit location if last result was zero
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
                {
                    GBC_CPU_InstructionTicks += 24;

                    uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC += 2;

                    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC = operand;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }

                break;
            }
            case 0xCD: // Call routine at 16-bit location
            {
                GBC_CPU_InstructionTicks += 24;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC += 2;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = operand;

                break;
            }
            case 0xCE: // Add 8-bit immediate and carry to A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_ADC(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xCF: // Call routine at address 0008h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x8;

                break;
            }
            case 0xD0: // Return if last result caused no carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 8;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 20;
                    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
                }

                break;
            }
            case 0xD1: // Pop 16-bit value from stack into DE
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.DE);

                break;
            }
            case 0xD2: // Absolute jump to 16-bit location if last result caused no carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 16;
                    GBC_CPU_Register.PC = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                }

                break;
            }
            case 0xD3: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xD4: // Call routine at 16-bit location if last result caused no carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 24;

                    uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC += 2;

                    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC = operand;
                }

                break;
            }
            case 0xD5: // Push 16-bit DE onto stack
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.DE);

                break;
            }
            case 0xD6: // Subtract 8-bit immediate from A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_SUB(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xD7: // Call routine at address 0010h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x10;

                break;
            }
            case 0xD8: // Return if last result caused carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 20;
                    GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);
                }
                else
                {
                    GBC_CPU_InstructionTicks += 8;
                }

                break;
            }
            case 0xD9: // Enable interrupts and return to calling routine
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_InterruptMasterEnable = true;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.PC);

                break;
            }
            case 0xDA: // Absolute jump to 16-bit location if last result caused carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 16;
                    GBC_CPU_Register.PC = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                }
                else
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }

                break;
            }
            case 0xDB: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xDC: // Call routine at 16-bit location if last result caused carry
            {
                if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
                {
                    GBC_CPU_InstructionTicks += 24;

                    uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC += 2;

                    GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                    GBC_CPU_Register.PC = operand;
                }
                else
                {
                    GBC_CPU_InstructionTicks += 12;
                    GBC_CPU_Register.PC += 2;
                }

                break;
            }
            case 0xDD: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xDE: // Subtract 8-bit immediate and carry from A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_SBC(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xDF: // Call routine at address 0018h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x18;

                break;
            }
            case 0xE0: // Save A at address pointed to by (FF00h + 8-bit immediate)
            {
                GBC_CPU_InstructionTicks += 12;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_MMU_WriteByte(0xFF00 + operand, GBC_CPU_Register.A);

                break;
            }
            case 0xE1: // Pop 16-bit value from stack into HL
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.HL);

                break;
            }
            case 0xE2: // Save A at address pointed to by (FF00h + C)
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_MMU_WriteByte(0xFF00 + GBC_CPU_Register.C, GBC_CPU_Register.A);

                break;
            }
            case 0xE3: // Operation removed in this CPU
            case 0xE4: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xE5: // Push 16-bit HL onto stack
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.HL);

                break;
            }
            case 0xE6: // Logical AND 8-bit value immediate against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_AND(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xE7: // Call routine at address 0020h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x20;

                break;
            }
            case 0xE8: // Add signed 8-bit immediate to SP
            {
                GBC_CPU_InstructionTicks += 16;

                int8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);
                int32_t result = GBC_CPU_Register.SP + operand;

                if ((GBC_CPU_Register.SP & 0xFF) + (operand & 0xFF) > 0xFF)
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
                }
                else
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
                }

                if ((GBC_CPU_Register.SP & 0x0F) + (operand & 0x0F) > 0x0F)
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
                }
                else
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
                }

                // GBC_CPU_FLAGS_ZERO is cleared according to documentation
                GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);

                result &= 0xFFFF;

                GBC_CPU_Register.SP = result;

                break;
            }
            case 0xE9: // Jump to HL
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_Register.PC = GBC_CPU_Register.HL;

                break;
            }
            case 0xEA: // Save A at given 16-bit address
            {
                GBC_CPU_InstructionTicks += 16;

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC += 2;

                GBC_MMU_WriteByte(operand, GBC_CPU_Register.A);

                break;
            }
            case 0xEB: // Operation removed in this CPU
            case 0xEC: // Operation removed in this CPU
            case 0xED: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xEE: // Logical XOR 8-bit value immediate against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_XOR(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xEF: // Call routine at address 0028h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x28;

                break;
            }
            case 0xF0: // Load A from address pointed to by (FF00h + 8-bit immediate)
            {
                GBC_CPU_InstructionTicks += 12;

                //if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
                //{
                //    GBC_CPU_Register.PC++;
                //
                //    GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
                //    break;
                //}

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_Register.A = GBC_MMU_ReadByte(0xFF00 + operand);

                break;
            }
            case 0xF1: // Pop 16-bit value from stack into AF
            {
                GBC_CPU_InstructionTicks += 12;

                GBC_CPU_POP_FROM_STACK(GBC_CPU_Register.AF);
                GBC_CPU_Register.AF = GBC_CPU_Register.AF & 0xFFF0;

                break;
            }
            case 0xF2: // Load A from address pointed to by (FF00h + C)
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(0xFF00 + GBC_CPU_Register.C);

                break;
            }
            case 0xF3: // Disable Interrupts
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_InterruptMasterEnableDelayTicks = 0;
                GBC_CPU_InterruptMasterEnable = false;

                break;
            }
            case 0xF4: // Operation removed in this CPU
            {
                // Do nothing

                break;
            }
            case 0xF5: // Push 16-bit AF onto stack
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.AF);

                break;
            }
            case 0xF6: // Logical OR 8-bit value immediate against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_OR(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xF7: // Call routine at address 0030h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x30;

                break;
            }
            case 0xF8: // Add signed 8-bit immediate to SP and save result in HL
            {
                GBC_CPU_InstructionTicks += 12;

                int8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                int32_t result = GBC_CPU_Register.SP + operand;

                if ((GBC_CPU_Register.SP & 0xFF) + (operand & 0xFF) > 0xFF)
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
                }
                else
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
                }

                if ((GBC_CPU_Register.SP & 0x0F) + (operand & 0x0F) > 0x0F)
                {
                    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
                }
                else
                {
                    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
                }

                // GBC_CPU_FLAGS_ZERO is cleared according to documentation
                GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);

                result &= 0xFFFF;

                GBC_CPU_Register.HL = result;

                break;
            }
            case 0xF9: // Copy HL to SP
            {
                GBC_CPU_InstructionTicks += 8;

                GBC_CPU_Register.SP = GBC_CPU_Register.HL;

                break;
            }
            case 0xFA: // Load A from given 16-bit address
            {
                GBC_CPU_InstructionTicks += 16;

                //if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
                //{
                //    GBC_CPU_Register.PC += 2;
                //
                //    GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
                //    break;
                //}

                uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC += 2;

                GBC_CPU_Register.A = GBC_MMU_ReadByte(operand);

                break;
            }
            case 0xFB: // Enable Interrupts
            {
                GBC_CPU_InstructionTicks += 4;

                GBC_CPU_InterruptMasterEnableDelayTicks = GBC_CPU_Instructions[0xFB].Ticks + 1;

                break;
            }
            case 0xFC: // Operation removed in this CPU
            case 0xFD: // Operation removed in this CPU
            {
                // Do nothing
                
                break;
            }
            case 0xFE: // Compare 8-bit value immediate against A
            {
                GBC_CPU_InstructionTicks += 8;

                uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC++);

                GBC_CPU_COMPARE(GBC_CPU_Register.A, operand);

                break;
            }
            case 0xFF: // Call routine at address 0038h
            {
                GBC_CPU_InstructionTicks += 16;

                GBC_CPU_PUSH_TO_STACK(GBC_CPU_Register.PC);
                GBC_CPU_Register.PC = 0x38;

                break;
            }
            default:
            {
                // Get instruction information from instruction array
                GBC_CPU_Instruction_t instruction = GBC_CPU_Instructions[GBC_CPU_InstructionOpcode];

                switch (instruction.OperandBytes)
                {
                    case GBC_CPU_OPERAND_BYTES_0:
                    {
                        ((void (*)(void))instruction.Handler)();

                        break;
                    }
                    case GBC_CPU_OPERAND_BYTES_1:
                    {

                        uint8_t operand = GBC_MMU_ReadByte(GBC_CPU_Register.PC);

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
                        if (GBC_CPU_InstructionOpcode == 0xCB)
                        {
                            GBC_CPU_InstructionCalls.instr_calls[256 + operand]++;
                        }
#endif

                        GBC_CPU_Register.PC++;

                        ((void (*)(uint8_t))instruction.Handler)(operand);

                        break;
                    }
                    case GBC_CPU_OPERAND_BYTES_2:
                    {
                        uint16_t operand = GBC_MMU_ReadShort(GBC_CPU_Register.PC);

                        GBC_CPU_Register.PC += 2;

                        ((void (*)(uint16_t))instruction.Handler)(operand);

                        break;
                    }
                }

                GBC_CPU_InstructionTicks += instruction.Ticks;

                break;
            }
        }
#endif

        GBC_CPU_StepTicks += GBC_CPU_InstructionTicks;

        /*switch (GBC_CPU_MemoryAccessDelayState)
        {
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE:
                GBC_CPU_StepTicks += GBC_CPU_InstructionTicks;
                break;
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT:
                GBC_CPU_StepTicks += GBC_CPU_InstructionTicks - 8;
                GBC_CPU_Register.PC = GBC_CPU_InstructionAddress;
                GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT2;
                break;
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT2:
                GBC_CPU_StepTicks += 8;
                GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE;
                break;
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT:
                GBC_CPU_StepTicks += GBC_CPU_InstructionTicks - 12;
                GBC_CPU_Register.PC = GBC_CPU_InstructionAddress;
                GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT2;
                break;
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT2:
                GBC_CPU_StepTicks += 4;
                GBC_CPU_Register.PC = GBC_CPU_InstructionAddress;
                GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT3;
                break;
            case GBC_CPU_MEMORY_ACCESS_DELAY_STATE_12_TICKS_LEFT3:
                GBC_CPU_StepTicks += 8;
                GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE;
                break;
        }*/
    }

    if (GBC_CPU_InterruptMasterEnableDelayTicks)
    {
        GBC_CPU_InterruptMasterEnableDelayTicks -= GBC_CPU_StepTicks;

        if (GBC_CPU_InterruptMasterEnableDelayTicks <= 0)
        {
            GBC_CPU_InterruptMasterEnableDelayTicks = 0;
            GBC_CPU_InterruptMasterEnable = true;
        }
    }

#ifdef DEBUG_PRINT_INSTRUCTION_CALLS
    if (GBC_CPU_DebugPrintEnabled)
    {
        for (long i = 0; i < 512; i++)
        {
            // PB3 is used for serial transfer
            printf("%ld;%ld*", i, GBC_CPU_InstructionCalls.instr_calls[i]);
        }

        GBC_CPU_DebugPrintEnabled = false;
    }
#endif

    GBC_CPU_StepTicks >>= GBC_CPU_SpeedModifier;
}
