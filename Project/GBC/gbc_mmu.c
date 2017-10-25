#include "gbc_mmu.h"
#include "gbc.h"
#include "sdc.h"
#include "ff.h"
#include "cmod.h"

// GBC Memory
GBC_MMU_Memory_t GBC_MMU_Memory;
// SDC ROM file object
FIL GBC_MMU_SDC_ROMFile;
// SDC ROM file open state
bool GBC_MMU_SDC_ROMFileStreamOpen = false;

// Current Memory Bank Controller
GBC_MMU_MemoryBankController_t GBC_MMU_MemoryBankController = GBC_MMU_MBC_NONE;
// Current ROM Bank ID
uint16_t GBC_MMU_CurrentROMBankID = 1;
// Current ROM Bank Start Address
uint16_t GBC_MMU_CurrentROMBankAddress = 16384;
// ROM/RAM Mode Select
GBC_MMU_MBC1Mode_t GBC_MMU_MBC1Mode = GBC_MMU_MBC1_MODE_ROM;
// External RAM Enabled State
bool GBC_MMU_ERAMEnabled = false;
// Current ERAM Bank ID
uint8_t GBC_MMU_CurrentERAMBankID = 0;
// Current ERAM Bank Start Address
uint16_t GBC_MMU_CurrentERAMBankAddress = 0;
// External RTC Selected State (ERAM address space redirects
// to RTCRegister, when ERTC selected is true and ERAM is enabled)
bool GBC_MMU_RTC_Selected = false;
// External RTC Register
GBC_MMU_RTC_Register_t GBC_MMU_RTC_Register;
// External RTC Register ID
uint8_t GBC_MMU_RTC_RegisterID = 0;
// Used to indicate if last write was zero
uint8_t GBC_MMU_RTC_LatchClockDataHelper = 0;

void GBC_MMU_OnROMBank0Loaded()
{
    GBC_MMU_MemoryBankController = GBC_MMU_GetMemoryBankController();
}

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
            GBC_MMU_OnROMBank0Loaded();
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

    // Reset ROM Bank ID to 1
    GBC_MMU_CurrentROMBankID = 1;
    // Reset Current ROM Bank Start Address to 16384
    GBC_MMU_CurrentROMBankAddress = 16384;
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
        uint8_t result = 0;

        if (GBC_IsLoadedFromCartridge())
        {
            CMOD_ReadByte(address, &result);
            while (CMOD_GetStatus() == CMOD_PROCESSING);
            return result;
        }
        else
        {
            uint32_t bytesRead = 0;
            f_lseek(&GBC_MMU_SDC_ROMFile, GBC_MMU_CurrentROMBankAddress + (address - 0x4000));
            f_read(&GBC_MMU_SDC_ROMFile, &result, 1, &bytesRead);
            return result;
        }
    }
    // Video RAM bank X
    else if (address <= 0x9FFF)
    {
        // ToDo: Implement VRAM bank switching
    }
    // External RAM bank X
    else if (address <= 0xBFFF)
    {
        if (GBC_IsLoadedFromCartridge())
        {
            uint8_t result = 0;
            CMOD_ReadByte(address, &result);
            while (CMOD_GetStatus() == CMOD_PROCESSING);
            return result;
        }
        else if (GBC_MMU_ERAMEnabled)
        {
            if (GBC_MMU_RTC_Selected)
            {
                return GBC_MMU_RTC_Register.Data[GBC_MMU_RTC_RegisterID];
            }
            else
            {
                return GBC_MMU_Memory.ERAMBank0[GBC_MMU_CurrentERAMBankAddress + (address - 0xA000)];
            }
        }
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

void GBC_MMU_MBC_None(uint16_t address, uint8_t value)
{
    // Do nothing
}

void GBC_MMU_MBC1_Write(uint16_t address, uint8_t value)
{
    // RAM Enable
    if (address <= 0x1FFF)
    {
        GBC_MMU_ERAMEnabled = value & 0xA ? true : false;
    }
    // ROM Bank Number
    else if (address <= 0x3FFF)
    {
        GBC_MMU_CurrentROMBankID &= ~0x1F;        // Set first 5 bits to 0
        GBC_MMU_CurrentROMBankID |= value & 0x1F; // Set first 5 bits to value

        // Any attempt to address ROM Banks 00h, 20h, 40h, 60h will select Bank 21h, 41h, and 61h instead
        switch (GBC_MMU_CurrentROMBankID)
        {
            case 0x00:
            case 0x20:
            case 0x40:
            case 0x60:
                GBC_MMU_CurrentROMBankID++;
                break;
        }

        GBC_MMU_CurrentROMBankAddress = 16384 << (GBC_MMU_CurrentROMBankID - 1);
    }
    // RAM Bank Number - or - Upper Bits of ROM Bank Number
    else if (address <= 0x5FFF)
    {
        switch (GBC_MMU_MBC1Mode)
        {
            case GBC_MMU_MBC1_MODE_ROM: // Specify the upper two bits (Bit 6-7) of the ROM Bank number
                GBC_MMU_CurrentROMBankID &= ~0x60;        // Set bit 6 and 7 to 0
                GBC_MMU_CurrentROMBankID |= value & 0x60; // Set bit 6 and 7 to value

                // Any attempt to address ROM Banks 00h, 20h, 40h, 60h will select Bank 21h, 41h, and 61h instead
                switch (GBC_MMU_CurrentROMBankID)
                {
                    case 0x00:
                    case 0x20:
                    case 0x40:
                    case 0x60:
                        GBC_MMU_CurrentROMBankID++;
                        break;
                }

                GBC_MMU_CurrentROMBankAddress = 16384 << (GBC_MMU_CurrentROMBankID - 1);

                break;
            case GBC_MMU_MBC1_MODE_RAM: // Select a RAM Bank in range from 00-03h
                if (value <= 0x3)
                {
                    GBC_MMU_CurrentERAMBankID = value;
                    GBC_MMU_CurrentERAMBankAddress = (GBC_MMU_CurrentERAMBankID == 0) ? 0 : 8192 << (GBC_MMU_CurrentERAMBankID - 1);
                }
                break;
        }
    }
    // ROM/RAM Mode Select
    else // address <= 0x7FFF
    {
        GBC_MMU_MBC1Mode = (value == 0) ? GBC_MMU_MBC1_MODE_ROM : GBC_MMU_MBC1_MODE_RAM;
    }
}

void GBC_MMU_MBC2_Write(uint16_t address, uint8_t value)
{
    // RAM Enable
    if (address <= 0x1FFF)
    {
        // The least significant bit of the upper address byte must be zero to enable/disable cart RAM
        if (!(address & 0x100))
        {
            GBC_MMU_ERAMEnabled = (GBC_MMU_ERAMEnabled == true) ? false : true;
        }
    }
    // ROM Bank Number
    else if (address <= 0x3FFF)
    {
        if (address & 0x100)
        {
            GBC_MMU_CurrentROMBankID = value & 0xF;

            if (GBC_MMU_CurrentROMBankID == 0)
            {
                GBC_MMU_CurrentROMBankID++;
            }

            GBC_MMU_CurrentROMBankAddress = 16384 << (GBC_MMU_CurrentROMBankID - 1);
        }
    }
}

void GBC_MMU_MBC3_Write(uint16_t address, uint8_t value)
{
    // RAM and Timer Enable
    if (address <= 0x1FFF)
    {
        GBC_MMU_ERAMEnabled = value & 0xA ? true : false;
    }
    // ROM Bank Number
    else if (address <= 0x3FFF)
    {
        GBC_MMU_CurrentROMBankID = value & 0x7F;

        if (GBC_MMU_CurrentROMBankID == 0)
        {
            GBC_MMU_CurrentROMBankID++;
        }

        GBC_MMU_CurrentROMBankAddress = 16384 << (GBC_MMU_CurrentROMBankID - 1);
    }
    // RAM Bank Number 
    else if (address <= 0x5FFF)
    {
        if (value <= 0x3)
        {
            GBC_MMU_RTC_Selected = false;

            GBC_MMU_CurrentERAMBankID = value;
            GBC_MMU_CurrentERAMBankAddress = (GBC_MMU_CurrentERAMBankID == 0) ? 0 : 8192 << (GBC_MMU_CurrentERAMBankID - 1);
        }
        else if (value >= 0x8 && value <= 0xC)
        {
            GBC_MMU_RTC_Selected = true;
            GBC_MMU_RTC_RegisterID = value - 0x8;
        }
    }
    // Latch Clock Data
    else // address <= 0x7FFF
    {
        // When writing 00h, and then 01h to this variable, the current time becomes latched into the RTC registers
        if (GBC_MMU_RTC_LatchClockDataHelper == 0 && value == 1)
        {
            // ToDo: Write to RTC registers
        }

        GBC_MMU_RTC_LatchClockDataHelper = value;
    }
}

void GBC_MMU_MBC5_Write(uint16_t address, uint8_t value)
{
    // RAM Enable
    if (address <= 0x1FFF)
    {
        GBC_MMU_ERAMEnabled = value & 0xA ? true : false;
    }
    // Low 8 bits of ROM Bank Number
    else if (address <= 0x2FFF)
    {
        GBC_MMU_CurrentROMBankID &= ~0xFF;        // Set first 8 bits to 0
        GBC_MMU_CurrentROMBankID |= value & 0xFF; // Set first 8 bits to value
        GBC_MMU_CurrentROMBankAddress = (GBC_MMU_CurrentROMBankID == 0) ? 0 : 16384 << (GBC_MMU_CurrentROMBankID - 1);
    }
    // High bit of ROM Bank Number
    else if (address <= 0x3FFF)
    {
        GBC_MMU_CurrentROMBankID &= ~0x100;             // Set bit 9 to 0
        GBC_MMU_CurrentROMBankID |= (value & 0x1) << 8; // Set bit 9 to value
        GBC_MMU_CurrentROMBankAddress = (GBC_MMU_CurrentROMBankID == 0) ? 0 : 16384 << (GBC_MMU_CurrentROMBankID - 1);
    }
    // RAM Bank Number
    else if (address <= 0x5FFF)
    {
        if (value <= 0xF)
        {
            GBC_MMU_CurrentERAMBankID = value;
            GBC_MMU_CurrentERAMBankAddress = (GBC_MMU_CurrentERAMBankID == 0) ? 0 : 8192 << (GBC_MMU_CurrentERAMBankID - 1);
        }
    }
}

void (*GBC_MMU_MBC_Table[5])(uint16_t, uint8_t) =
{
    GBC_MMU_MBC_None,
    GBC_MMU_MBC1_Write,
    GBC_MMU_MBC2_Write,
    GBC_MMU_MBC3_Write,
    GBC_MMU_MBC5_Write,
};

void GBC_MMU_WriteByte(uint16_t address, uint8_t value)
{
    // Memory Bank Switch
    if (address <= 0x7FFF)
    {
        if (GBC_IsLoadedFromCartridge())
        {
            CMOD_WriteByte(address, &value);
            while (CMOD_GetStatus() == CMOD_PROCESSING);
        }
        else // Loaded from SDC
        {
            GBC_MMU_MBC_Table[GBC_MMU_MemoryBankController](address, value);
        }
    }
    // Video RAM bank X
    else if (address <= 0x9FFF)
    {
        // ToDo: Implement VRAM bank switching
    }
    // External RAM bank X
    else if (address <= 0xBFFF)
    {
        if (GBC_IsLoadedFromCartridge())
        {
            CMOD_WriteByte(address, &value);
            while (CMOD_GetStatus() == CMOD_PROCESSING);
        }
        else if (GBC_MMU_ERAMEnabled)
        {
            if (GBC_MMU_RTC_Selected)
            {
                GBC_MMU_RTC_Register.Data[GBC_MMU_RTC_RegisterID] = value;
            }
            else
            {
                GBC_MMU_Memory.ERAMBank0[GBC_MMU_CurrentERAMBankAddress + (address - 0xA000)] = value;
            }
        }
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

GBC_MMU_MemoryBankController_t GBC_MMU_GetMemoryBankController(void)
{
    switch (GBC_MMU_Memory.CartridgeType)
    {
        case GBC_MMU_CARTRIDGE_TYPE_ROM_ONLY:
        case GBC_MMU_CARTRIDGE_TYPE_ROM_RAM:
        case GBC_MMU_CARTRIDGE_TYPE_ROM_RAM_BATTERY:
            return GBC_MMU_MBC_NONE;
        case GBC_MMU_CARTRIDGE_TYPE_MBC1:
        case GBC_MMU_CARTRIDGE_TYPE_MBC1_RAM:
        case GBC_MMU_CARTRIDGE_TYPE_MBC1_RAM_BATTERY:
            return GBC_MMU_MBC1;
        case GBC_MMU_CARTRIDGE_TYPE_MBC2:
        case GBC_MMU_CARTRIDGE_TYPE_MBC2_BATTERY:
            return GBC_MMU_MBC2;
        case GBC_MMU_CARTRIDGE_TYPE_MBC3_TIMER_BATTERY:
        case GBC_MMU_CARTRIDGE_TYPE_MBC3_TIMER_RAM_BATTERY:
        case GBC_MMU_CARTRIDGE_TYPE_MBC3:
        case GBC_MMU_CARTRIDGE_TYPE_MBC3_RAM:
        case GBC_MMU_CARTRIDGE_TYPE_MBC3_RAM_BATTERY:
            return GBC_MMU_MBC3;
        case GBC_MMU_CARTRIDGE_TYPE_MBC5:
        case GBC_MMU_CARTRIDGE_TYPE_MBC5_RAM:
        case GBC_MMU_CARTRIDGE_TYPE_MBC5_RAM_BATTERY:
        case GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE:
        case GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE_RAM:
        case GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE_RAM_BATTERY:
            return GBC_MMU_MBC5;
    }

    return GBC_MMU_MBC_NONE;
}
