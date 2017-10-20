#include "gbc_cpu.h"
#include "gbc_mmu.h"

// Global GBC CPU register
GBC_CPU_Register_t GBC_CPU_Register;
// Global GBC CPU ticks
uint32_t GBC_CPU_Ticks = 0;
// Global GBC CPU stopped state
bool GBC_CPU_Stopped = false;

uint8_t GBC_CPU_IncrementByte(uint8_t value)
{
    value++;

    // Carry flag not affected

    if ((value & 0xF) == 0)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

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

uint8_t GBC_CPU_DecrementByte(uint8_t value)
{
    value--;

    // Carry flag not affected

    if ((value & 0xF) == 0xF)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);

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

uint16_t GBC_CPU_AddShorts(uint16_t a, uint16_t b)
{
    uint32_t result = a + b;
    
    if (result & 0x10000)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    result &= 0xFFFF;

    if (((a & 0xFFF) + (b & 0xFFF)) & 0x1000)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }
    
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

    // Zero flag not affected

    return result;
}

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
    GBC_CPU_Register.B = GBC_CPU_IncrementByte(GBC_CPU_Register.B);
}

void GBC_CPU_DEC_B()                    // 0x05 - Decrement B
{
    GBC_CPU_Register.B = GBC_CPU_DecrementByte(GBC_CPU_Register.B);
}

void GBC_CPU_LD_B_X(uint8_t operand)    // 0x06 - Load 8-bit immediate into B
{
    GBC_CPU_Register.B = operand;
}

void GBC_CPU_RLC_A()                    // 0x07 - Rotate A left with carry
{
    uint8_t value = GBC_CPU_Register.A;

    if (value & 0x8)
    {
        value <<= 0x1;
        value  |= 0x1;

        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        value <<= 0x1;

        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

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

void GBC_CPU_LD_XXP_SP(uint16_t operand)// 0x08 - Save SP to given address
{
    GBC_MMU_WriteShort(operand, GBC_CPU_Register.SP);
}

void GBC_CPU_ADD_HL_BC()                // 0x09 - Add 16-bit BC to HL
{
    GBC_CPU_Register.HL = GBC_CPU_AddShorts(GBC_CPU_Register.HL, GBC_CPU_Register.BC);
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
    GBC_CPU_Register.C = GBC_CPU_IncrementByte(GBC_CPU_Register.C);
}

void GBC_CPU_DEC_C()                    // 0x0D - Decrement C
{
    GBC_CPU_Register.C = GBC_CPU_DecrementByte(GBC_CPU_Register.C);
}

void GBC_CPU_LD_C_X(uint8_t operand)    // 0x0E - Load 8-bit immediate into C
{
    GBC_CPU_Register.C = operand;
}

void GBC_CPU_RRC_A()                    // 0x0F - Rotate A right with carry
{
    uint8_t value = GBC_CPU_Register.A;

    if (value & 0x1)
    {
        value >>= 0x1;
        value  |= 0x8;

        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        value >>= 0x1;

        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

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

void GBC_CPU_STOP()                     // 0x10 - Stop processor
{
    GBC_CPU_Stopped = true;
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
    GBC_CPU_Register.D = GBC_CPU_IncrementByte(GBC_CPU_Register.D);
}

void GBC_CPU_DEC_D()                    // 0x15 - Decrement D
{
    GBC_CPU_Register.D = GBC_CPU_DecrementByte(GBC_CPU_Register.D);
}

void GBC_CPU_LD_D_X(uint8_t operand)    // 0x16 - Load 8-bit immediate into D
{
    GBC_CPU_Register.D = operand;
}

void GBC_CPU_RL_A()                     // 0x17 - Rotate A left
{
    uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0;
    uint8_t value = GBC_CPU_Register.A;

    if (value & 0x8)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    value <<= 0x1;
    value  |= carry;

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

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

void GBC_CPU_JR_X(int8_t operand)       // 0x18 - Relative jump by signed immediate
{
    GBC_CPU_Register.PC += operand;
}

void GBC_CPU_ADD_HL_DE()                // 0x19 - Add 16-bit DE to HL
{
    GBC_CPU_Register.HL = GBC_CPU_AddShorts(GBC_CPU_Register.HL, GBC_CPU_Register.DE);
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
    GBC_CPU_Register.E = GBC_CPU_IncrementByte(GBC_CPU_Register.E);
}

void GBC_CPU_DEC_E()                    // 0x1D - Decrement E
{
    GBC_CPU_Register.E = GBC_CPU_DecrementByte(GBC_CPU_Register.E);
}

void GBC_CPU_LD_E_X(uint8_t operand)    // 0x1E - Load 8-bit immediate into E
{
    GBC_CPU_Register.E = operand;
}

void GBC_CPU_RR_A()                     // 0x1F - Rotate A right
{
    uint8_t carry = GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0;
    uint8_t value = GBC_CPU_Register.A;

    if (value & 0x1)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    value >>= 0x1;
    value  |= carry;

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

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

void GBC_CPU_JR_NZ_X(int8_t operand)    // 0x20 - Relative jump by signed immediate if last result was not zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_Ticks += 8;
    }
    else
    {
        GBC_CPU_Register.PC += operand;
        GBC_CPU_Ticks += 12;
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
    GBC_CPU_Register.H = GBC_CPU_IncrementByte(GBC_CPU_Register.H);
}

void GBC_CPU_DEC_H()                    // 0x25 - Decrement H
{
    GBC_CPU_Register.H = GBC_CPU_DecrementByte(GBC_CPU_Register.H);
}

void GBC_CPU_LD_H_X(uint8_t operand)    // 0x26 - Load 8-bit immediate into H
{
    GBC_CPU_Register.H = operand;
}

void GBC_CPU_DA_A()                     // 0x27 - Adjust A for BCD (Binary Coded Decimal) addition
{
    uint16_t value = GBC_CPU_Register.A;

    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_SUBTRACTION))
    {
        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY))
        {
            value = (value - 6) & 0xFF;
        }

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
        {
            value -= 0x6;
        }
    }
    else
    {
        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_HALFCARRY) || (value & 0xF) > 9)
        {
            value += 0x6;
        }

        if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) || value > 0x9F)
        {
            value += 0x6;
        }
    }
    
    if ((value & 0x100) == 0x100)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
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

void GBC_CPU_JR_Z_X(int8_t operand)     // 0x28 - Relative jump by signed immediate if last result was zero
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_ZERO))
    {
        GBC_CPU_Register.PC += operand;
        GBC_CPU_Ticks += 12;
    }
    else
    {
        GBC_CPU_Ticks += 8;
    }
}

void GBC_CPU_ADD_HL_HL()                // 0x29 - Add 16-bit HL to HL
{
    GBC_CPU_Register.HL = GBC_CPU_AddShorts(GBC_CPU_Register.HL, GBC_CPU_Register.HL);
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
    GBC_CPU_Register.L = GBC_CPU_IncrementByte(GBC_CPU_Register.L);
}

void GBC_CPU_DEC_L()                    // 0x2D - Decrement L
{
    GBC_CPU_Register.L = GBC_CPU_DecrementByte(GBC_CPU_Register.L);
}

void GBC_CPU_LD_L_X(uint8_t operand)    // 0x2E - Load 8-bit immediate into L
{
    GBC_CPU_Register.L = operand;
}

void GBC_CPU_CPL()                      // 0x2F - Complement (logical NOT) on A (flip all bits)
{
    GBC_CPU_Register.A = ~GBC_CPU_Register.A;

    // Carry flag not affected
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);
    // Zero flag not affected
}

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_Instruction_t GBC_CPU_Instructions[48] =
{
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x00 - No operation
    { GBC_CPU_LD_BC_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6  }, // 0x01 - Load 16-bit immediate into BC
    { GBC_CPU_LD_BCP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x02 - Save A to address pointed by BC
    { GBC_CPU_INC_BC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x03 - Increment 16-bit BC
    { GBC_CPU_INC_B,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x04 - Increment B
    { GBC_CPU_DEC_B,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x05 - Decrement B
    { GBC_CPU_LD_B_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x06 - Load 8-bit immediate into B
    { GBC_CPU_RLC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x07 - Rotate A left with carry
    { GBC_CPU_LD_XXP_SP, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_10 }, // 0x08 - Save SP to given address
    { GBC_CPU_ADD_HL_BC, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x09 - Add 16-bit BC to HL
    { GBC_CPU_LD_A_BCP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0A - Load A from address pointed to by BC
    { GBC_CPU_DEC_BC,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0B - Decrement 16-bit BC
    { GBC_CPU_INC_C,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x0C - Increment C
    { GBC_CPU_DEC_C,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x0D - Decrement C
    { GBC_CPU_LD_C_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x0E - Load 8-bit immediate into C
    { GBC_CPU_RRC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x0F - Rotate A right with carry
    { GBC_CPU_STOP,      GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_2  }, // 0x10 - Stop processor
    { GBC_CPU_LD_DE_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6  }, // 0x11 - Load 16-bit immediate into DE
    { GBC_CPU_LD_DEP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x12 - Save A to address pointed by DE
    { GBC_CPU_INC_DE,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x13 - Increment 16-bit DE
    { GBC_CPU_INC_D,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x14 - Increment D
    { GBC_CPU_DEC_D,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x15 - Decrement D
    { GBC_CPU_LD_D_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x16 - Load 8-bit immediate into D
    { GBC_CPU_RL_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x17 - Rotate A left
    { GBC_CPU_JR_X,      GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x18 - Relative jump by signed immediate
    { GBC_CPU_ADD_HL_DE, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x19 - Add 16-bit DE to HL
    { GBC_CPU_LD_A_DEP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1A - Load A from address pointed to by DE
    { GBC_CPU_DEC_DE,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1B - Decrement 16-bit DE
    { GBC_CPU_INC_E,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x1C - Increment E
    { GBC_CPU_DEC_E,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x1D - Decrement E
    { GBC_CPU_LD_E_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x1E - Load 8-bit immediate into E
    { GBC_CPU_RR_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x1F - Rotate A right
    { GBC_CPU_JR_NZ_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0x20 - Relative jump by signed immediate if last result was not zero
    { GBC_CPU_LD_HL_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6  }, // 0x21 - Load 16-bit immediate into HL
    { GBC_CPU_LDI_HLP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x22 - Save A to address pointed by HL, and increment HL
    { GBC_CPU_INC_HL,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x23 - Increment 16-bit HL
    { GBC_CPU_INC_H,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x24 - Increment H
    { GBC_CPU_DEC_H,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x25 - Decrement H
    { GBC_CPU_LD_H_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x26 - Load 8-bit immediate into H
    { GBC_CPU_DA_A,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x27 - Adjust A for BCD (Binary Coded Decimal) addition
    { GBC_CPU_JR_Z_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0x28 - Relative jump by signed immediate if last result was zero
    { GBC_CPU_ADD_HL_HL, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x29 - Add 16-bit HL to HL
    { GBC_CPU_LDI_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x2A - Load A from address pointed to by HL, and increment HL
    { GBC_CPU_DEC_HL,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x2B - Decrement 16-bit HL
    { GBC_CPU_INC_L,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x2C - Increment L
    { GBC_CPU_DEC_L,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x2D - Decrement L
    { GBC_CPU_LD_L_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x2E - Load 8-bit immediate into L
    { GBC_CPU_CPL,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x2F - Complement (logical NOT) on A (flip all bits)
};
