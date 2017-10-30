#include "gbc_mmu.h"
#include "gbc.h"
#include "sdc.h"
#include "ff.h"
#include "cmod.h"
#include "input.h"
#include "string.h"

GBC_MMU_Memory_t GBC_MMU_Memory;                                                   // GBC Memory
                                                                                   
FIL GBC_MMU_SDC_ROMFile;                                                           // SDC ROM file object
bool GBC_MMU_SDC_ROMFileStreamOpen = false;                                        // SDC ROM file open state

GBC_MMU_MemoryBankController_t GBC_MMU_MemoryBankController = GBC_MMU_MBC_UNKNOWN; // Current Memory Bank Controller
GBC_MMU_MBC1Mode_t GBC_MMU_MBC1Mode = GBC_MMU_MBC1_MODE_ROM;                       // ROM/RAM Mode Select
uint16_t GBC_MMU_CurrentROMBankID = 1;                                             // Current ROM Bank ID
uint16_t GBC_MMU_CurrentROMBankAddress = 16384;                                    // Current ROM Bank Start Address
                                                                                   
bool GBC_MMU_ERAMEnabled = false;                                                  // External RAM Enabled State
uint8_t GBC_MMU_CurrentERAMBankID = 0;                                             // Current ERAM Bank ID
uint16_t GBC_MMU_CurrentERAMBankAddress = 0;                                       // Current ERAM Bank Start Address
                                                                                   
bool GBC_MMU_RTC_Selected = false;                                                 // External RTC Selected State
GBC_MMU_RTC_Register_t GBC_MMU_RTC_Register;                                       // External RTC Register
uint8_t GBC_MMU_RTC_RegisterID = 0;                                                // External RTC Register ID
uint8_t GBC_MMU_RTC_LatchClockDataHelper = 0;                                      // Indicates if last write was zero
                                                                                   
const uint8_t GBC_MMU_NintendoLogo[48] =
{
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

// From Gambatte emulator
const uint8_t GBC_MMU_InitialValuesForFFXX[256] =
{
    0xCF, 0x00, 0x7E, 0xFF, 0xD3, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA, 0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
    0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
    0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
    0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
    0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
    0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
    0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
    0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00
};

// From Gambatte emulator
const uint8_t GBC_MMU_InitialValuesForColorFFXX[256] =
{
    0xCF, 0x00, 0x7C, 0xFF, 0x44, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
    0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
    0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
    0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
    0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
    0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
};

void GBC_MMU_Initialize(void)
{
    GBC_MMU_MemoryBankController = GBC_MMU_GetMemoryBankController();

    // Initialize memory
    if (GBC_MMU_Memory.CGBFlag & (GBC_MMU_CGB_FLAG_SUPPORTED | GBC_MMU_CGB_FLAG_ONLY))
    {
        for (long i = 0; i < 256; i++)
        {
            GBC_MMU_Memory.IO[i] = GBC_MMU_InitialValuesForColorFFXX[i];
        }
    }
    else
    {
        for (long i = 0; i < 256; i++)
        {
            GBC_MMU_Memory.IO[i] = GBC_MMU_InitialValuesForFFXX[i];
        }
    }
}

bool GBC_MMU_IsValidROMHeader(void)
{
    for (long i = 0; i < 48; i++)
    {
        if (GBC_MMU_Memory.NintendoLogo[i] != GBC_MMU_NintendoLogo[i])
        {
            return false;
        }
    }

    return true;
}

bool GBC_MMU_LoadFromCartridge(void)
{
    if (!CMOD_Detect())
    {
        return false;
    }

    GBC_MMU_Unload();

    // Read ROM Bank 0
    CMOD_ReadBytes(0x0000, 16384, GBC_MMU_Memory.CartridgeBank0);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    // Check ROM Header
    if (!GBC_MMU_IsValidROMHeader())
    {
        return false;
    }

    GBC_MMU_Initialize();

    return true;
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
        if (f_read(&GBC_MMU_SDC_ROMFile, GBC_MMU_Memory.CartridgeBank0, 16384, &bytesRead) != FR_OK || bytesRead != 16384)
        {
            f_close(&GBC_MMU_SDC_ROMFile);
            return false;
        }

        // Check ROM Header
        if (!GBC_MMU_IsValidROMHeader())
        {
            f_close(&GBC_MMU_SDC_ROMFile);
            return false;
        }
        
        GBC_MMU_SDC_ROMFileStreamOpen = true;

        GBC_MMU_Initialize();

        return true;
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

    memset(&GBC_MMU_Memory, 0, sizeof(GBC_MMU_Memory_t));

    GBC_MMU_MemoryBankController = GBC_MMU_MBC_UNKNOWN;
    GBC_MMU_MBC1Mode = GBC_MMU_MBC1_MODE_ROM;
    GBC_MMU_CurrentROMBankID = 1;
    GBC_MMU_CurrentROMBankAddress = 16384;

    GBC_MMU_ERAMEnabled = false;
    GBC_MMU_CurrentERAMBankID = 0;
    GBC_MMU_CurrentERAMBankAddress = 0;

    GBC_MMU_RTC_Selected = false;
    memset(&GBC_MMU_RTC_Register, 0, sizeof(GBC_MMU_RTC_Register_t));
    GBC_MMU_RTC_RegisterID = 0;
    GBC_MMU_RTC_LatchClockDataHelper = 0;
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
        if (GBC_MMU_Memory.VRAMBankID == 0)
        {
            return GBC_MMU_Memory.VRAMBank0[address - 0x8000];
        }
        else
        {
            return GBC_MMU_Memory.VRAMBank1[address - 0x8000];
        }
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
        if (GBC_MMU_Memory.WRAMBankID == 0)
        {
            return GBC_MMU_Memory.WRAMBank1[address - 0xD000];
        }
        else
        {
            return GBC_MMU_Memory.WRAMBank0[(4096 << (GBC_MMU_Memory.WRAMBankID - 1)) + (address - 0xD000)];
        }
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

    return 0xFF; // Return 0xFF for unused memory (!!!)
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

void (*GBC_MMU_MBC_Table[6])(uint16_t, uint8_t) =
{
    GBC_MMU_MBC_None,
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
        if (GBC_MMU_Memory.VRAMBankID == 0)
        {
            GBC_MMU_Memory.VRAMBank0[address - 0x8000] = value;
        }
        else
        {
            GBC_MMU_Memory.VRAMBank1[address - 0x8000] = value;
        }
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
        if (GBC_MMU_Memory.WRAMBankID == 0)
        {
            GBC_MMU_Memory.WRAMBank1[address - 0xD000] = value;
        }
        else
        {
            GBC_MMU_Memory.WRAMBank0[(4096 << (GBC_MMU_Memory.WRAMBankID - 1)) + (address - 0xD000)] = value;
        }
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

        switch (address)
        {
            case 0xFF00:
                Input_UpdateJoypadState();
                break;
        }
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

    return GBC_MMU_MBC_UNKNOWN;
}
