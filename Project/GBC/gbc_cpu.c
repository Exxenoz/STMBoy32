#include "gbc_cpu.h"

// Global GBC CPU register
GBC_CPU_Register_t GBC_CPU_Register;

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
    // ToDo: Replicate GBC memory structure
}

const GBC_CPU_Instruction_t GBC_CPU_Instructions[3] =
{
    { GBC_CPU_NOP,      GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_2 }, // 0x00 - No operation
    { GBC_CPU_LD_BC_XX, GBC_CPU_OPERAND_BYTES_2, GBC_CPU_TICKS_6 }, // 0x01 - Load 2 bytes into BC
    { GBC_CPU_LD_BCP_A, GBC_CPU_OPERAND_BYTES_0, GBC_CPU_TICKS_4 }, // 0x02 - Save A to address pointed by BC
};
