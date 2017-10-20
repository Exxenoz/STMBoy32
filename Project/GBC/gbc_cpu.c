#include "gbc_cpu.h"
#include "gbc_mmu.h"

// Global GBC CPU register
GBC_CPU_Register_t GBC_CPU_Register;

uint8_t GBC_CPU_IncrementByte(uint8_t value)
{
    value++;

    if ((value & 0xF) == 0)
    {
        GBC_CPU_Register.F |= GBC_CPU_FLAGS_HALFCARRY;
    }
    else
    {
        GBC_CPU_Register.F &= ~GBC_CPU_FLAGS_HALFCARRY;
    }

    GBC_CPU_Register.F &= ~GBC_CPU_FLAGS_SUBTRACTION;

    if (value)
    {
        GBC_CPU_Register.F &= ~GBC_CPU_FLAGS_ZERO;
    }
    else
    {
        GBC_CPU_Register.F |= GBC_CPU_FLAGS_ZERO;
    }

    return value;
}

uint8_t GBC_CPU_DecrementByte(uint8_t value)
{
    value--;

    if ((value & 0xF) == 0xF)
    {
        GBC_CPU_Register.F |= GBC_CPU_FLAGS_HALFCARRY;
    }
    else
    {
        GBC_CPU_Register.F &= ~GBC_CPU_FLAGS_HALFCARRY;
    }

    GBC_CPU_Register.F |= GBC_CPU_FLAGS_SUBTRACTION;

    if (value)
    {
        GBC_CPU_Register.F &= ~GBC_CPU_FLAGS_ZERO;
    }
    else
    {
        GBC_CPU_Register.F |= GBC_CPU_FLAGS_ZERO;
    }

    return value;
}

void GBC_CPU_NOP()                      // 0x00 - No operation
{
    // Do nothing
}

void GBC_CPU_LD_BC_XX(uint16_t operand) // 0x01 - Load 2 bytes into BC
{
    GBC_CPU_Register.BC = operand;
}

void GBC_CPU_LD_BCP_A()                 // 0x02 - Save A to address pointed by BC
{
    GBC_MMU_WriteByte(GBC_CPU_Register.BC, GBC_CPU_Register.A);
}

void GBC_CPU_INC_BC()                   // 0x03 - Increment register BC
{
    GBC_CPU_Register.BC++;
}

void GBC_CPU_INC_B()                    // 0x04 - Increment register B
{
    GBC_CPU_Register.B = GBC_CPU_IncrementByte(GBC_CPU_Register.B);
}

void GBC_CPU_DEC_B()                    // 0x05 - Decrement register B
{
    GBC_CPU_Register.B = GBC_CPU_DecrementByte(GBC_CPU_Register.B);
}

const GBC_CPU_Instruction_t GBC_CPU_Instructions[6] =
{
    { GBC_CPU_NOP,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2 }, // 0x00 - No operation
    { GBC_CPU_LD_BC_XX, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6 }, // 0x01 - Load 2 bytes into BC
    { GBC_CPU_LD_BCP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4 }, // 0x02 - Save A to address pointed by BC
    { GBC_CPU_INC_BC,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4 }, // 0x03 - Increment register BC
    { GBC_CPU_INC_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2 }, // 0x04 - Increment register B
    { GBC_CPU_DEC_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2 }, // 0x05 - Decrement register B
};
