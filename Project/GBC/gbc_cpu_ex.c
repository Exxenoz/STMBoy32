#include "gbc_cpu_ex.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint8_t GBC_CPU_EX_RLC(uint8_t value)
{
    if (value & 0x80)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);

        value <<= 1;
        value  |= 1;
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

uint8_t GBC_CPU_EX_RL(uint8_t value)
{
    if (value & 0x80)
    {
        value <<= 1;

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value |= 1;
        }

        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        value <<= 1;

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value |= 1;
        }

        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
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

uint8_t GBC_CPU_EX_RR(uint8_t value)
{
    if (value & 0x01)
    {
        value >>= 1;

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value |= 0x80;
        }

        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        value >>= 1;

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value |= 0x80;
        }

        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
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

uint8_t GBC_CPU_EX_SLA(uint8_t value)
{
    if (value & 0x80)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    value <<= 1;

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

uint8_t GBC_CPU_EX_SRA(uint8_t value)
{
    if (value & 0x01)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    value = (value & 0x80) | (value >> 1);

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

void GBC_CPU_EX_RL_B()                      // 0x10 - Rotate B left
{
    GBC_CPU_Register.B = GBC_CPU_EX_RL(GBC_CPU_Register.B);
}

void GBC_CPU_EX_RL_C()                      // 0x11 - Rotate C left
{
    GBC_CPU_Register.C = GBC_CPU_EX_RL(GBC_CPU_Register.C);
}

void GBC_CPU_EX_RL_D()                      // 0x12 - Rotate D left
{
    GBC_CPU_Register.D = GBC_CPU_EX_RL(GBC_CPU_Register.D);
}

void GBC_CPU_EX_RL_E()                      // 0x13 - Rotate E left
{
    GBC_CPU_Register.E = GBC_CPU_EX_RL(GBC_CPU_Register.E);
}

void GBC_CPU_EX_RL_H()                      // 0x14 - Rotate H left
{
    GBC_CPU_Register.H = GBC_CPU_EX_RL(GBC_CPU_Register.H);
}

void GBC_CPU_EX_RL_L()                      // 0x15 - Rotate L left
{
    GBC_CPU_Register.L = GBC_CPU_EX_RL(GBC_CPU_Register.L);
}

void GBC_CPU_EX_RL_HLP()                    // 0x16 - Rotate value pointed by HL left
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_RL(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_RL_A()                      // 0x17 - Rotate A left
{
    GBC_CPU_Register.A = GBC_CPU_EX_RL(GBC_CPU_Register.A);
}

void GBC_CPU_EX_RR_B()                      // 0x18 - Rotate B right
{
    GBC_CPU_Register.B = GBC_CPU_EX_RR(GBC_CPU_Register.B);
}

void GBC_CPU_EX_RR_C()                      // 0x19 - Rotate C right
{
    GBC_CPU_Register.C = GBC_CPU_EX_RR(GBC_CPU_Register.C);
}

void GBC_CPU_EX_RR_D()                      // 0x1A - Rotate D right
{
    GBC_CPU_Register.D = GBC_CPU_EX_RR(GBC_CPU_Register.D);
}

void GBC_CPU_EX_RR_E()                      // 0x1B - Rotate E right
{
    GBC_CPU_Register.E = GBC_CPU_EX_RR(GBC_CPU_Register.E);
}

void GBC_CPU_EX_RR_H()                      // 0x1C - Rotate H right
{
    GBC_CPU_Register.H = GBC_CPU_EX_RR(GBC_CPU_Register.H);
}

void GBC_CPU_EX_RR_L()                      // 0x1D - Rotate L right
{
    GBC_CPU_Register.L = GBC_CPU_EX_RR(GBC_CPU_Register.L);
}

void GBC_CPU_EX_RR_HLP()                    // 0x1E - Rotate value pointed by HL right
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_RR(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_RR_A()                      // 0x1F - Rotate A right
{
    GBC_CPU_Register.A = GBC_CPU_EX_RR(GBC_CPU_Register.A);
}

void GBC_CPU_EX_SLA_B()                     // 0x20 - Shift B left preserving sign
{
    GBC_CPU_Register.B = GBC_CPU_EX_SLA(GBC_CPU_Register.B);
}

void GBC_CPU_EX_SLA_C()                     // 0x21 - Shift C left preserving sign
{
    GBC_CPU_Register.C = GBC_CPU_EX_SLA(GBC_CPU_Register.C);
}

void GBC_CPU_EX_SLA_D()                     // 0x22 - Shift D left preserving sign
{
    GBC_CPU_Register.D = GBC_CPU_EX_SLA(GBC_CPU_Register.D);
}

void GBC_CPU_EX_SLA_E()                     // 0x23 - Shift E left preserving sign
{
    GBC_CPU_Register.E = GBC_CPU_EX_SLA(GBC_CPU_Register.E);
}

void GBC_CPU_EX_SLA_H()                     // 0x24 - Shift H left preserving sign
{
    GBC_CPU_Register.H = GBC_CPU_EX_SLA(GBC_CPU_Register.H);
}

void GBC_CPU_EX_SLA_L()                     // 0x25 - Shift L left preserving sign
{
    GBC_CPU_Register.L = GBC_CPU_EX_SLA(GBC_CPU_Register.L);
}

void GBC_CPU_EX_SLA_HLP()                   // 0x26 - Shift value pointed by HL left preserving sign
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_SLA(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_SLA_A()                     // 0x27 - Shift A left preserving sign
{
    GBC_CPU_Register.A = GBC_CPU_EX_SLA(GBC_CPU_Register.A);
}

void GBC_CPU_EX_SRA_B()                     // 0x28 - Shift B right preserving sign
{
    GBC_CPU_Register.B = GBC_CPU_EX_SRA(GBC_CPU_Register.B);
}

void GBC_CPU_EX_SRA_C()                     // 0x29 - Shift C right preserving sign
{
    GBC_CPU_Register.C = GBC_CPU_EX_SRA(GBC_CPU_Register.C);
}

void GBC_CPU_EX_SRA_D()                     // 0x2A - Shift D right preserving sign
{
    GBC_CPU_Register.D = GBC_CPU_EX_SRA(GBC_CPU_Register.D);
}

void GBC_CPU_EX_SRA_E()                     // 0x2B - Shift E right preserving sign
{
    GBC_CPU_Register.E = GBC_CPU_EX_SRA(GBC_CPU_Register.E);
}

void GBC_CPU_EX_SRA_H()                     // 0x2C - Shift H right preserving sign
{
    GBC_CPU_Register.H = GBC_CPU_EX_SRA(GBC_CPU_Register.H);
}

void GBC_CPU_EX_SRA_L()                     // 0x2D - Shift L right preserving sign
{
    GBC_CPU_Register.L = GBC_CPU_EX_SRA(GBC_CPU_Register.L);
}

void GBC_CPU_EX_SRA_HLP()                   // 0x2E - Shift value pointed by HL right preserving sign
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_SRA(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_SRA_A()                     // 0x2F - Shift A right preserving sign
{
    GBC_CPU_Register.A = GBC_CPU_EX_SRA(GBC_CPU_Register.A);
}

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_EX_Instruction_t GBC_CPU_EX_Instructions[48] =
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
    GBC_CPU_EX_RL_B,    GBC_CPU_TICKS_8,  // 0x10 - Rotate B left
    GBC_CPU_EX_RL_C,    GBC_CPU_TICKS_8,  // 0x11 - Rotate C left
    GBC_CPU_EX_RL_D,    GBC_CPU_TICKS_8,  // 0x12 - Rotate D left
    GBC_CPU_EX_RL_E,    GBC_CPU_TICKS_8,  // 0x13 - Rotate E left
    GBC_CPU_EX_RL_H,    GBC_CPU_TICKS_8,  // 0x14 - Rotate H left
    GBC_CPU_EX_RL_L,    GBC_CPU_TICKS_8,  // 0x15 - Rotate L left
    GBC_CPU_EX_RL_HLP,  GBC_CPU_TICKS_16, // 0x16 - Rotate value pointed by HL left
    GBC_CPU_EX_RL_A,    GBC_CPU_TICKS_8,  // 0x17 - Rotate A left
    GBC_CPU_EX_RR_B,    GBC_CPU_TICKS_8,  // 0x18 - Rotate B right
    GBC_CPU_EX_RR_C,    GBC_CPU_TICKS_8,  // 0x19 - Rotate C right
    GBC_CPU_EX_RR_D,    GBC_CPU_TICKS_8,  // 0x1A - Rotate D right
    GBC_CPU_EX_RR_E,    GBC_CPU_TICKS_8,  // 0x1B - Rotate E right
    GBC_CPU_EX_RR_H,    GBC_CPU_TICKS_8,  // 0x1C - Rotate H right
    GBC_CPU_EX_RR_L,    GBC_CPU_TICKS_8,  // 0x1D - Rotate L right
    GBC_CPU_EX_RR_HLP,  GBC_CPU_TICKS_16, // 0x1E - Rotate value pointed by HL right
    GBC_CPU_EX_RR_A,    GBC_CPU_TICKS_8,  // 0x1F - Rotate A right
    GBC_CPU_EX_SLA_B,   GBC_CPU_TICKS_8,  // 0x20 - Shift B left preserving sign
    GBC_CPU_EX_SLA_C,   GBC_CPU_TICKS_8,  // 0x21 - Shift C left preserving sign
    GBC_CPU_EX_SLA_D,   GBC_CPU_TICKS_8,  // 0x22 - Shift D left preserving sign
    GBC_CPU_EX_SLA_E,   GBC_CPU_TICKS_8,  // 0x23 - Shift E left preserving sign
    GBC_CPU_EX_SLA_H,   GBC_CPU_TICKS_8,  // 0x24 - Shift H left preserving sign
    GBC_CPU_EX_SLA_L,   GBC_CPU_TICKS_8,  // 0x25 - Shift L left preserving sign
    GBC_CPU_EX_SLA_HLP, GBC_CPU_TICKS_16, // 0x26 - Shift value pointed by HL left preserving sign
    GBC_CPU_EX_SLA_A,   GBC_CPU_TICKS_8,  // 0x27 - Shift A left preserving sign
    GBC_CPU_EX_SRA_B,   GBC_CPU_TICKS_8,  // 0x28 - Shift B right preserving sign
    GBC_CPU_EX_SRA_C,   GBC_CPU_TICKS_8,  // 0x29 - Shift C right preserving sign
    GBC_CPU_EX_SRA_D,   GBC_CPU_TICKS_8,  // 0x2A - Shift D right preserving sign
    GBC_CPU_EX_SRA_E,   GBC_CPU_TICKS_8,  // 0x2B - Shift E right preserving sign
    GBC_CPU_EX_SRA_H,   GBC_CPU_TICKS_8,  // 0x2C - Shift H right preserving sign
    GBC_CPU_EX_SRA_L,   GBC_CPU_TICKS_8,  // 0x2D - Shift L right preserving sign
    GBC_CPU_EX_SRA_HLP, GBC_CPU_TICKS_16, // 0x2E - Shift value pointed by HL right preserving sign
    GBC_CPU_EX_SRA_A,   GBC_CPU_TICKS_8,  // 0x2F - Shift A right preserving sign
};
