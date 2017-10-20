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

uint8_t GBC_CPU_AddBytes(uint8_t a, uint8_t b)
{
    uint32_t result = a + b; // ToDo: Make overflow possible

    if (result & 0x100)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    if (((a & 0xF) + (b & 0xF)) & 0x10)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);

    result &= 0xFF;

    if (result)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return result;
}

uint8_t GBC_CPU_SubBytes(uint8_t a, uint8_t b)
{
    uint8_t result = a - b;

    if (b > a)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    if ((b & 0xF) > (a & 0xF))
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);

    if (result)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return result;
}

uint16_t GBC_CPU_AddShorts(uint16_t a, uint16_t b)
{
    uint32_t result = a + b; // ToDo: Make overflow possible

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

uint8_t GBC_CPU_AND_Operator(uint8_t a, uint8_t b)
{
    uint8_t result = a & b;

    if (result)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_SUBTRACTION | GBC_CPU_FLAGS_ZERO);
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_SUBTRACTION);
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_ZERO);
    }

    return result;
}

uint8_t GBC_CPU_OR_Operator(uint8_t a, uint8_t b)
{
    uint8_t result = a | b;

    if (result)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ALL);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return result;
}

uint8_t GBC_CPU_XOR_Operator(uint8_t a, uint8_t b)
{
    uint8_t result = a ^ b;

    if (result)
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ALL);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY | GBC_CPU_FLAGS_HALFCARRY | GBC_CPU_FLAGS_SUBTRACTION);
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }

    return result;
}

void GBC_CPU_COMPARE_Operator(uint8_t a, uint8_t b)
{
    if (a < b)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }

    if ((a & 0xF) < (b & 0xF))
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_HALFCARRY);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    }

    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_SUBTRACTION);

    if (a == b)
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_ZERO);
    }
    else
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_ZERO);
    }
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

void GBC_CPU_JR_NC_X(int8_t operand)    // 0x30 - Relative jump by signed immediate if last result caused no carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_Ticks += 8;
    }
    else
    {
        GBC_CPU_Register.PC += operand;
        GBC_CPU_Ticks += 12;
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
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_IncrementByte(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_DEC_HLP()                  // 0x35 - Decrement value pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, GBC_CPU_DecrementByte(GBC_MMU_ReadByte(GBC_CPU_Register.HL)));
}

void GBC_CPU_LD_HLP_X(uint8_t operand)  // 0x36 - Load 8-bit immediate into address pointed by HL
{
    GBC_MMU_WriteByte(GBC_CPU_Register.HL, operand);
}

void GBC_CPU_SCF()                      // 0x37 - Set carry flag
{
    GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);
    // Zero flag not affected
}

void GBC_CPU_JR_C_X(int8_t operand)     // 0x38 - Relative jump by signed immediate if last result caused carry
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_Register.PC += operand;
        GBC_CPU_Ticks += 12;
    }
    else
    {
        GBC_CPU_Ticks += 8;
    }
}

void GBC_CPU_ADD_HL_SP()                // 0x39 - Add 16-bit SP to HL
{
    GBC_CPU_Register.HL = GBC_CPU_AddShorts(GBC_CPU_Register.HL, GBC_CPU_Register.SP);
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
    GBC_CPU_Register.A = GBC_CPU_IncrementByte(GBC_CPU_Register.A);
}

void GBC_CPU_DEC_A()                    // 0x3D - Decrement A
{
    GBC_CPU_Register.A = GBC_CPU_DecrementByte(GBC_CPU_Register.A);
}

void GBC_CPU_LD_A_X(uint8_t operand)    // 0x3E - Load 8-bit immediate into A
{
    GBC_CPU_Register.A = operand;
}

void GBC_CPU_CCF()                      // 0x3F - Complement carry flag
{
    if (GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY))
    {
        GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_CARRY);
    }
    else
    {
        GBC_CPU_FLAGS_SET(GBC_CPU_FLAGS_CARRY);
    }

    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_HALFCARRY);
    GBC_CPU_FLAGS_CLEAR(GBC_CPU_FLAGS_SUBTRACTION);
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
    // ToDo
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
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_ADD_A_C()                  // 0x81 - Add C to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_ADD_A_D()                  // 0x82 - Add D to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_ADD_A_E()                  // 0x83 - Add E to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_ADD_A_H()                  // 0x84 - Add H to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_ADD_A_L()                  // 0x85 - Add L to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_ADD_A_HLP()                // 0x86 - Add value pointed by HL to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_ADD_A_A()                  // 0x87 - Add A to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_ADC_A_B()                  // 0x88 - Add B and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.B + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_C()                  // 0x89 - Add C and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.C + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_D()                  // 0x8A - Add D and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.D + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_E()                  // 0x8B - Add E and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.E + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_H()                  // 0x8C - Add H and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.H + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_L()                  // 0x8D - Add L and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.L + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_HLP()                // 0x8E - Add value pointed by HL and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL) + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_ADC_A_A()                  // 0x8F - Add A and carry flag to A
{
    GBC_CPU_Register.A = GBC_CPU_AddBytes(GBC_CPU_Register.A, GBC_CPU_Register.A + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SUB_A_B()                  // 0x90 - Subtract B from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_SUB_A_C()                  // 0x91 - Subtract C from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_SUB_A_D()                  // 0x92 - Subtract D from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_SUB_A_E()                  // 0x93 - Subtract E from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_SUB_A_H()                  // 0x94 - Subtract H from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_SUB_A_L()                  // 0x95 - Subtract L from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_SUB_A_HLP()                // 0x96 - Subtract value pointed by HL from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_SUB_A_A()                  // 0x97 - Subtract A from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_SDC_A_B()                  // 0x98 - Subtract B and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.B + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_C()                  // 0x99 - Subtract C and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.C + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_D()                  // 0x9A - Subtract D and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.D + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_E()                  // 0x9B - Subtract E and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.E + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_H()                  // 0x9C - Subtract H and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.H + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_L()                  // 0x9D - Subtract L and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.L + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_HLP()                // 0x9E - Subtract value pointed by HL and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL) + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_SDC_A_A()                  // 0x9F - Subtract A and carry flag from A
{
    GBC_CPU_Register.A = GBC_CPU_SubBytes(GBC_CPU_Register.A, GBC_CPU_Register.A + GBC_CPU_FLAGS_HAS(GBC_CPU_FLAGS_CARRY) ? 1 : 0);
}

void GBC_CPU_AND_A_B()                  // 0xA0 - Logical AND B against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_AND_A_C()                  // 0xA1 - Logical AND C against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_AND_A_D()                  // 0xA2 - Logical AND D against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_AND_A_E()                  // 0xA3 - Logical AND E against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_AND_A_H()                  // 0xA4 - Logical AND H against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_AND_A_L()                  // 0xA5 - Logical AND L against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_AND_A_HLP()                // 0xA6 - Logical AND value pointed by HL against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_AND_A_A()                  // 0xA7 - Logical AND A against A
{
    GBC_CPU_Register.A = GBC_CPU_AND_Operator(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_XOR_A_B()                  // 0xA8 - Logical XOR B against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_XOR_A_C()                  // 0xA9 - Logical XOR C against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_XOR_A_D()                  // 0xAA - Logical XOR D against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_XOR_A_E()                  // 0xAB - Logical XOR E against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_XOR_A_H()                  // 0xAD - Logical XOR H against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_XOR_A_L()                  // 0xAD - Logical XOR L against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_XOR_A_HLP()                // 0xAE - Logical XOR value pointed by HL against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_XOR_A_A()                  // 0xAF - Logical XOR A against A
{
    GBC_CPU_Register.A = GBC_CPU_XOR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_OR_A_B()                  // 0xB0 - Logical OR B against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_OR_A_C()                  // 0xB1 - Logical OR C against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_OR_A_D()                  // 0xB2 - Logical OR D against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_OR_A_E()                  // 0xB3 - Logical OR E against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_OR_A_H()                  // 0xB4 - Logical OR H against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_OR_A_L()                  // 0xB5 - Logical OR L against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_OR_A_HLP()                // 0xB6 - Logical OR value pointed by HL against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_OR_A_A()                  // 0xB7 - Logical OR A against A
{
    GBC_CPU_Register.A = GBC_CPU_OR_Operator(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

void GBC_CPU_CP_A_B()                  // 0xB8 - Compare B against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.B);
}

void GBC_CPU_CP_A_C()                  // 0xB9 - Compare C against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.C);
}

void GBC_CPU_CP_A_D()                  // 0xBA - Compare D against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.D);
}

void GBC_CPU_CP_A_E()                  // 0xBB - Compare E against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.E);
}

void GBC_CPU_CP_A_H()                  // 0xBC - Compare H against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.H);
}

void GBC_CPU_CP_A_L()                  // 0xBD - Compare L against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.L);
}

void GBC_CPU_CP_A_HLP()                // 0xBE - Compare value pointed by HL against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_MMU_ReadByte(GBC_CPU_Register.HL));
}

void GBC_CPU_CP_A_A()                  // 0xBF - Compare A against A
{
    GBC_CPU_COMPARE_Operator(GBC_CPU_Register.A, GBC_CPU_Register.A);
}

/*******************************************************************************/
/* Opcode table and comments from http://imrannazar.com/Gameboy-Z80-Opcode-Map */
/*******************************************************************************/
const GBC_CPU_Instruction_t GBC_CPU_Instructions[192] =
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
    { GBC_CPU_JR_NC_X,   GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0x30 - Relative jump by signed immediate if last result caused no carry
    { GBC_CPU_LD_SP_XX,  GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6  }, // 0x31 - Load 16-bit immediate into SP
    { GBC_CPU_LDD_HLP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x32 - Save A to address pointed by HL, and decrement HL
    { GBC_CPU_INC_SP,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x33 - Increment 16-bit SP
    { GBC_CPU_INC_HLP,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_6  }, // 0x34 - Increment value pointed by HL
    { GBC_CPU_DEC_HLP,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_6  }, // 0x35 - Decrement value pointed by HL
    { GBC_CPU_LD_HLP_X,  GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_6  }, // 0x36 - Load 8-bit immediate into address pointed by HL
    { GBC_CPU_SCF,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x37 - Set carry flag
    { GBC_CPU_JR_C_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_0  }, // 0x38 - Relative jump by signed immediate if last result caused carry
    { GBC_CPU_ADD_HL_SP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x39 - Add 16-bit SP to HL
    { GBC_CPU_LDD_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x3A - Load A from address pointed to by HL, and decrement HL
    { GBC_CPU_DEC_SP,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x3B - Decrement 16-bit SP
    { GBC_CPU_INC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x3C - Increment A
    { GBC_CPU_DEC_A,     GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x3D - Decrement A
    { GBC_CPU_LD_A_X,    GBC_CPU_OPERAND_BYTES_1, GBC_CPU_TICKS_4  }, // 0x3E - Load 8-bit immediate into A
    { GBC_CPU_CCF,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x3F - Complement carry flag
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x40 - Copy B to B
    { GBC_CPU_LD_B_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x41 - Copy C to B
    { GBC_CPU_LD_B_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x42 - Copy D to B
    { GBC_CPU_LD_B_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x43 - Copy E to B
    { GBC_CPU_LD_B_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x44 - Copy H to B
    { GBC_CPU_LD_B_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x45 - Copy L to B
    { GBC_CPU_LD_B_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x46 - Copy value pointed by HL to B
    { GBC_CPU_LD_B_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x47 - Copy A to B
    { GBC_CPU_LD_C_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x48 - Copy B to C
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x49 - Copy C to C
    { GBC_CPU_LD_C_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x4A - Copy D to C
    { GBC_CPU_LD_C_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x4B - Copy E to C
    { GBC_CPU_LD_C_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x4C - Copy H to C
    { GBC_CPU_LD_C_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x4D - Copy L to C
    { GBC_CPU_LD_C_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x4E - Copy value pointed by HL to C
    { GBC_CPU_LD_C_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x4F - Copy A to C
    { GBC_CPU_LD_D_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x50 - Copy B to D
    { GBC_CPU_LD_D_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x51 - Copy C to D
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x52 - Copy D to D
    { GBC_CPU_LD_D_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x53 - Copy E to D
    { GBC_CPU_LD_D_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x54 - Copy H to D
    { GBC_CPU_LD_D_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x55 - Copy L to D
    { GBC_CPU_LD_D_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x56 - Copy value pointed by HL to D
    { GBC_CPU_LD_D_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x57 - Copy A to D
    { GBC_CPU_LD_E_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x58 - Copy B to E
    { GBC_CPU_LD_E_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x59 - Copy C to E
    { GBC_CPU_LD_E_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x5A - Copy D to E
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x5B - Copy E to E
    { GBC_CPU_LD_E_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x5C - Copy H to E
    { GBC_CPU_LD_E_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x5D - Copy L to E
    { GBC_CPU_LD_E_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x5E - Copy value pointed by HL to E
    { GBC_CPU_LD_E_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x5F - Copy A to E
    { GBC_CPU_LD_H_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x60 - Copy B to H
    { GBC_CPU_LD_H_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x61 - Copy C to H
    { GBC_CPU_LD_H_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x62 - Copy D to H
    { GBC_CPU_LD_H_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x63 - Copy E to H
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x64 - Copy H to H
    { GBC_CPU_LD_H_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x65 - Copy L to H
    { GBC_CPU_LD_H_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x66 - Copy value pointed by HL to H
    { GBC_CPU_LD_H_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x67 - Copy A to H
    { GBC_CPU_LD_L_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x68 - Copy B to L
    { GBC_CPU_LD_L_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x69 - Copy C to L
    { GBC_CPU_LD_L_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x6A - Copy D to L
    { GBC_CPU_LD_L_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x6B - Copy E to L
    { GBC_CPU_LD_L_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x6C - Copy H to L
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x6D - Copy L to L
    { GBC_CPU_LD_L_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x6E - Copy value pointed by HL to L
    { GBC_CPU_LD_L_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x6F - Copy A to L
    { GBC_CPU_LD_HLP_B,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x70 - Copy B to address pointed by HL
    { GBC_CPU_LD_HLP_C,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x71 - Copy C to address pointed by HL
    { GBC_CPU_LD_HLP_D,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x72 - Copy D to address pointed by HL
    { GBC_CPU_LD_HLP_E,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x73 - Copy E to address pointed by HL
    { GBC_CPU_LD_HLP_H,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x74 - Copy H to address pointed by HL
    { GBC_CPU_LD_HLP_L,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x75 - Copy L to address pointed by HL
    { GBC_CPU_HALT,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x76 - Halt processor
    { GBC_CPU_LD_HLP_A,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x77 - Copy A to address pointed by HL
    { GBC_CPU_LD_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x78 - Copy B to A
    { GBC_CPU_LD_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x79 - Copy C to A
    { GBC_CPU_LD_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x7A - Copy D to A
    { GBC_CPU_LD_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x7B - Copy E to A
    { GBC_CPU_LD_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x7C - Copy H to A
    { GBC_CPU_LD_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x7D - Copy L to A
    { GBC_CPU_LD_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x7E - Copy value pointed by HL to A
    { GBC_CPU_NOP,       GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x7F - Copy A to A
    { GBC_CPU_ADD_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x80 - Add B to A
    { GBC_CPU_ADD_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x81 - Add C to A
    { GBC_CPU_ADD_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x82 - Add D to A
    { GBC_CPU_ADD_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x83 - Add E to A
    { GBC_CPU_ADD_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x84 - Add H to A
    { GBC_CPU_ADD_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x85 - Add L to A
    { GBC_CPU_ADD_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x86 - Add value pointed by HL to A
    { GBC_CPU_ADD_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x87 - Add A to A
    { GBC_CPU_ADC_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x88 - Add B and carry flag to A
    { GBC_CPU_ADC_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x89 - Add C and carry flag to A
    { GBC_CPU_ADC_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x8A - Add D and carry flag to A
    { GBC_CPU_ADC_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x8B - Add E and carry flag to A
    { GBC_CPU_ADC_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x8C - Add H and carry flag to A
    { GBC_CPU_ADC_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x8D - Add L and carry flag to A
    { GBC_CPU_ADC_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x8E - Add value pointed by HL and carry flag to A
    { GBC_CPU_ADC_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x8F - Add A and carry flag to A
    { GBC_CPU_SUB_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x90 - Subtract B from A
    { GBC_CPU_SUB_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x91 - Subtract C from A
    { GBC_CPU_SUB_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x92 - Subtract D from A
    { GBC_CPU_SUB_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x93 - Subtract E from A
    { GBC_CPU_SUB_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x94 - Subtract H from A
    { GBC_CPU_SUB_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x95 - Subtract L from A
    { GBC_CPU_SUB_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x96 - Subtract value pointed by HL from A
    { GBC_CPU_SUB_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x97 - Subtract A from A
    { GBC_CPU_SDC_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x98 - Subtract B and carry flag from A
    { GBC_CPU_SDC_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x99 - Subtract C and carry flag from A
    { GBC_CPU_SDC_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x9A - Subtract D and carry flag from A
    { GBC_CPU_SDC_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x9B - Subtract E and carry flag from A
    { GBC_CPU_SDC_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x9C - Subtract H and carry flag from A
    { GBC_CPU_SDC_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x9D - Subtract L and carry flag from A
    { GBC_CPU_SDC_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0x9E - Subtract value pointed by HL and carry flag from A
    { GBC_CPU_SDC_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0x9F - Subtract A and carry flag from A
    { GBC_CPU_AND_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA0 - Logical AND B against A
    { GBC_CPU_AND_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA1 - Logical AND C against A
    { GBC_CPU_AND_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA2 - Logical AND D against A
    { GBC_CPU_AND_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA3 - Logical AND E against A
    { GBC_CPU_AND_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA4 - Logical AND H against A
    { GBC_CPU_AND_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA5 - Logical AND L against A
    { GBC_CPU_AND_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xA6 - Logical AND value pointed by HL against A
    { GBC_CPU_AND_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA7 - Logical AND A against A
    { GBC_CPU_XOR_A_B,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA8 - Logical XOR B against A
    { GBC_CPU_XOR_A_C,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xA9 - Logical XOR C against A
    { GBC_CPU_XOR_A_D,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xAA - Logical XOR D against A
    { GBC_CPU_XOR_A_E,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xAB - Logical XOR E against A
    { GBC_CPU_XOR_A_H,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xAC - Logical XOR H against A
    { GBC_CPU_XOR_A_L,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xAD - Logical XOR L against A
    { GBC_CPU_XOR_A_HLP, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xAE - Logical XOR value pointed by HL against A
    { GBC_CPU_XOR_A_A,   GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xAF - Logical XOR A against A
    { GBC_CPU_OR_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB0 - Logical OR B against A
    { GBC_CPU_OR_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB1 - Logical OR C against A
    { GBC_CPU_OR_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB2 - Logical OR D against A
    { GBC_CPU_OR_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB3 - Logical OR E against A
    { GBC_CPU_OR_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB4 - Logical OR H against A
    { GBC_CPU_OR_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB5 - Logical OR L against A
    { GBC_CPU_OR_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xB6 - Logical OR value pointed by HL against A
    { GBC_CPU_OR_A_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB7 - Logical OR A against A
    { GBC_CPU_CP_A_B,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB8 - Compare B against A
    { GBC_CPU_CP_A_C,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xB9 - Compare C against A
    { GBC_CPU_CP_A_D,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xBA - Compare D against A
    { GBC_CPU_CP_A_E,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xBB - Compare E against A
    { GBC_CPU_CP_A_H,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xBC - Compare H against A
    { GBC_CPU_CP_A_L,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xBD - Compare L against A
    { GBC_CPU_CP_A_HLP,  GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4  }, // 0xBE - Compare value pointed by HL against A
    { GBC_CPU_CP_A_A,    GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2  }, // 0xBF - Compare A against A
};

void GBC_CPU_Step()
{
    if (GBC_CPU_Stopped)
    {
        return;
    }

    GBC_CPU_Instruction_t instruction = GBC_CPU_Instructions[GBC_MMU_ReadByte(GBC_CPU_Register.PC++)];

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

    GBC_CPU_Ticks += instruction.Ticks;
}
