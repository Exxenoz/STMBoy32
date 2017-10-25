#include "gbc_mmu.h"
#include "sdc.h"
#include "ff.h"

// Global GBC Memory
GBC_MMU_Memory_t GBC_MMU_Memory;
// Global SDC ROM file object
FIL GBC_MMU_SDC_ROMFile;
// Global SDC ROM file open state
bool GBC_MMU_SDC_ROMFileStreamOpen = false;

bool GBC_MMU_LoadFromCartridge(void)
{
    // ToDo
    return false;
}

bool GBC_MMU_LoadFromSDC(char* fileName)
{
    if (!SDC_IsMounted())
    {
        return false;
    }

    GBC_MMU_Unload();

    if (f_open(&GBC_MMU_SDC_ROMFile, fileName, FA_OPEN_ALWAYS | FA_READ) == FR_OK)
    {
        uint32_t bytesRead = 0;

        // Read ROM Bank 0
        if (f_read(&GBC_MMU_SDC_ROMFile, GBC_MMU_Memory.CartridgeBank0, 16384, &bytesRead) == FR_OK && bytesRead == 16384)
        {
            GBC_MMU_SDC_ROMFileStreamOpen = true;
            return true;
        }

        f_close(&GBC_MMU_SDC_ROMFile);
    }

    return false;
}

void GBC_MMU_Unload(void)
{
    if (GBC_MMU_SDC_ROMFileStreamOpen)
    {
        GBC_MMU_SDC_ROMFileStreamOpen = false;

        f_close(&GBC_MMU_SDC_ROMFile);
    }
}

uint8_t GBC_MMU_ReadByte(uint16_t address)
{
    // Cartridge ROM bank 0
    if (address <= 0x3FFF)
    {
        return GBC_MMU_Memory.CartridgeBank0[address];
    }
    // Cartridge ROM bank X
    else if (address <= 0x7FFF)
    {
        // ToDo: Implement ROM bank reading
    }
    // Video RAM bank X
    else if (address <= 0x9FFF)
    {
        // ToDo: Implement VRAM bank switching
    }
    // External RAM bank X
    else if (address <= 0xBFFF)
    {
        // ToDo: Implement External RAM bank reading
    }
    // Work RAM Bank 0
    else if (address <= 0xCFFF)
    {
        return GBC_MMU_Memory.WRAMBank0[address - 0xC000];
    }
    // Work RAM Bank X
    else if (address <= 0xDFFF)
    {
        // ToDo: Implement WRAM bank switching
    }
    // Shadow RAM redirection to WRAM
    else if (address <= 0xFDFF)
    {
        return GBC_MMU_ReadByte(0xC000 + (address - 0xE000));
    }
    // Object Attribute Memory
    else if (address <= 0xFE9F)
    {
        return GBC_MMU_Memory.OAM[address - 0xFE00];
    }
    // Unused
    else if (address <= 0xFEFF)
    {
        // Do nothing
    }
    // Memory-mapped I/O
    else if (address <= 0xFF7F)
    {
        return GBC_MMU_Memory.IO[address - 0xFF00];
    }
    // High RAM and Interrupt Enable Register
    else if (address <= 0xFFFF)
    {
        return GBC_MMU_Memory.HRAM[address - 0xFF80];
    }

    return 0;
}

uint16_t GBC_MMU_ReadShort(uint16_t address)
{
    return GBC_MMU_ReadByte(address) | (GBC_MMU_ReadByte(address + 1) << 8);
}

void GBC_MMU_WriteByte(uint16_t address, uint8_t value)
{
    // Cartridge ROM bank 0
    if (address <= 0x3FFF)
    {
        GBC_MMU_Memory.CartridgeBank0[address] = value;
    }
    // Cartridge ROM bank X
    else if (address <= 0x7FFF)
    {
        // ToDo: Implement ROM bank writing
    }
    // Video RAM bank X
    else if (address <= 0x9FFF)
    {
        // ToDo: Implement VRAM bank switching
    }
    // External RAM bank X
    else if (address <= 0xBFFF)
    {
        // ToDo: Implement External RAM bank writing
    }
    // Work RAM Bank 0
    else if (address <= 0xCFFF)
    {
        GBC_MMU_Memory.WRAMBank0[address - 0xC000] = value;
    }
    // Work RAM Bank X
    else if (address <= 0xDFFF)
    {
        // ToDo: Implement WRAM bank switching
    }
    // Shadow RAM redirection to WRAM
    else if (address <= 0xFDFF)
    {
        GBC_MMU_WriteByte(0xC000 + (address - 0xE000), value);
    }
    // Object Attribute Memory
    else if (address <= 0xFE9F)
    {
        GBC_MMU_Memory.OAM[address - 0xFE00] = value;
    }
    // Unused
    else if (address <= 0xFEFF)
    {
        // Do nothing
    }
    // Memory-mapped I/O
    else if (address <= 0xFF7F)
    {
        GBC_MMU_Memory.IO[address - 0xFF00] = value;
    }
    // High RAM and Interrupt Enable Register
    else if (address <= 0xFFFF)
    {
        GBC_MMU_Memory.HRAM[address - 0xFF80] = value;
    }
}

void GBC_MMU_WriteShort(uint16_t address, uint16_t value)
{
    GBC_MMU_WriteByte(address, value & 0xFF);
    GBC_MMU_WriteByte(address + 1, (value & 0xFF00) >> 8);
}
