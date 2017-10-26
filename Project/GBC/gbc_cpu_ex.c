#include "gbc_cpu_ex.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint8_t GBC_CPU_EX_RLC(uint8_t value)
{
    if (value & 0x80)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);

        value <<= 1;
        value  += 1;
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);

        value <<= 1;
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);

    if (value)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return value;
}

uint8_t GBC_CPU_EX_RRC(uint8_t value)
{
    if (value & 0x01)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);

        value >>= 1;
        value  |= 0x80;
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);

        value >>= 1;
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);

    if (value)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return value;
}

void GBC_CPU_EX_RLC_B()                     // 0x00 - Rotate B left with carry
{
    GBC_CPU_Register.B = GBC_CPU_EX_RLC(GBC_CPU_Register.B);
}

void GBC_CPU_EX_RLC_C()                     // 0x01 - Rotate C left with carry
{
    GBC_CPU_Register.C = GBC_CPU_EX_RLC(GBC_CPU_Register.C);
}

void GBC_CPU_EX_RLC_D()                     // 0x02 - Rotate D left with carry
{
    GBC_CPU_Register.D = GBC_CPU_EX_RLC(GBC_CPU_Register.D);
}

void GBC_CPU_EX_RLC_E()                     // 0x03 - Rotate E left with carry
{
    GBC_CPU_Register.E = GBC_CPU_EX_RLC(GBC_CPU_Register.E);
}

void GBC_CPU_EX_RLC_H()                     // 0x04 - Rotate H left with carry
{
    GBC_CPU_Register.H = GBC_CPU_EX_RLC(GBC_CPU_Register.H);
}

void GBC_CPU_EX_RLC_L()                     // 0x05 - Rotate L left with carry
{
    GBC_CPU_Register.L = GBC_CPU_EX_RLC(GBC_CPU_Register.L);
}

void GBC_CPU_EX_RLC_HLP()                   // 0x06 - Rotate value pointed by HL left with carry
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_RLC(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_RLC_A()                     // 0x07 - Rotate A left with carry
{
    GBC_CPU_Register.A = GBC_CPU_EX_RLC(GBC_CPU_Register.A);
}

void GBC_CPU_EX_RRC_B()                     // 0x08 - Rotate B right with carry
{
    GBC_CPU_Register.B = GBC_CPU_EX_RRC(GBC_CPU_Register.B);
}

void GBC_CPU_EX_RRC_C()                     // 0x09 - Rotate C right with carry
{
    GBC_CPU_Register.C = GBC_CPU_EX_RRC(GBC_CPU_Register.C);
}

void GBC_CPU_EX_RRC_D()                     // 0x0A - Rotate D right with carry
{
    GBC_CPU_Register.D = GBC_CPU_EX_RRC(GBC_CPU_Register.D);
}

void GBC_CPU_EX_RRC_E()                     // 0x0B - Rotate E right with carry
{
    GBC_CPU_Register.E = GBC_CPU_EX_RRC(GBC_CPU_Register.E);
}

void GBC_CPU_EX_RRC_H()                     // 0x0C - Rotate H right with carry
{
    GBC_CPU_Register.H = GBC_CPU_EX_RRC(GBC_CPU_Register.H);
}

void GBC_CPU_EX_RRC_L()                     // 0x0D - Rotate L right with carry
{
    GBC_CPU_Register.L = GBC_CPU_EX_RRC(GBC_CPU_Register.L);
}

void GBC_CPU_EX_RRC_HLP()                   // 0x0E - Rotate value pointed by HL right with carry
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_RRC(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_RRC_A()                     // 0x0F - Rotate A right with carry
{
    GBC_CPU_Register.A = GBC_CPU_EX_RRC(GBC_CPU_Register.A);
}

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_EX_Instruction_t GBC_CPU_EX_Instructions[16] =
{
    GBC_CPU_EX_RLC_B,   GBC_CPU_TICKS_8,  // 0x00 - Rotate B left with carry
    GBC_CPU_EX_RLC_C,   GBC_CPU_TICKS_8,  // 0x01 - Rotate C left with carry
    GBC_CPU_EX_RLC_D,   GBC_CPU_TICKS_8,  // 0x02 - Rotate D left with carry
    GBC_CPU_EX_RLC_E,   GBC_CPU_TICKS_8,  // 0x03 - Rotate E left with carry
    GBC_CPU_EX_RLC_H,   GBC_CPU_TICKS_8,  // 0x04 - Rotate H left with carry
    GBC_CPU_EX_RLC_L,   GBC_CPU_TICKS_8,  // 0x05 - Rotate L left with carry
    GBC_CPU_EX_RLC_HLP, GBC_CPU_TICKS_16, // 0x06 - Rotate value pointed by HL left with carry
    GBC_CPU_EX_RLC_A,   GBC_CPU_TICKS_8,  // 0x07 - Rotate A left with carry
    GBC_CPU_EX_RRC_B,   GBC_CPU_TICKS_8,  // 0x08 - Rotate B right with carry
    GBC_CPU_EX_RRC_C,   GBC_CPU_TICKS_8,  // 0x09 - Rotate C right with carry
    GBC_CPU_EX_RRC_D,   GBC_CPU_TICKS_8,  // 0x0A - Rotate D right with carry
    GBC_CPU_EX_RRC_E,   GBC_CPU_TICKS_8,  // 0x0B - Rotate E right with carry
    GBC_CPU_EX_RRC_H,   GBC_CPU_TICKS_8,  // 0x0C - Rotate H right with carry
    GBC_CPU_EX_RRC_L,   GBC_CPU_TICKS_8,  // 0x0D - Rotate L right with carry
    GBC_CPU_EX_RRC_HLP, GBC_CPU_TICKS_16, // 0x0E - Rotate value pointed by HL right with carry
    GBC_CPU_EX_RRC_A,   GBC_CPU_TICKS_8,  // 0x0F - Rotate A right with carry
};
