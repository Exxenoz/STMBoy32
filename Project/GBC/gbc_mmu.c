#include "gbc_mmu.h"

// Global GBC Memory
GBC_MMU_Memory_t GBC_MMU_Memory;

uint8_t GBC_MMU_ReadByte(uint16_t address)
{
    // Shadow RAM redirection to WRAM
    if (address >= 0xE000 && address <= 0xFDFF)
    {
        return GBC_MMU_Memory.WRAMBank0[address - 0xE000];
    }

    return GBC_MMU_Memory.Data[address];
}

uint16_t GBC_MMU_ReadShort(uint16_t address)
{
    return GBC_MMU_ReadByte(address) | (GBC_MMU_ReadByte(address + 1) << 8);
}

void GBC_MMU_WriteByte(uint16_t address, uint8_t value)
{
    // Shadow RAM redirection to WRAM
    if (address >= 0xE000 && address <= 0xFDFF)
    {
        GBC_MMU_Memory.WRAMBank0[address - 0xE000] = value;
    }

    GBC_MMU_Memory.Data[address] = value;
}

void GBC_MMU_WriteShort(uint16_t address, uint16_t value)
{
    GBC_MMU_WriteByte(address, value & 0xFF);
    GBC_MMU_WriteByte(address + 1, (value & 0xFF00) >> 8);
}
