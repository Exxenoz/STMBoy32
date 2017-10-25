#include "gbc_mmu.h"
#include "sdc.h"
#include "ff.h"

// Global GBC Memory
GBC_MMU_Memory_t GBC_MMU_Memory;
// Global SDC ROM file object
FIL GBC_MMU_SDC_ROMFile;
// Global SDC ROM file open state
bool GBC_MMU_SDC_ROMFileStreamOpen = false;

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

bool GBC_MMU_LoadFromSDC(char* fileName)
{
    if (!SDC_IsMounted())
    {
        return false;
    }

    if (GBC_MMU_SDC_ROMFileStreamOpen)
    {
        GBC_MMU_SDC_ROMFileStreamOpen = false;

        f_close(&GBC_MMU_SDC_ROMFile);
    }

    if (f_open(&GBC_MMU_SDC_ROMFile, fileName, FA_OPEN_ALWAYS | FA_READ) == FR_OK)
    {
        uint32_t bytesRead = 0;

        if (f_read(&GBC_MMU_SDC_ROMFile, GBC_MMU_Memory.Data, 32768, &bytesRead) == FR_OK && bytesRead == 32768)
        {
            GBC_MMU_SDC_ROMFileStreamOpen = true;
            return true;
        }

        f_close(&GBC_MMU_SDC_ROMFile);
    }

    return false;
}
