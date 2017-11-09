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

uint8_t GBC_CPU_EX_SWAP(uint8_t value)              // Swap upper & lower nibbles of value
{
    value = ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);

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

uint8_t GBC_CPU_EX_SRL(uint8_t value)               // Shift value right into carry. MSB set to 0.
{
    if (value & 0x01)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    value >>= 1;

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

void GBC_CPU_EX_BIT(uint8_t bit, uint8_t value)     // Test bit b in value
{
    // Carry flag not affected

    if (value & bit)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_ZERO);
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);
    }
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

void GBC_CPU_EX_SWAP_B()                    // 0x30 - Swap nibbles in B
{
    GBC_CPU_Register.B = GBC_CPU_EX_SWAP(GBC_CPU_Register.B);
}

void GBC_CPU_EX_SWAP_C()                    // 0x31 - Swap nibbles in C
{
    GBC_CPU_Register.C = GBC_CPU_EX_SWAP(GBC_CPU_Register.C);
}

void GBC_CPU_EX_SWAP_D()                    // 0x32 - Swap nibbles in D
{
    GBC_CPU_Register.D = GBC_CPU_EX_SWAP(GBC_CPU_Register.D);
}

void GBC_CPU_EX_SWAP_E()                    // 0x33 - Swap nibbles in E
{
    GBC_CPU_Register.E = GBC_CPU_EX_SWAP(GBC_CPU_Register.E);
}

void GBC_CPU_EX_SWAP_H()                    // 0x34 - Swap nibbles in H
{
    GBC_CPU_Register.H = GBC_CPU_EX_SWAP(GBC_CPU_Register.H);
}

void GBC_CPU_EX_SWAP_L()                    // 0x35 - Swap nibbles in L
{
    GBC_CPU_Register.L = GBC_CPU_EX_SWAP(GBC_CPU_Register.L);
}

void GBC_CPU_EX_SWAP_HLP()                  // 0x36 - Swap nibbles in value pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_SWAP(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_SWAP_A()                    // 0x37 - Swap nibbles in A
{
    GBC_CPU_Register.A = GBC_CPU_EX_SWAP(GBC_CPU_Register.A);
}

void GBC_CPU_EX_SRL_B()                     // 0x38 - Shift B right
{
    GBC_CPU_Register.B = GBC_CPU_EX_SRL(GBC_CPU_Register.B);
}

void GBC_CPU_EX_SRL_C()                     // 0x39 - Shift C right
{
    GBC_CPU_Register.C = GBC_CPU_EX_SRL(GBC_CPU_Register.C);
}

void GBC_CPU_EX_SRL_D()                     // 0x3A - Shift D right
{
    GBC_CPU_Register.D = GBC_CPU_EX_SRL(GBC_CPU_Register.D);
}

void GBC_CPU_EX_SRL_E()                     // 0x3B - Shift E right
{
    GBC_CPU_Register.E = GBC_CPU_EX_SRL(GBC_CPU_Register.E);
}

void GBC_CPU_EX_SRL_H()                     // 0x3C - Shift H right
{
    GBC_CPU_Register.H = GBC_CPU_EX_SRL(GBC_CPU_Register.H);
}

void GBC_CPU_EX_SRL_L()                     // 0x3D - Shift L right
{
    GBC_CPU_Register.L = GBC_CPU_EX_SRL(GBC_CPU_Register.L);
}

void GBC_CPU_EX_SRL_HLP()                   // 0x3E - Shift value pointed by HL right
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_EX_SRL(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_EX_SRL_A()                     // 0x3F - Shift A right
{
    GBC_CPU_Register.A = GBC_CPU_EX_SRL(GBC_CPU_Register.A);
}

void GBC_CPU_EX_BIT_0_B()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.B);                    } // 0x40 - Test bit 0 of B
void GBC_CPU_EX_BIT_0_C()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.C);                    } // 0x41 - Test bit 0 of C
void GBC_CPU_EX_BIT_0_D()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.D);                    } // 0x42 - Test bit 0 of D
void GBC_CPU_EX_BIT_0_E()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.E);                    } // 0x43 - Test bit 0 of E
void GBC_CPU_EX_BIT_0_H()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.H);                    } // 0x44 - Test bit 0 of H
void GBC_CPU_EX_BIT_0_L()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.L);                    } // 0x45 - Test bit 0 of L
void GBC_CPU_EX_BIT_0_HLP()                                                                    // 0x46 - Test bit 0 of value pointed by HL
{
    if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
    {
        GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
        return;
    }

    GBC_CPU_EX_BIT(1 << 0, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}
void GBC_CPU_EX_BIT_0_A()   { GBC_CPU_EX_BIT(1 << 0, GBC_CPU_Register.A);                    } // 0x47 - Test bit 0 of A

void GBC_CPU_EX_BIT_1_B()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.B);                    } // 0x48 - Test bit 1 of B
void GBC_CPU_EX_BIT_1_C()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.C);                    } // 0x49 - Test bit 1 of C
void GBC_CPU_EX_BIT_1_D()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.D);                    } // 0x4A - Test bit 1 of D
void GBC_CPU_EX_BIT_1_E()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.E);                    } // 0x4B - Test bit 1 of E
void GBC_CPU_EX_BIT_1_H()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.H);                    } // 0x4C - Test bit 1 of H
void GBC_CPU_EX_BIT_1_L()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.L);                    } // 0x4D - Test bit 1 of L
void GBC_CPU_EX_BIT_1_HLP()                                                                    // 0x4E - Test bit 1 of value pointed by HL
{
    if (GBC_CPU_MemoryAccessDelayState == GBC_CPU_MEMORY_ACCESS_DELAY_STATE_NONE)
    {
        GBC_CPU_MemoryAccessDelayState = GBC_CPU_MEMORY_ACCESS_DELAY_STATE_08_TICKS_LEFT;
        return;
    }

    GBC_CPU_EX_BIT(1 << 1, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}
void GBC_CPU_EX_BIT_1_A()   { GBC_CPU_EX_BIT(1 << 1, GBC_CPU_Register.A);                    } // 0x4F - Test bit 1 of A

void GBC_CPU_EX_BIT_2_B()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.B);                    } // 0x50 - Test bit 2 of B
void GBC_CPU_EX_BIT_2_C()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.C);                    } // 0x51 - Test bit 2 of C
void GBC_CPU_EX_BIT_2_D()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.D);                    } // 0x52 - Test bit 2 of D
void GBC_CPU_EX_BIT_2_E()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.E);                    } // 0x53 - Test bit 2 of E
void GBC_CPU_EX_BIT_2_H()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.H);                    } // 0x54 - Test bit 2 of H
void GBC_CPU_EX_BIT_2_L()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.L);                    } // 0x55 - Test bit 2 of L
void GBC_CPU_EX_BIT_2_HLP() { GBC_CPU_EX_BIT(1 << 2, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x56 - Test bit 2 of value pointed by HL
void GBC_CPU_EX_BIT_2_A()   { GBC_CPU_EX_BIT(1 << 2, GBC_CPU_Register.A);                    } // 0x57 - Test bit 2 of A

void GBC_CPU_EX_BIT_3_B()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.B);                    } // 0x58 - Test bit 3 of B
void GBC_CPU_EX_BIT_3_C()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.C);                    } // 0x59 - Test bit 3 of C
void GBC_CPU_EX_BIT_3_D()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.D);                    } // 0x5A - Test bit 3 of D
void GBC_CPU_EX_BIT_3_E()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.E);                    } // 0x5B - Test bit 3 of E
void GBC_CPU_EX_BIT_3_H()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.H);                    } // 0x5C - Test bit 3 of H
void GBC_CPU_EX_BIT_3_L()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.L);                    } // 0x5D - Test bit 3 of L
void GBC_CPU_EX_BIT_3_HLP() { GBC_CPU_EX_BIT(1 << 3, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x5E - Test bit 3 of value pointed by HL
void GBC_CPU_EX_BIT_3_A()   { GBC_CPU_EX_BIT(1 << 3, GBC_CPU_Register.A);                    } // 0x5F - Test bit 3 of A

void GBC_CPU_EX_BIT_4_B()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.B);                    } // 0x60 - Test bit 4 of B
void GBC_CPU_EX_BIT_4_C()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.C);                    } // 0x61 - Test bit 4 of C
void GBC_CPU_EX_BIT_4_D()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.D);                    } // 0x62 - Test bit 4 of D
void GBC_CPU_EX_BIT_4_E()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.E);                    } // 0x63 - Test bit 4 of E
void GBC_CPU_EX_BIT_4_H()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.H);                    } // 0x64 - Test bit 4 of H
void GBC_CPU_EX_BIT_4_L()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.L);                    } // 0x65 - Test bit 4 of L
void GBC_CPU_EX_BIT_4_HLP() { GBC_CPU_EX_BIT(1 << 4, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x66 - Test bit 4 of value pointed by HL
void GBC_CPU_EX_BIT_4_A()   { GBC_CPU_EX_BIT(1 << 4, GBC_CPU_Register.A);                    } // 0x67 - Test bit 4 of A

void GBC_CPU_EX_BIT_5_B()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.B);                    } // 0x68 - Test bit 5 of B
void GBC_CPU_EX_BIT_5_C()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.C);                    } // 0x69 - Test bit 5 of C
void GBC_CPU_EX_BIT_5_D()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.D);                    } // 0x6A - Test bit 5 of D
void GBC_CPU_EX_BIT_5_E()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.E);                    } // 0x6B - Test bit 5 of E
void GBC_CPU_EX_BIT_5_H()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.H);                    } // 0x6C - Test bit 5 of H
void GBC_CPU_EX_BIT_5_L()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.L);                    } // 0x6D - Test bit 5 of L
void GBC_CPU_EX_BIT_5_HLP() { GBC_CPU_EX_BIT(1 << 5, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x6E - Test bit 5 of value pointed by HL
void GBC_CPU_EX_BIT_5_A()   { GBC_CPU_EX_BIT(1 << 5, GBC_CPU_Register.A);                    } // 0x6F - Test bit 5 of A

void GBC_CPU_EX_BIT_6_B()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.B);                    } // 0x70 - Test bit 6 of B
void GBC_CPU_EX_BIT_6_C()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.C);                    } // 0x71 - Test bit 6 of C
void GBC_CPU_EX_BIT_6_D()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.D);                    } // 0x72 - Test bit 6 of D
void GBC_CPU_EX_BIT_6_E()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.E);                    } // 0x73 - Test bit 6 of E
void GBC_CPU_EX_BIT_6_H()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.H);                    } // 0x74 - Test bit 6 of H
void GBC_CPU_EX_BIT_6_L()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.L);                    } // 0x75 - Test bit 6 of L
void GBC_CPU_EX_BIT_6_HLP() { GBC_CPU_EX_BIT(1 << 6, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x76 - Test bit 6 of value pointed by HL
void GBC_CPU_EX_BIT_6_A()   { GBC_CPU_EX_BIT(1 << 6, GBC_CPU_Register.A);                    } // 0x77 - Test bit 6 of A

void GBC_CPU_EX_BIT_7_B()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.B);                    } // 0x78 - Test bit 7 of B
void GBC_CPU_EX_BIT_7_C()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.C);                    } // 0x79 - Test bit 7 of C
void GBC_CPU_EX_BIT_7_D()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.D);                    } // 0x7A - Test bit 7 of D
void GBC_CPU_EX_BIT_7_E()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.E);                    } // 0x7B - Test bit 7 of E
void GBC_CPU_EX_BIT_7_H()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.H);                    } // 0x7C - Test bit 7 of H
void GBC_CPU_EX_BIT_7_L()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.L);                    } // 0x7D - Test bit 7 of L
void GBC_CPU_EX_BIT_7_HLP() { GBC_CPU_EX_BIT(1 << 7, GBC_MMU_ReadByte(GBC_CPU_Register.HL)); } // 0x7E - Test bit 7 of value pointed by HL
void GBC_CPU_EX_BIT_7_A()   { GBC_CPU_EX_BIT(1 << 7, GBC_CPU_Register.A);                    } // 0x7F - Test bit 7 of A

void GBC_CPU_EX_RES_0_B()  { GBC_CPU_Register.B &= ~(1 << 0);                                } // 0x80 - Clear (reset) bit 0 of B
void GBC_CPU_EX_RES_0_C()  { GBC_CPU_Register.C &= ~(1 << 0);                                } // 0x81 - Clear (reset) bit 0 of C
void GBC_CPU_EX_RES_0_D()  { GBC_CPU_Register.D &= ~(1 << 0);                                } // 0x82 - Clear (reset) bit 0 of D
void GBC_CPU_EX_RES_0_E()  { GBC_CPU_Register.E &= ~(1 << 0);                                } // 0x83 - Clear (reset) bit 0 of E
void GBC_CPU_EX_RES_0_H()  { GBC_CPU_Register.H &= ~(1 << 0);                                } // 0x84 - Clear (reset) bit 0 of H
void GBC_CPU_EX_RES_0_L()  { GBC_CPU_Register.L &= ~(1 << 0);                                } // 0x85 - Clear (reset) bit 0 of L
void GBC_CPU_EX_RES_0_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 0));                               } // 0x86 - Clear (reset) bit 0 of value pointed by HL
void GBC_CPU_EX_RES_0_A()  { GBC_CPU_Register.A &= ~(1 << 0);                                } // 0x87 - Clear (reset) bit 0 of A

void GBC_CPU_EX_RES_1_B()  { GBC_CPU_Register.B &= ~(1 << 1);                                } // 0x88 - Clear (reset) bit 1 of B
void GBC_CPU_EX_RES_1_C()  { GBC_CPU_Register.C &= ~(1 << 1);                                } // 0x89 - Clear (reset) bit 1 of C
void GBC_CPU_EX_RES_1_D()  { GBC_CPU_Register.D &= ~(1 << 1);                                } // 0x8A - Clear (reset) bit 1 of D
void GBC_CPU_EX_RES_1_E()  { GBC_CPU_Register.E &= ~(1 << 1);                                } // 0x8B - Clear (reset) bit 1 of E
void GBC_CPU_EX_RES_1_H()  { GBC_CPU_Register.H &= ~(1 << 1);                                } // 0x8C - Clear (reset) bit 1 of H
void GBC_CPU_EX_RES_1_L()  { GBC_CPU_Register.L &= ~(1 << 1);                                } // 0x8D - Clear (reset) bit 1 of L
void GBC_CPU_EX_RES_1_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 1));                               } // 0x8E - Clear (reset) bit 1 of value pointed by HL
void GBC_CPU_EX_RES_1_A()  { GBC_CPU_Register.A &= ~(1 << 1);                                } // 0x8F - Clear (reset) bit 1 of A

void GBC_CPU_EX_RES_2_B()  { GBC_CPU_Register.B &= ~(1 << 2);                                } // 0x90 - Clear (reset) bit 2 of B
void GBC_CPU_EX_RES_2_C()  { GBC_CPU_Register.C &= ~(1 << 2);                                } // 0x91 - Clear (reset) bit 2 of C
void GBC_CPU_EX_RES_2_D()  { GBC_CPU_Register.D &= ~(1 << 2);                                } // 0x92 - Clear (reset) bit 2 of D
void GBC_CPU_EX_RES_2_E()  { GBC_CPU_Register.E &= ~(1 << 2);                                } // 0x93 - Clear (reset) bit 2 of E
void GBC_CPU_EX_RES_2_H()  { GBC_CPU_Register.H &= ~(1 << 2);                                } // 0x94 - Clear (reset) bit 2 of H
void GBC_CPU_EX_RES_2_L()  { GBC_CPU_Register.L &= ~(1 << 2);                                } // 0x95 - Clear (reset) bit 2 of L
void GBC_CPU_EX_RES_2_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 2));                               } // 0x96 - Clear (reset) bit 2 of value pointed by HL
void GBC_CPU_EX_RES_2_A()  { GBC_CPU_Register.A &= ~(1 << 2);                                } // 0x97 - Clear (reset) bit 2 of A

void GBC_CPU_EX_RES_3_B()  { GBC_CPU_Register.B &= ~(1 << 3);                                } // 0x98 - Clear (reset) bit 3 of B
void GBC_CPU_EX_RES_3_C()  { GBC_CPU_Register.C &= ~(1 << 3);                                } // 0x99 - Clear (reset) bit 3 of C
void GBC_CPU_EX_RES_3_D()  { GBC_CPU_Register.D &= ~(1 << 3);                                } // 0x9A - Clear (reset) bit 3 of D
void GBC_CPU_EX_RES_3_E()  { GBC_CPU_Register.E &= ~(1 << 3);                                } // 0x9B - Clear (reset) bit 3 of E
void GBC_CPU_EX_RES_3_H()  { GBC_CPU_Register.H &= ~(1 << 3);                                } // 0x9C - Clear (reset) bit 3 of H
void GBC_CPU_EX_RES_3_L()  { GBC_CPU_Register.L &= ~(1 << 3);                                } // 0x9D - Clear (reset) bit 3 of L
void GBC_CPU_EX_RES_3_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 3));                               } // 0x9E - Clear (reset) bit 3 of value pointed by HL
void GBC_CPU_EX_RES_3_A()  { GBC_CPU_Register.A &= ~(1 << 3);                                } // 0x9F - Clear (reset) bit 3 of A

void GBC_CPU_EX_RES_4_B()  { GBC_CPU_Register.B &= ~(1 << 4);                                } // 0xA0 - Clear (reset) bit 4 of B
void GBC_CPU_EX_RES_4_C()  { GBC_CPU_Register.C &= ~(1 << 4);                                } // 0xA1 - Clear (reset) bit 4 of C
void GBC_CPU_EX_RES_4_D()  { GBC_CPU_Register.D &= ~(1 << 4);                                } // 0xA2 - Clear (reset) bit 4 of D
void GBC_CPU_EX_RES_4_E()  { GBC_CPU_Register.E &= ~(1 << 4);                                } // 0xA3 - Clear (reset) bit 4 of E
void GBC_CPU_EX_RES_4_H()  { GBC_CPU_Register.H &= ~(1 << 4);                                } // 0xA4 - Clear (reset) bit 4 of H
void GBC_CPU_EX_RES_4_L()  { GBC_CPU_Register.L &= ~(1 << 4);                                } // 0xA5 - Clear (reset) bit 4 of L
void GBC_CPU_EX_RES_4_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 4));                               } // 0xA6 - Clear (reset) bit 4 of value pointed by HL
void GBC_CPU_EX_RES_4_A()  { GBC_CPU_Register.A &= ~(1 << 4);                                } // 0xA7 - Clear (reset) bit 4 of A

void GBC_CPU_EX_RES_5_B()  { GBC_CPU_Register.B &= ~(1 << 5);                                } // 0xA8 - Clear (reset) bit 5 of B
void GBC_CPU_EX_RES_5_C()  { GBC_CPU_Register.C &= ~(1 << 5);                                } // 0xA9 - Clear (reset) bit 5 of C
void GBC_CPU_EX_RES_5_D()  { GBC_CPU_Register.D &= ~(1 << 5);                                } // 0xAA - Clear (reset) bit 5 of D
void GBC_CPU_EX_RES_5_E()  { GBC_CPU_Register.E &= ~(1 << 5);                                } // 0xAB - Clear (reset) bit 5 of E
void GBC_CPU_EX_RES_5_H()  { GBC_CPU_Register.H &= ~(1 << 5);                                } // 0xAC - Clear (reset) bit 5 of H
void GBC_CPU_EX_RES_5_L()  { GBC_CPU_Register.L &= ~(1 << 5);                                } // 0xAD - Clear (reset) bit 5 of L
void GBC_CPU_EX_RES_5_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 5));                               } // 0xAE - Clear (reset) bit 5 of value pointed by HL
void GBC_CPU_EX_RES_5_A()  { GBC_CPU_Register.A &= ~(1 << 5);                                } // 0xAF - Clear (reset) bit 5 of A

void GBC_CPU_EX_RES_6_B()  { GBC_CPU_Register.B &= ~(1 << 6);                                } // 0xB0 - Clear (reset) bit 6 of B
void GBC_CPU_EX_RES_6_C()  { GBC_CPU_Register.C &= ~(1 << 6);                                } // 0xB1 - Clear (reset) bit 6 of C
void GBC_CPU_EX_RES_6_D()  { GBC_CPU_Register.D &= ~(1 << 6);                                } // 0xB2 - Clear (reset) bit 6 of D
void GBC_CPU_EX_RES_6_E()  { GBC_CPU_Register.E &= ~(1 << 6);                                } // 0xB3 - Clear (reset) bit 6 of E
void GBC_CPU_EX_RES_6_H()  { GBC_CPU_Register.H &= ~(1 << 6);                                } // 0xB4 - Clear (reset) bit 6 of H
void GBC_CPU_EX_RES_6_L()  { GBC_CPU_Register.L &= ~(1 << 6);                                } // 0xB5 - Clear (reset) bit 6 of L
void GBC_CPU_EX_RES_6_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 6));                               } // 0xB6 - Clear (reset) bit 6 of value pointed by HL
void GBC_CPU_EX_RES_6_A()  { GBC_CPU_Register.A &= ~(1 << 6);                                } // 0xB7 - Clear (reset) bit 6 of A

void GBC_CPU_EX_RES_7_B()  { GBC_CPU_Register.B &= ~(1 << 7);                                } // 0xB8 - Clear (reset) bit 7 of B
void GBC_CPU_EX_RES_7_C()  { GBC_CPU_Register.C &= ~(1 << 7);                                } // 0xB9 - Clear (reset) bit 7 of C
void GBC_CPU_EX_RES_7_D()  { GBC_CPU_Register.D &= ~(1 << 7);                                } // 0xBA - Clear (reset) bit 7 of D
void GBC_CPU_EX_RES_7_E()  { GBC_CPU_Register.E &= ~(1 << 7);                                } // 0xBB - Clear (reset) bit 7 of E
void GBC_CPU_EX_RES_7_H()  { GBC_CPU_Register.H &= ~(1 << 7);                                } // 0xBC - Clear (reset) bit 7 of H
void GBC_CPU_EX_RES_7_L()  { GBC_CPU_Register.L &= ~(1 << 7);                                } // 0xBD - Clear (reset) bit 7 of L
void GBC_CPU_EX_RES_7_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) & ~(1 << 7));                               } // 0xBE - Clear (reset) bit 7 of value pointed by HL
void GBC_CPU_EX_RES_7_A()  { GBC_CPU_Register.A &= ~(1 << 7);                                } // 0xBF - Clear (reset) bit 7 of A

void GBC_CPU_EX_SET_0_B()  { GBC_CPU_Register.B |= (1 << 0);                                  } // 0xC0 - Set bit 0 of B
void GBC_CPU_EX_SET_0_C()  { GBC_CPU_Register.C |= (1 << 0);                                  } // 0xC1 - Set bit 0 of C
void GBC_CPU_EX_SET_0_D()  { GBC_CPU_Register.D |= (1 << 0);                                  } // 0xC2 - Set bit 0 of D
void GBC_CPU_EX_SET_0_E()  { GBC_CPU_Register.E |= (1 << 0);                                  } // 0xC3 - Set bit 0 of E
void GBC_CPU_EX_SET_0_H()  { GBC_CPU_Register.H |= (1 << 0);                                  } // 0xC4 - Set bit 0 of H
void GBC_CPU_EX_SET_0_L()  { GBC_CPU_Register.L |= (1 << 0);                                  } // 0xC5 - Set bit 0 of L
void GBC_CPU_EX_SET_0_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 0));                                 } // 0xC6 - Set bit 0 of value pointed by HL
void GBC_CPU_EX_SET_0_A()  { GBC_CPU_Register.A |= (1 << 0);                                  } // 0xC7 - Set bit 0 of A

void GBC_CPU_EX_SET_1_B()  { GBC_CPU_Register.B |= (1 << 1);                                  } // 0xC8 - Set bit 1 of B
void GBC_CPU_EX_SET_1_C()  { GBC_CPU_Register.C |= (1 << 1);                                  } // 0xC9 - Set bit 1 of C
void GBC_CPU_EX_SET_1_D()  { GBC_CPU_Register.D |= (1 << 1);                                  } // 0xCA - Set bit 1 of D
void GBC_CPU_EX_SET_1_E()  { GBC_CPU_Register.E |= (1 << 1);                                  } // 0xCB - Set bit 1 of E
void GBC_CPU_EX_SET_1_H()  { GBC_CPU_Register.H |= (1 << 1);                                  } // 0xCC - Set bit 1 of H
void GBC_CPU_EX_SET_1_L()  { GBC_CPU_Register.L |= (1 << 1);                                  } // 0xCD - Set bit 1 of L
void GBC_CPU_EX_SET_1_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 1));                                 } // 0xCE - Set bit 1 of value pointed by HL
void GBC_CPU_EX_SET_1_A()  { GBC_CPU_Register.A |= (1 << 1);                                  } // 0xCF - Set bit 1 of A

void GBC_CPU_EX_SET_2_B()  { GBC_CPU_Register.B |= (1 << 2);                                  } // 0xD0 - Set bit 2 of B
void GBC_CPU_EX_SET_2_C()  { GBC_CPU_Register.C |= (1 << 2);                                  } // 0xD1 - Set bit 2 of C
void GBC_CPU_EX_SET_2_D()  { GBC_CPU_Register.D |= (1 << 2);                                  } // 0xD2 - Set bit 2 of D
void GBC_CPU_EX_SET_2_E()  { GBC_CPU_Register.E |= (1 << 2);                                  } // 0xD3 - Set bit 2 of E
void GBC_CPU_EX_SET_2_H()  { GBC_CPU_Register.H |= (1 << 2);                                  } // 0xD4 - Set bit 2 of H
void GBC_CPU_EX_SET_2_L()  { GBC_CPU_Register.L |= (1 << 2);                                  } // 0xD5 - Set bit 2 of L
void GBC_CPU_EX_SET_2_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 2));                                 } // 0xD6 - Set bit 2 of value pointed by HL
void GBC_CPU_EX_SET_2_A()  { GBC_CPU_Register.A |= (1 << 2);                                  } // 0xD7 - Set bit 2 of A

void GBC_CPU_EX_SET_3_B()  { GBC_CPU_Register.B |= (1 << 3);                                  } // 0xD8 - Set bit 3 of B
void GBC_CPU_EX_SET_3_C()  { GBC_CPU_Register.C |= (1 << 3);                                  } // 0xD9 - Set bit 3 of C
void GBC_CPU_EX_SET_3_D()  { GBC_CPU_Register.D |= (1 << 3);                                  } // 0xDA - Set bit 3 of D
void GBC_CPU_EX_SET_3_E()  { GBC_CPU_Register.E |= (1 << 3);                                  } // 0xDB - Set bit 3 of E
void GBC_CPU_EX_SET_3_H()  { GBC_CPU_Register.H |= (1 << 3);                                  } // 0xDC - Set bit 3 of H
void GBC_CPU_EX_SET_3_L()  { GBC_CPU_Register.L |= (1 << 3);                                  } // 0xDD - Set bit 3 of L
void GBC_CPU_EX_SET_3_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 3));                                 } // 0xDE - Set bit 3 of value pointed by HL
void GBC_CPU_EX_SET_3_A()  { GBC_CPU_Register.A |= (1 << 3);                                  } // 0xDF - Set bit 3 of A

void GBC_CPU_EX_SET_4_B()  { GBC_CPU_Register.B |= (1 << 4);                                  } // 0xD0 - Set bit 4 of B
void GBC_CPU_EX_SET_4_C()  { GBC_CPU_Register.C |= (1 << 4);                                  } // 0xD1 - Set bit 4 of C
void GBC_CPU_EX_SET_4_D()  { GBC_CPU_Register.D |= (1 << 4);                                  } // 0xD2 - Set bit 4 of D
void GBC_CPU_EX_SET_4_E()  { GBC_CPU_Register.E |= (1 << 4);                                  } // 0xD3 - Set bit 4 of E
void GBC_CPU_EX_SET_4_H()  { GBC_CPU_Register.H |= (1 << 4);                                  } // 0xD4 - Set bit 4 of H
void GBC_CPU_EX_SET_4_L()  { GBC_CPU_Register.L |= (1 << 4);                                  } // 0xD5 - Set bit 4 of L
void GBC_CPU_EX_SET_4_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 4));                                 } // 0xD6 - Set bit 4 of value pointed by HL
void GBC_CPU_EX_SET_4_A()  { GBC_CPU_Register.A |= (1 << 4);                                  } // 0xD7 - Set bit 4 of A

void GBC_CPU_EX_SET_5_B()  { GBC_CPU_Register.B |= (1 << 5);                                  } // 0xE8 - Set bit 5 of B
void GBC_CPU_EX_SET_5_C()  { GBC_CPU_Register.C |= (1 << 5);                                  } // 0xE9 - Set bit 5 of C
void GBC_CPU_EX_SET_5_D()  { GBC_CPU_Register.D |= (1 << 5);                                  } // 0xEA - Set bit 5 of D
void GBC_CPU_EX_SET_5_E()  { GBC_CPU_Register.E |= (1 << 5);                                  } // 0xEB - Set bit 5 of E
void GBC_CPU_EX_SET_5_H()  { GBC_CPU_Register.H |= (1 << 5);                                  } // 0xEC - Set bit 5 of H
void GBC_CPU_EX_SET_5_L()  { GBC_CPU_Register.L |= (1 << 5);                                  } // 0xED - Set bit 5 of L
void GBC_CPU_EX_SET_5_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 5));                                 } // 0xEE - Set bit 5 of value pointed by HL
void GBC_CPU_EX_SET_5_A()  { GBC_CPU_Register.A |= (1 << 5);                                  } // 0xEF - Set bit 5 of A

void GBC_CPU_EX_SET_6_B()  { GBC_CPU_Register.B |= (1 << 6);                                  } // 0xF0 - Set bit 6 of B
void GBC_CPU_EX_SET_6_C()  { GBC_CPU_Register.C |= (1 << 6);                                  } // 0xF1 - Set bit 6 of C
void GBC_CPU_EX_SET_6_D()  { GBC_CPU_Register.D |= (1 << 6);                                  } // 0xF2 - Set bit 6 of D
void GBC_CPU_EX_SET_6_E()  { GBC_CPU_Register.E |= (1 << 6);                                  } // 0xF3 - Set bit 6 of E
void GBC_CPU_EX_SET_6_H()  { GBC_CPU_Register.H |= (1 << 6);                                  } // 0xF4 - Set bit 6 of H
void GBC_CPU_EX_SET_6_L()  { GBC_CPU_Register.L |= (1 << 6);                                  } // 0xF5 - Set bit 6 of L
void GBC_CPU_EX_SET_6_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 6));                                 } // 0xF6 - Set bit 6 of value pointed by HL
void GBC_CPU_EX_SET_6_A()  { GBC_CPU_Register.A |= (1 << 6);                                  } // 0xF7 - Set bit 6 of A

void GBC_CPU_EX_SET_7_B()  { GBC_CPU_Register.B |= (1 << 7);                                  } // 0xF8 - Set bit 7 of B
void GBC_CPU_EX_SET_7_C()  { GBC_CPU_Register.C |= (1 << 7);                                  } // 0xF9 - Set bit 7 of C
void GBC_CPU_EX_SET_7_D()  { GBC_CPU_Register.D |= (1 << 7);                                  } // 0xFA - Set bit 7 of D
void GBC_CPU_EX_SET_7_E()  { GBC_CPU_Register.E |= (1 << 7);                                  } // 0xFB - Set bit 7 of E
void GBC_CPU_EX_SET_7_H()  { GBC_CPU_Register.H |= (1 << 7);                                  } // 0xFC - Set bit 7 of H
void GBC_CPU_EX_SET_7_L()  { GBC_CPU_Register.L |= (1 << 7);                                  } // 0xFD - Set bit 7 of L
void GBC_CPU_EX_SET_7_HLP(){ GBC_MMU_WriteByte(GBC_CPU_Register.HL,
           GBC_MMU_ReadByte(GBC_CPU_Register.HL) | (1 << 7));                                 } // 0xFE - Set bit 7 of value pointed by HL
void GBC_CPU_EX_SET_7_A()  { GBC_CPU_Register.A |= (1 << 7);                                  } // 0xFF - Set bit 7 of A

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_EX_Instruction_t GBC_CPU_EX_Instructions[256] =
{
    { GBC_CPU_EX_RLC_B,     GBC_CPU_TICKS_8  }, // 0x00 - Rotate B left with carry
    { GBC_CPU_EX_RLC_C,     GBC_CPU_TICKS_8  }, // 0x01 - Rotate C left with carry
    { GBC_CPU_EX_RLC_D,     GBC_CPU_TICKS_8  }, // 0x02 - Rotate D left with carry
    { GBC_CPU_EX_RLC_E,     GBC_CPU_TICKS_8  }, // 0x03 - Rotate E left with carry
    { GBC_CPU_EX_RLC_H,     GBC_CPU_TICKS_8  }, // 0x04 - Rotate H left with carry
    { GBC_CPU_EX_RLC_L,     GBC_CPU_TICKS_8  }, // 0x05 - Rotate L left with carry
    { GBC_CPU_EX_RLC_HLP,   GBC_CPU_TICKS_16 }, // 0x06 - Rotate value pointed by HL left with carry
    { GBC_CPU_EX_RLC_A,     GBC_CPU_TICKS_8  }, // 0x07 - Rotate A left with carry
    { GBC_CPU_EX_RRC_B,     GBC_CPU_TICKS_8  }, // 0x08 - Rotate B right with carry
    { GBC_CPU_EX_RRC_C,     GBC_CPU_TICKS_8  }, // 0x09 - Rotate C right with carry
    { GBC_CPU_EX_RRC_D,     GBC_CPU_TICKS_8  }, // 0x0A - Rotate D right with carry
    { GBC_CPU_EX_RRC_E,     GBC_CPU_TICKS_8  }, // 0x0B - Rotate E right with carry
    { GBC_CPU_EX_RRC_H,     GBC_CPU_TICKS_8  }, // 0x0C - Rotate H right with carry
    { GBC_CPU_EX_RRC_L,     GBC_CPU_TICKS_8  }, // 0x0D - Rotate L right with carry
    { GBC_CPU_EX_RRC_HLP,   GBC_CPU_TICKS_16 }, // 0x0E - Rotate value pointed by HL right with carry
    { GBC_CPU_EX_RRC_A,     GBC_CPU_TICKS_8  }, // 0x0F - Rotate A right with carry
    { GBC_CPU_EX_RL_B,      GBC_CPU_TICKS_8  }, // 0x10 - Rotate B left
    { GBC_CPU_EX_RL_C,      GBC_CPU_TICKS_8  }, // 0x11 - Rotate C left
    { GBC_CPU_EX_RL_D,      GBC_CPU_TICKS_8  }, // 0x12 - Rotate D left
    { GBC_CPU_EX_RL_E,      GBC_CPU_TICKS_8  }, // 0x13 - Rotate E left
    { GBC_CPU_EX_RL_H,      GBC_CPU_TICKS_8  }, // 0x14 - Rotate H left
    { GBC_CPU_EX_RL_L,      GBC_CPU_TICKS_8  }, // 0x15 - Rotate L left
    { GBC_CPU_EX_RL_HLP,    GBC_CPU_TICKS_16 }, // 0x16 - Rotate value pointed by HL left
    { GBC_CPU_EX_RL_A,      GBC_CPU_TICKS_8  }, // 0x17 - Rotate A left
    { GBC_CPU_EX_RR_B,      GBC_CPU_TICKS_8  }, // 0x18 - Rotate B right
    { GBC_CPU_EX_RR_C,      GBC_CPU_TICKS_8  }, // 0x19 - Rotate C right
    { GBC_CPU_EX_RR_D,      GBC_CPU_TICKS_8  }, // 0x1A - Rotate D right
    { GBC_CPU_EX_RR_E,      GBC_CPU_TICKS_8  }, // 0x1B - Rotate E right
    { GBC_CPU_EX_RR_H,      GBC_CPU_TICKS_8  }, // 0x1C - Rotate H right
    { GBC_CPU_EX_RR_L,      GBC_CPU_TICKS_8  }, // 0x1D - Rotate L right
    { GBC_CPU_EX_RR_HLP,    GBC_CPU_TICKS_16 }, // 0x1E - Rotate value pointed by HL right
    { GBC_CPU_EX_RR_A,      GBC_CPU_TICKS_8  }, // 0x1F - Rotate A right
    { GBC_CPU_EX_SLA_B,     GBC_CPU_TICKS_8  }, // 0x20 - Shift B left preserving sign
    { GBC_CPU_EX_SLA_C,     GBC_CPU_TICKS_8  }, // 0x21 - Shift C left preserving sign
    { GBC_CPU_EX_SLA_D,     GBC_CPU_TICKS_8  }, // 0x22 - Shift D left preserving sign
    { GBC_CPU_EX_SLA_E,     GBC_CPU_TICKS_8  }, // 0x23 - Shift E left preserving sign
    { GBC_CPU_EX_SLA_H,     GBC_CPU_TICKS_8  }, // 0x24 - Shift H left preserving sign
    { GBC_CPU_EX_SLA_L,     GBC_CPU_TICKS_8  }, // 0x25 - Shift L left preserving sign
    { GBC_CPU_EX_SLA_HLP,   GBC_CPU_TICKS_16 }, // 0x26 - Shift value pointed by HL left preserving sign
    { GBC_CPU_EX_SLA_A,     GBC_CPU_TICKS_8  }, // 0x27 - Shift A left preserving sign
    { GBC_CPU_EX_SRA_B,     GBC_CPU_TICKS_8  }, // 0x28 - Shift B right preserving sign
    { GBC_CPU_EX_SRA_C,     GBC_CPU_TICKS_8  }, // 0x29 - Shift C right preserving sign
    { GBC_CPU_EX_SRA_D,     GBC_CPU_TICKS_8  }, // 0x2A - Shift D right preserving sign
    { GBC_CPU_EX_SRA_E,     GBC_CPU_TICKS_8  }, // 0x2B - Shift E right preserving sign
    { GBC_CPU_EX_SRA_H,     GBC_CPU_TICKS_8  }, // 0x2C - Shift H right preserving sign
    { GBC_CPU_EX_SRA_L,     GBC_CPU_TICKS_8  }, // 0x2D - Shift L right preserving sign
    { GBC_CPU_EX_SRA_HLP,   GBC_CPU_TICKS_16 }, // 0x2E - Shift value pointed by HL right preserving sign
    { GBC_CPU_EX_SRA_A,     GBC_CPU_TICKS_8  }, // 0x2F - Shift A right preserving sign
    { GBC_CPU_EX_SWAP_B,    GBC_CPU_TICKS_8  }, // 0x30 - Swap nibbles in B
    { GBC_CPU_EX_SWAP_C,    GBC_CPU_TICKS_8  }, // 0x31 - Swap nibbles in C
    { GBC_CPU_EX_SWAP_D,    GBC_CPU_TICKS_8  }, // 0x32 - Swap nibbles in D
    { GBC_CPU_EX_SWAP_E,    GBC_CPU_TICKS_8  }, // 0x33 - Swap nibbles in E
    { GBC_CPU_EX_SWAP_H,    GBC_CPU_TICKS_8  }, // 0x34 - Swap nibbles in H
    { GBC_CPU_EX_SWAP_L,    GBC_CPU_TICKS_8  }, // 0x35 - Swap nibbles in L
    { GBC_CPU_EX_SWAP_HLP,  GBC_CPU_TICKS_16 }, // 0x36 - Swap nibbles in value pointed by HL
    { GBC_CPU_EX_SWAP_A,    GBC_CPU_TICKS_8  }, // 0x37 - Swap nibbles in A
    { GBC_CPU_EX_SRL_B,     GBC_CPU_TICKS_8  }, // 0x38 - Shift B right
    { GBC_CPU_EX_SRL_C,     GBC_CPU_TICKS_8  }, // 0x39 - Shift B right
    { GBC_CPU_EX_SRL_D,     GBC_CPU_TICKS_8  }, // 0x3A - Shift B right
    { GBC_CPU_EX_SRL_E,     GBC_CPU_TICKS_8  }, // 0x3B - Shift B right
    { GBC_CPU_EX_SRL_H,     GBC_CPU_TICKS_8  }, // 0x3C - Shift B right
    { GBC_CPU_EX_SRL_L,     GBC_CPU_TICKS_8  }, // 0x3D - Shift B right
    { GBC_CPU_EX_SRL_HLP,   GBC_CPU_TICKS_16 }, // 0x3E - Shift B right
    { GBC_CPU_EX_SRL_A,     GBC_CPU_TICKS_8  }, // 0x3F - Shift B right
    { GBC_CPU_EX_BIT_0_B,   GBC_CPU_TICKS_8  }, // 0x40 - Test bit 0 of B
    { GBC_CPU_EX_BIT_0_C,   GBC_CPU_TICKS_8  }, // 0x41 - Test bit 0 of C
    { GBC_CPU_EX_BIT_0_D,   GBC_CPU_TICKS_8  }, // 0x42 - Test bit 0 of D
    { GBC_CPU_EX_BIT_0_E,   GBC_CPU_TICKS_8  }, // 0x43 - Test bit 0 of E
    { GBC_CPU_EX_BIT_0_H,   GBC_CPU_TICKS_8  }, // 0x44 - Test bit 0 of H
    { GBC_CPU_EX_BIT_0_L,   GBC_CPU_TICKS_8  }, // 0x45 - Test bit 0 of L
    { GBC_CPU_EX_BIT_0_HLP, GBC_CPU_TICKS_12 }, // 0x46 - Test bit 0 of value pointed by HL
    { GBC_CPU_EX_BIT_0_A,   GBC_CPU_TICKS_8  }, // 0x47 - Test bit 0 of A
    { GBC_CPU_EX_BIT_1_B,   GBC_CPU_TICKS_8  }, // 0x48 - Test bit 1 of B
    { GBC_CPU_EX_BIT_1_C,   GBC_CPU_TICKS_8  }, // 0x49 - Test bit 1 of C
    { GBC_CPU_EX_BIT_1_D,   GBC_CPU_TICKS_8  }, // 0x4A - Test bit 1 of D
    { GBC_CPU_EX_BIT_1_E,   GBC_CPU_TICKS_8  }, // 0x4B - Test bit 1 of E
    { GBC_CPU_EX_BIT_1_H,   GBC_CPU_TICKS_8  }, // 0x4C - Test bit 1 of H
    { GBC_CPU_EX_BIT_1_L,   GBC_CPU_TICKS_8  }, // 0x4D - Test bit 1 of L
    { GBC_CPU_EX_BIT_1_HLP, GBC_CPU_TICKS_12 }, // 0x4E - Test bit 1 of value pointed by HL
    { GBC_CPU_EX_BIT_1_A,   GBC_CPU_TICKS_8  }, // 0x4F - Test bit 1 of A
    { GBC_CPU_EX_BIT_2_B,   GBC_CPU_TICKS_8  }, // 0x50 - Test bit 2 of B
    { GBC_CPU_EX_BIT_2_C,   GBC_CPU_TICKS_8  }, // 0x51 - Test bit 2 of C
    { GBC_CPU_EX_BIT_2_D,   GBC_CPU_TICKS_8  }, // 0x52 - Test bit 2 of D
    { GBC_CPU_EX_BIT_2_E,   GBC_CPU_TICKS_8  }, // 0x53 - Test bit 2 of E
    { GBC_CPU_EX_BIT_2_H,   GBC_CPU_TICKS_8  }, // 0x54 - Test bit 2 of H
    { GBC_CPU_EX_BIT_2_L,   GBC_CPU_TICKS_8  }, // 0x55 - Test bit 2 of L
    { GBC_CPU_EX_BIT_2_HLP, GBC_CPU_TICKS_12 }, // 0x56 - Test bit 2 of value pointed by HL
    { GBC_CPU_EX_BIT_2_A,   GBC_CPU_TICKS_8  }, // 0x57 - Test bit 2 of A
    { GBC_CPU_EX_BIT_3_B,   GBC_CPU_TICKS_8  }, // 0x58 - Test bit 3 of B
    { GBC_CPU_EX_BIT_3_C,   GBC_CPU_TICKS_8  }, // 0x59 - Test bit 3 of C
    { GBC_CPU_EX_BIT_3_D,   GBC_CPU_TICKS_8  }, // 0x5A - Test bit 3 of D
    { GBC_CPU_EX_BIT_3_E,   GBC_CPU_TICKS_8  }, // 0x5B - Test bit 3 of E
    { GBC_CPU_EX_BIT_3_H,   GBC_CPU_TICKS_8  }, // 0x5C - Test bit 3 of H
    { GBC_CPU_EX_BIT_3_L,   GBC_CPU_TICKS_8  }, // 0x5D - Test bit 3 of L
    { GBC_CPU_EX_BIT_3_HLP, GBC_CPU_TICKS_12 }, // 0x5E - Test bit 3 of value pointed by HL
    { GBC_CPU_EX_BIT_3_A,   GBC_CPU_TICKS_8  }, // 0x5F - Test bit 3 of A
    { GBC_CPU_EX_BIT_4_B,   GBC_CPU_TICKS_8  }, // 0x60 - Test bit 4 of B
    { GBC_CPU_EX_BIT_4_C,   GBC_CPU_TICKS_8  }, // 0x61 - Test bit 4 of C
    { GBC_CPU_EX_BIT_4_D,   GBC_CPU_TICKS_8  }, // 0x62 - Test bit 4 of D
    { GBC_CPU_EX_BIT_4_E,   GBC_CPU_TICKS_8  }, // 0x63 - Test bit 4 of E
    { GBC_CPU_EX_BIT_4_H,   GBC_CPU_TICKS_8  }, // 0x64 - Test bit 4 of H
    { GBC_CPU_EX_BIT_4_L,   GBC_CPU_TICKS_8  }, // 0x65 - Test bit 4 of L
    { GBC_CPU_EX_BIT_4_HLP, GBC_CPU_TICKS_12 }, // 0x66 - Test bit 4 of value pointed by HL
    { GBC_CPU_EX_BIT_4_A,   GBC_CPU_TICKS_8  }, // 0x67 - Test bit 4 of A
    { GBC_CPU_EX_BIT_5_B,   GBC_CPU_TICKS_8  }, // 0x68 - Test bit 5 of B
    { GBC_CPU_EX_BIT_5_C,   GBC_CPU_TICKS_8  }, // 0x69 - Test bit 5 of C
    { GBC_CPU_EX_BIT_5_D,   GBC_CPU_TICKS_8  }, // 0x6A - Test bit 5 of D
    { GBC_CPU_EX_BIT_5_E,   GBC_CPU_TICKS_8  }, // 0x6B - Test bit 5 of E
    { GBC_CPU_EX_BIT_5_H,   GBC_CPU_TICKS_8  }, // 0x6C - Test bit 5 of H
    { GBC_CPU_EX_BIT_5_L,   GBC_CPU_TICKS_8  }, // 0x6D - Test bit 5 of L
    { GBC_CPU_EX_BIT_5_HLP, GBC_CPU_TICKS_12 }, // 0x6E - Test bit 5 of value pointed by HL
    { GBC_CPU_EX_BIT_5_A,   GBC_CPU_TICKS_8  }, // 0x6F - Test bit 5 of A
    { GBC_CPU_EX_BIT_6_B,   GBC_CPU_TICKS_8  }, // 0x70 - Test bit 6 of B
    { GBC_CPU_EX_BIT_6_C,   GBC_CPU_TICKS_8  }, // 0x71 - Test bit 6 of C
    { GBC_CPU_EX_BIT_6_D,   GBC_CPU_TICKS_8  }, // 0x72 - Test bit 6 of D
    { GBC_CPU_EX_BIT_6_E,   GBC_CPU_TICKS_8  }, // 0x73 - Test bit 6 of E
    { GBC_CPU_EX_BIT_6_H,   GBC_CPU_TICKS_8  }, // 0x74 - Test bit 6 of H
    { GBC_CPU_EX_BIT_6_L,   GBC_CPU_TICKS_8  }, // 0x75 - Test bit 6 of L
    { GBC_CPU_EX_BIT_6_HLP, GBC_CPU_TICKS_12 }, // 0x76 - Test bit 6 of value pointed by HL
    { GBC_CPU_EX_BIT_6_A,   GBC_CPU_TICKS_8  }, // 0x77 - Test bit 6 of A
    { GBC_CPU_EX_BIT_7_B,   GBC_CPU_TICKS_8  }, // 0x78 - Test bit 7 of B
    { GBC_CPU_EX_BIT_7_C,   GBC_CPU_TICKS_8  }, // 0x79 - Test bit 7 of C
    { GBC_CPU_EX_BIT_7_D,   GBC_CPU_TICKS_8  }, // 0x7A - Test bit 7 of D
    { GBC_CPU_EX_BIT_7_E,   GBC_CPU_TICKS_8  }, // 0x7B - Test bit 7 of E
    { GBC_CPU_EX_BIT_7_H,   GBC_CPU_TICKS_8  }, // 0x7C - Test bit 7 of H
    { GBC_CPU_EX_BIT_7_L,   GBC_CPU_TICKS_8  }, // 0x7D - Test bit 7 of L
    { GBC_CPU_EX_BIT_7_HLP, GBC_CPU_TICKS_12 }, // 0x7E - Test bit 7 of value pointed by HL
    { GBC_CPU_EX_BIT_7_A,   GBC_CPU_TICKS_8  }, // 0x7F - Test bit 7 of A
    { GBC_CPU_EX_RES_0_B,   GBC_CPU_TICKS_8  }, // 0x80 - Clear (reset) bit 0 of B
    { GBC_CPU_EX_RES_0_C,   GBC_CPU_TICKS_8  }, // 0x81 - Clear (reset) bit 0 of C
    { GBC_CPU_EX_RES_0_D,   GBC_CPU_TICKS_8  }, // 0x82 - Clear (reset) bit 0 of D
    { GBC_CPU_EX_RES_0_E,   GBC_CPU_TICKS_8  }, // 0x83 - Clear (reset) bit 0 of E
    { GBC_CPU_EX_RES_0_H,   GBC_CPU_TICKS_8  }, // 0x84 - Clear (reset) bit 0 of H
    { GBC_CPU_EX_RES_0_L,   GBC_CPU_TICKS_8  }, // 0x85 - Clear (reset) bit 0 of L
    { GBC_CPU_EX_RES_0_HLP, GBC_CPU_TICKS_16 }, // 0x86 - Clear (reset) bit 0 of value pointed by HL
    { GBC_CPU_EX_RES_0_A,   GBC_CPU_TICKS_8  }, // 0x87 - Clear (reset) bit 0 of A
    { GBC_CPU_EX_RES_1_B,   GBC_CPU_TICKS_8  }, // 0x88 - Clear (reset) bit 1 of B
    { GBC_CPU_EX_RES_1_C,   GBC_CPU_TICKS_8  }, // 0x89 - Clear (reset) bit 1 of C
    { GBC_CPU_EX_RES_1_D,   GBC_CPU_TICKS_8  }, // 0x8A - Clear (reset) bit 1 of D
    { GBC_CPU_EX_RES_1_E,   GBC_CPU_TICKS_8  }, // 0x8B - Clear (reset) bit 1 of E
    { GBC_CPU_EX_RES_1_H,   GBC_CPU_TICKS_8  }, // 0x8C - Clear (reset) bit 1 of H
    { GBC_CPU_EX_RES_1_L,   GBC_CPU_TICKS_8  }, // 0x8D - Clear (reset) bit 1 of L
    { GBC_CPU_EX_RES_1_HLP, GBC_CPU_TICKS_16 }, // 0x8E - Clear (reset) bit 1 of value pointed by HL
    { GBC_CPU_EX_RES_1_A,   GBC_CPU_TICKS_8  }, // 0x8F - Clear (reset) bit 1 of A
    { GBC_CPU_EX_RES_2_B,   GBC_CPU_TICKS_8  }, // 0x90 - Clear (reset) bit 2 of B
    { GBC_CPU_EX_RES_2_C,   GBC_CPU_TICKS_8  }, // 0x91 - Clear (reset) bit 2 of C
    { GBC_CPU_EX_RES_2_D,   GBC_CPU_TICKS_8  }, // 0x92 - Clear (reset) bit 2 of D
    { GBC_CPU_EX_RES_2_E,   GBC_CPU_TICKS_8  }, // 0x93 - Clear (reset) bit 2 of E
    { GBC_CPU_EX_RES_2_H,   GBC_CPU_TICKS_8  }, // 0x94 - Clear (reset) bit 2 of H
    { GBC_CPU_EX_RES_2_L,   GBC_CPU_TICKS_8  }, // 0x95 - Clear (reset) bit 2 of L
    { GBC_CPU_EX_RES_2_HLP, GBC_CPU_TICKS_16 }, // 0x96 - Clear (reset) bit 2 of value pointed by HL
    { GBC_CPU_EX_RES_2_A,   GBC_CPU_TICKS_8  }, // 0x97 - Clear (reset) bit 2 of A
    { GBC_CPU_EX_RES_3_B,   GBC_CPU_TICKS_8  }, // 0x98 - Clear (reset) bit 3 of B
    { GBC_CPU_EX_RES_3_C,   GBC_CPU_TICKS_8  }, // 0x99 - Clear (reset) bit 3 of C
    { GBC_CPU_EX_RES_3_D,   GBC_CPU_TICKS_8  }, // 0x9A - Clear (reset) bit 3 of D
    { GBC_CPU_EX_RES_3_E,   GBC_CPU_TICKS_8  }, // 0x9B - Clear (reset) bit 3 of E
    { GBC_CPU_EX_RES_3_H,   GBC_CPU_TICKS_8  }, // 0x9C - Clear (reset) bit 3 of H
    { GBC_CPU_EX_RES_3_L,   GBC_CPU_TICKS_8  }, // 0x9D - Clear (reset) bit 3 of L
    { GBC_CPU_EX_RES_3_HLP, GBC_CPU_TICKS_16 }, // 0x9E - Clear (reset) bit 3 of value pointed by HL
    { GBC_CPU_EX_RES_3_A,   GBC_CPU_TICKS_8  }, // 0x9F - Clear (reset) bit 3 of A
    { GBC_CPU_EX_RES_4_B,   GBC_CPU_TICKS_8  }, // 0xA0 - Clear (reset) bit 4 of B
    { GBC_CPU_EX_RES_4_C,   GBC_CPU_TICKS_8  }, // 0xA1 - Clear (reset) bit 4 of C
    { GBC_CPU_EX_RES_4_D,   GBC_CPU_TICKS_8  }, // 0xA2 - Clear (reset) bit 4 of D
    { GBC_CPU_EX_RES_4_E,   GBC_CPU_TICKS_8  }, // 0xA3 - Clear (reset) bit 4 of E
    { GBC_CPU_EX_RES_4_H,   GBC_CPU_TICKS_8  }, // 0xA4 - Clear (reset) bit 4 of H
    { GBC_CPU_EX_RES_4_L,   GBC_CPU_TICKS_8  }, // 0xA5 - Clear (reset) bit 4 of L
    { GBC_CPU_EX_RES_4_HLP, GBC_CPU_TICKS_16 }, // 0xA6 - Clear (reset) bit 4 of value pointed by HL
    { GBC_CPU_EX_RES_4_A,   GBC_CPU_TICKS_8  }, // 0xA7 - Clear (reset) bit 4 of A
    { GBC_CPU_EX_RES_5_B,   GBC_CPU_TICKS_8  }, // 0xA8 - Clear (reset) bit 5 of B
    { GBC_CPU_EX_RES_5_C,   GBC_CPU_TICKS_8  }, // 0xA9 - Clear (reset) bit 5 of C
    { GBC_CPU_EX_RES_5_D,   GBC_CPU_TICKS_8  }, // 0xAA - Clear (reset) bit 5 of D
    { GBC_CPU_EX_RES_5_E,   GBC_CPU_TICKS_8  }, // 0xAB - Clear (reset) bit 5 of E
    { GBC_CPU_EX_RES_5_H,   GBC_CPU_TICKS_8  }, // 0xAC - Clear (reset) bit 5 of H
    { GBC_CPU_EX_RES_5_L,   GBC_CPU_TICKS_8  }, // 0xAD - Clear (reset) bit 5 of L
    { GBC_CPU_EX_RES_5_HLP, GBC_CPU_TICKS_16 }, // 0xAE - Clear (reset) bit 5 of value pointed by HL
    { GBC_CPU_EX_RES_5_A,   GBC_CPU_TICKS_8  }, // 0xAF - Clear (reset) bit 5 of A
    { GBC_CPU_EX_RES_6_B,   GBC_CPU_TICKS_8  }, // 0xB0 - Clear (reset) bit 6 of B
    { GBC_CPU_EX_RES_6_C,   GBC_CPU_TICKS_8  }, // 0xB1 - Clear (reset) bit 6 of C
    { GBC_CPU_EX_RES_6_D,   GBC_CPU_TICKS_8  }, // 0xB2 - Clear (reset) bit 6 of D
    { GBC_CPU_EX_RES_6_E,   GBC_CPU_TICKS_8  }, // 0xB3 - Clear (reset) bit 6 of E
    { GBC_CPU_EX_RES_6_H,   GBC_CPU_TICKS_8  }, // 0xB4 - Clear (reset) bit 6 of H
    { GBC_CPU_EX_RES_6_L,   GBC_CPU_TICKS_8  }, // 0xB5 - Clear (reset) bit 6 of L
    { GBC_CPU_EX_RES_6_HLP, GBC_CPU_TICKS_16 }, // 0xB6 - Clear (reset) bit 6 of value pointed by HL
    { GBC_CPU_EX_RES_6_A,   GBC_CPU_TICKS_8  }, // 0xB7 - Clear (reset) bit 6 of A
    { GBC_CPU_EX_RES_7_B,   GBC_CPU_TICKS_8  }, // 0xB8 - Clear (reset) bit 7 of B
    { GBC_CPU_EX_RES_7_C,   GBC_CPU_TICKS_8  }, // 0xB9 - Clear (reset) bit 7 of C
    { GBC_CPU_EX_RES_7_D,   GBC_CPU_TICKS_8  }, // 0xBA - Clear (reset) bit 7 of D
    { GBC_CPU_EX_RES_7_E,   GBC_CPU_TICKS_8  }, // 0xBB - Clear (reset) bit 7 of E
    { GBC_CPU_EX_RES_7_H,   GBC_CPU_TICKS_8  }, // 0xBC - Clear (reset) bit 7 of H
    { GBC_CPU_EX_RES_7_L,   GBC_CPU_TICKS_8  }, // 0xBD - Clear (reset) bit 7 of L
    { GBC_CPU_EX_RES_7_HLP, GBC_CPU_TICKS_16 }, // 0xBE - Clear (reset) bit 7 of value pointed by HL
    { GBC_CPU_EX_RES_7_A,   GBC_CPU_TICKS_8  }, // 0xBF - Clear (reset) bit 7 of A
    { GBC_CPU_EX_SET_0_B,   GBC_CPU_TICKS_8  }, // 0xC0 - Set bit 0 of B
    { GBC_CPU_EX_SET_0_C,   GBC_CPU_TICKS_8  }, // 0xC1 - Set bit 0 of C
    { GBC_CPU_EX_SET_0_D,   GBC_CPU_TICKS_8  }, // 0xC2 - Set bit 0 of D
    { GBC_CPU_EX_SET_0_E,   GBC_CPU_TICKS_8  }, // 0xC3 - Set bit 0 of E
    { GBC_CPU_EX_SET_0_H,   GBC_CPU_TICKS_8  }, // 0xC4 - Set bit 0 of H
    { GBC_CPU_EX_SET_0_L,   GBC_CPU_TICKS_8  }, // 0xC5 - Set bit 0 of L
    { GBC_CPU_EX_SET_0_HLP, GBC_CPU_TICKS_16 }, // 0xC6 - Set bit 0 of value pointed by HL
    { GBC_CPU_EX_SET_0_A,   GBC_CPU_TICKS_8  }, // 0xC7 - Set bit 0 of A
    { GBC_CPU_EX_SET_1_B,   GBC_CPU_TICKS_8  }, // 0xC8 - Set bit 1 of B
    { GBC_CPU_EX_SET_1_C,   GBC_CPU_TICKS_8  }, // 0xC9 - Set bit 1 of C
    { GBC_CPU_EX_SET_1_D,   GBC_CPU_TICKS_8  }, // 0xCA - Set bit 1 of D
    { GBC_CPU_EX_SET_1_E,   GBC_CPU_TICKS_8  }, // 0xCB - Set bit 1 of E
    { GBC_CPU_EX_SET_1_H,   GBC_CPU_TICKS_8  }, // 0xCC - Set bit 1 of H
    { GBC_CPU_EX_SET_1_L,   GBC_CPU_TICKS_8  }, // 0xCD - Set bit 1 of L
    { GBC_CPU_EX_SET_1_HLP, GBC_CPU_TICKS_16 }, // 0xCE - Set bit 1 of value pointed by HL
    { GBC_CPU_EX_SET_1_A,   GBC_CPU_TICKS_8  }, // 0xCF - Set bit 1 of A
    { GBC_CPU_EX_SET_2_B,   GBC_CPU_TICKS_8  }, // 0xD0 - Set bit 2 of B
    { GBC_CPU_EX_SET_2_C,   GBC_CPU_TICKS_8  }, // 0xD1 - Set bit 2 of C
    { GBC_CPU_EX_SET_2_D,   GBC_CPU_TICKS_8  }, // 0xD2 - Set bit 2 of D
    { GBC_CPU_EX_SET_2_E,   GBC_CPU_TICKS_8  }, // 0xD3 - Set bit 2 of E
    { GBC_CPU_EX_SET_2_H,   GBC_CPU_TICKS_8  }, // 0xD4 - Set bit 2 of H
    { GBC_CPU_EX_SET_2_L,   GBC_CPU_TICKS_8  }, // 0xD5 - Set bit 2 of L
    { GBC_CPU_EX_SET_2_HLP, GBC_CPU_TICKS_16 }, // 0xD6 - Set bit 2 of value pointed by HL
    { GBC_CPU_EX_SET_2_A,   GBC_CPU_TICKS_8  }, // 0xD7 - Set bit 2 of A
    { GBC_CPU_EX_SET_3_B,   GBC_CPU_TICKS_8  }, // 0xD8 - Set bit 3 of B
    { GBC_CPU_EX_SET_3_C,   GBC_CPU_TICKS_8  }, // 0xD9 - Set bit 3 of C
    { GBC_CPU_EX_SET_3_D,   GBC_CPU_TICKS_8  }, // 0xDA - Set bit 3 of D
    { GBC_CPU_EX_SET_3_E,   GBC_CPU_TICKS_8  }, // 0xDB - Set bit 3 of E
    { GBC_CPU_EX_SET_3_H,   GBC_CPU_TICKS_8  }, // 0xDC - Set bit 3 of H
    { GBC_CPU_EX_SET_3_L,   GBC_CPU_TICKS_8  }, // 0xDD - Set bit 3 of L
    { GBC_CPU_EX_SET_3_HLP, GBC_CPU_TICKS_16 }, // 0xDE - Set bit 3 of value pointed by HL
    { GBC_CPU_EX_SET_3_A,   GBC_CPU_TICKS_8  }, // 0xDF - Set bit 3 of A
    { GBC_CPU_EX_SET_4_B,   GBC_CPU_TICKS_8  }, // 0xE0 - Set bit 4 of B
    { GBC_CPU_EX_SET_4_C,   GBC_CPU_TICKS_8  }, // 0xE1 - Set bit 4 of C
    { GBC_CPU_EX_SET_4_D,   GBC_CPU_TICKS_8  }, // 0xE2 - Set bit 4 of D
    { GBC_CPU_EX_SET_4_E,   GBC_CPU_TICKS_8  }, // 0xE3 - Set bit 4 of E
    { GBC_CPU_EX_SET_4_H,   GBC_CPU_TICKS_8  }, // 0xE4 - Set bit 4 of H
    { GBC_CPU_EX_SET_4_L,   GBC_CPU_TICKS_8  }, // 0xE5 - Set bit 4 of L
    { GBC_CPU_EX_SET_4_HLP, GBC_CPU_TICKS_16 }, // 0xE6 - Set bit 4 of value pointed by HL
    { GBC_CPU_EX_SET_4_A,   GBC_CPU_TICKS_8  }, // 0xE7 - Set bit 4 of A
    { GBC_CPU_EX_SET_5_B,   GBC_CPU_TICKS_8  }, // 0xE8 - Set bit 5 of B
    { GBC_CPU_EX_SET_5_C,   GBC_CPU_TICKS_8  }, // 0xE9 - Set bit 5 of C
    { GBC_CPU_EX_SET_5_D,   GBC_CPU_TICKS_8  }, // 0xEA - Set bit 5 of D
    { GBC_CPU_EX_SET_5_E,   GBC_CPU_TICKS_8  }, // 0xEB - Set bit 5 of E
    { GBC_CPU_EX_SET_5_H,   GBC_CPU_TICKS_8  }, // 0xEC - Set bit 5 of H
    { GBC_CPU_EX_SET_5_L,   GBC_CPU_TICKS_8  }, // 0xED - Set bit 5 of L
    { GBC_CPU_EX_SET_5_HLP, GBC_CPU_TICKS_16 }, // 0xEE - Set bit 5 of value pointed by HL
    { GBC_CPU_EX_SET_5_A,   GBC_CPU_TICKS_8  }, // 0xEF - Set bit 5 of A
    { GBC_CPU_EX_SET_6_B,   GBC_CPU_TICKS_8  }, // 0xF0 - Set bit 6 of B
    { GBC_CPU_EX_SET_6_C,   GBC_CPU_TICKS_8  }, // 0xF1 - Set bit 6 of C
    { GBC_CPU_EX_SET_6_D,   GBC_CPU_TICKS_8  }, // 0xF2 - Set bit 6 of D
    { GBC_CPU_EX_SET_6_E,   GBC_CPU_TICKS_8  }, // 0xF3 - Set bit 6 of E
    { GBC_CPU_EX_SET_6_H,   GBC_CPU_TICKS_8  }, // 0xF4 - Set bit 6 of H
    { GBC_CPU_EX_SET_6_L,   GBC_CPU_TICKS_8  }, // 0xF5 - Set bit 6 of L
    { GBC_CPU_EX_SET_6_HLP, GBC_CPU_TICKS_16 }, // 0xF6 - Set bit 6 of value pointed by HL
    { GBC_CPU_EX_SET_6_A,   GBC_CPU_TICKS_8  }, // 0xF7 - Set bit 6 of A
    { GBC_CPU_EX_SET_7_B,   GBC_CPU_TICKS_8  }, // 0xF8 - Set bit 7 of B
    { GBC_CPU_EX_SET_7_C,   GBC_CPU_TICKS_8  }, // 0xF9 - Set bit 7 of C
    { GBC_CPU_EX_SET_7_D,   GBC_CPU_TICKS_8  }, // 0xFA - Set bit 7 of D
    { GBC_CPU_EX_SET_7_E,   GBC_CPU_TICKS_8  }, // 0xFB - Set bit 7 of E
    { GBC_CPU_EX_SET_7_H,   GBC_CPU_TICKS_8  }, // 0xFC - Set bit 7 of H
    { GBC_CPU_EX_SET_7_L,   GBC_CPU_TICKS_8  }, // 0xFD - Set bit 7 of L
    { GBC_CPU_EX_SET_7_HLP, GBC_CPU_TICKS_16 }, // 0xFE - Set bit 7 of value pointed by HL
    { GBC_CPU_EX_SET_7_A,   GBC_CPU_TICKS_8  }, // 0xFF - Set bit 7 of A
};
