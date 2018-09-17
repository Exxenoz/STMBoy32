#include "gbc_mmu.h"
#include "gbc_tim.h"
#include "gbc_apu.h"
#include "gbc_gpu.h"
#include "gbc.h"
#include "sdc.h"
#include "ff.h"
#include "cmod.h"
#include "input.h"
#include "string.h"
#include "sdram.h"

GBC_MMU_Memory_t GBC_MMU_Memory;                                                   // GBC Memory
GBC_MMU_MemoryBankController_t GBC_MMU_MemoryBankController = GBC_MMU_MBC_UNKNOWN; // Current Memory Bank Controller
GBC_MMU_MBC1Mode_t GBC_MMU_MBC1Mode = GBC_MMU_MBC1_MODE_ROM;                       // ROM/RAM Mode Select
uint16_t GBC_MMU_CurrentROMBankID = 1;                                             // Current ROM Bank ID
uint32_t GBC_MMU_CurrentROMBankAddress = 16384;                                    // Current ROM Bank Start Address

bool GBC_MMU_ERAMEnabled = false;                                                  // External RAM Enabled State
uint8_t GBC_MMU_CurrentERAMBankID = 0;                                             // Current ERAM Bank ID

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
    if (GBC_MMU_IS_CGB_MODE())
    {
        for (long i = 0; i < 128; i++)
        {
            GBC_MMU_Memory.IO[i] = GBC_MMU_InitialValuesForColorFFXX[i];
        }

        for (long i = 128, j = 0; i < 255; i++, j++)
        {
            GBC_MMU_Memory.HRAM[j] = GBC_MMU_InitialValuesForColorFFXX[i];
        }

        GBC_MMU_Memory.InterruptEnable = GBC_MMU_InitialValuesForColorFFXX[255];
    }
    else
    {
        for (long i = 0; i < 128; i++)
        {
            GBC_MMU_Memory.IO[i] = GBC_MMU_InitialValuesForFFXX[i];
        }

        for (long i = 128, j = 0; i < 255; i++, j++)
        {
            GBC_MMU_Memory.HRAM[j] = GBC_MMU_InitialValuesForFFXX[i];
        }

        GBC_MMU_Memory.InterruptEnable = GBC_MMU_InitialValuesForFFXX[255];
    }
}

bool GBC_MMU_IsValidROMHeader(void)
{
    for (long i = 0; i < 48; i++)
    {
        if (GBC_MMU_Memory.CartridgeBank0.NintendoLogo[i] != GBC_MMU_NintendoLogo[i])
        {
            return false;
        }
    }

    return true;
}

bool GBC_MMU_LoadFromCartridge(void)
{
    if (!CMOD_CheckForCartridge())
    {
        return false;
    }

    GBC_MMU_Unload();

    // Read ROM Bank 0
    CMOD_ReadBytes(0x0000, 16384, GBC_MMU_Memory.CartridgeBank0.Data);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    // Check ROM Header
    if (!GBC_MMU_IsValidROMHeader())
    {
        return false;
    }

    // ToDo: Read complete ROM and write it to SDRAM

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

    FIL ROMFile;
    if (f_open(&ROMFile, fileName, FA_OPEN_EXISTING | FA_READ) == FR_OK)
    {
        uint32_t bytesRead = 0;
        uint8_t readBuffer[512];

        // Read ROM Bank 0
        for (long i = 0; i < 16384; i += 512)
        {
            f_lseek(&ROMFile, i);

            if (f_read(&ROMFile, &GBC_MMU_Memory.CartridgeBank0.Data[i], 512, &bytesRead) != FR_OK || bytesRead != 512)
            {
                f_close(&ROMFile);
                return false;
            }
        }

        // Check ROM Header
        if (!GBC_MMU_IsValidROMHeader())
        {
            f_close(&ROMFile);
            return false;
        }

        // Read complete ROM and write it to SDRAM
        for (uint32_t position = 0, address = SDRAM_BANK_ADDR + WRITE_READ_ADDR; bytesRead; position += 512, address += 512)
        {
            f_lseek(&ROMFile, position);

            if (f_read(&ROMFile, &readBuffer, 512, &bytesRead) != FR_OK || (bytesRead != 0 && bytesRead != 512))
            {
                f_close(&ROMFile);
                return false;
            }

            memcpy((void*)address, readBuffer, 512);
        }

        f_close(&ROMFile);

        GBC_MMU_Initialize();

        return true;
    }

    return false;
}

void GBC_MMU_Unload(void)
{
    memset(&GBC_MMU_Memory, 0, sizeof(GBC_MMU_Memory_t));

    GBC_MMU_MemoryBankController = GBC_MMU_MBC_UNKNOWN;
    GBC_MMU_MBC1Mode = GBC_MMU_MBC1_MODE_ROM;
    GBC_MMU_CurrentROMBankID = 1;
    GBC_MMU_CurrentROMBankAddress = 16384;

    GBC_MMU_ERAMEnabled = false;
    GBC_MMU_CurrentERAMBankID = 0;

    GBC_MMU_RTC_Selected = false;
    memset(&GBC_MMU_RTC_Register, 0, sizeof(GBC_MMU_RTC_Register_t));
    GBC_MMU_RTC_RegisterID = 0;
    GBC_MMU_RTC_LatchClockDataHelper = 0;
}

uint8_t GBC_MMU_ReadByte(uint16_t address)
{
    switch (address & 0xF000)
    {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            // Cartridge ROM bank 0
            return GBC_MMU_Memory.CartridgeBank0.Data[address];
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        {
            // Cartridge ROM bank X
            return SDRAM_READ_BYTE(SDRAM_BANK_ADDR + WRITE_READ_ADDR + GBC_MMU_CurrentROMBankAddress + (address - 0x4000));
        }
        case 0x8000:
        case 0x9000:
            // Video RAM bank X
            return (GBC_MMU_Memory.VRAMBankID == 1) ? GBC_MMU_Memory.VRAMBank1.Data[address - 0x8000] : GBC_MMU_Memory.VRAMBank0.Data[address - 0x8000];
        case 0xA000:
        case 0xB000:
            // External RAM bank X
            if (GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE)
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
                else switch (GBC_MMU_CurrentERAMBankID)
                {
                    case 0:
                        return GBC_MMU_Memory.ERAMBank0[address - 0xA000];
                    case 1:
                        return GBC_MMU_Memory.ERAMBank1[address - 0xA000];
                    case 2:
                        return GBC_MMU_Memory.ERAMBank2[address - 0xA000];
                    case 3:
                        return GBC_MMU_Memory.ERAMBank3[address - 0xA000];
                }
            }
            break;
        case 0xC000:
            // Work RAM Bank 0
            return GBC_MMU_Memory.WRAMBank0[address - 0xC000];
        case 0xD000:
            // Work RAM Bank X
            switch (GBC_MMU_Memory.WRAMBankID)
            {
                case 0:
                case 1:
                    return GBC_MMU_Memory.WRAMBank1[address - 0xD000];
                case 2:
                    return GBC_MMU_Memory.WRAMBank2[address - 0xD000];
                case 3:
                    return GBC_MMU_Memory.WRAMBank3[address - 0xD000];
                case 4:
                    return GBC_MMU_Memory.WRAMBank4[address - 0xD000];
                case 5:
                    return GBC_MMU_Memory.WRAMBank5[address - 0xD000];
                case 6:
                    return GBC_MMU_Memory.WRAMBank6[address - 0xD000];
                case 7:
                    return GBC_MMU_Memory.WRAMBank7[address - 0xD000];
            }

            return GBC_MMU_Memory.WRAMBank1[address - 0xD000];
        case 0xE000:
            // Shadow RAM redirection to WRAM Bank 0
            return GBC_MMU_Memory.WRAMBank0[address - 0xE000];
        case 0xF000:
            switch (address & 0xFF00)
            {
                case 0xFE00:
                    // Object Attribute Memory
                    if (address <= 0xFE9F)
                    {
                        return GBC_MMU_Memory.OAM[address - 0xFE00];
                    }
                    // else unused memory
                    break;
                case 0xFF00:
                    // Memory-mapped I/O
                    if (address >= 0xFF10 && address <= 0xFF3F) // Sound Register
                    {
                        static const uint8_t soundRegisterReadMask[] =
                        {
                            0x80, 0x3F, 0x00, 0xFF, 0xBF, // Sound Channel 1 Register Read Mask
                            0xFF, 0x3F, 0x00, 0xFF, 0xBF, // Sound Channel 2 Register Read Mask (first register byte unused/unknown)
                            0x7F, 0xFF, 0x9F, 0xFF, 0xBF, // Sound Channel 3 Register Read Mask
                            0xFF, 0xFF, 0x00, 0x00, 0xBF, // Sound Channel 4 Register Read Mask (first register byte unused/unknown)
                            0x00, 0x00, 0x70,             // Sound Control Registers
                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // Unused/unknown register bytes
                        };

                        if (address <= 0xFF2F)
                        {
                            return GBC_MMU_Memory.IO[address - 0xFF00] | soundRegisterReadMask[address - 0xFF10];
                        }
                        else // Wave RAM
                        {
                            if (GBC_MMU_Memory.ChannelSound3Enabled)
                            {
                                address = GBC_APU_Channel3Phase;

                                if (GBC_MMU_IS_DMG_MODE())
                                {
                                    ++address;

                                    if (GBC_APU_Channel3PhaseTicks < -1)
                                    {
                                        return 0xFF;
                                    }
                                }

                                address >>= 1;
                            }

                            address &= 0x0F;

                            return GBC_MMU_Memory.Channel3WavePatternRAM[address];
                        }
                    }
                    else if (address <= 0xFF7F)
                    {
                        return GBC_MMU_Memory.IO[address - 0xFF00];
                    }
                    else // High RAM and Interrupt Enable Register
                    {
                        return GBC_MMU_Memory.HRAM[address - 0xFF80];
                    }
                default:
                    // Shadow RAM redirection to WRAM Bank X
                    switch (GBC_MMU_Memory.WRAMBankID)
                    {
                        case 0:
                        case 1:
                            return GBC_MMU_Memory.WRAMBank1[address - 0xF000];
                        case 2:
                            return GBC_MMU_Memory.WRAMBank2[address - 0xF000];
                        case 3:
                            return GBC_MMU_Memory.WRAMBank3[address - 0xF000];
                        case 4:
                            return GBC_MMU_Memory.WRAMBank4[address - 0xF000];
                        case 5:
                            return GBC_MMU_Memory.WRAMBank5[address - 0xF000];
                        case 6:
                            return GBC_MMU_Memory.WRAMBank6[address - 0xF000];
                        case 7:
                            return GBC_MMU_Memory.WRAMBank7[address - 0xF000];
                    }

                    return GBC_MMU_Memory.WRAMBank1[address - 0xF000];
            }
            break;
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

        GBC_MMU_CurrentROMBankAddress = 16384 * GBC_MMU_CurrentROMBankID;
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

                GBC_MMU_CurrentROMBankAddress = 16384 * GBC_MMU_CurrentROMBankID;

                break;
            case GBC_MMU_MBC1_MODE_RAM: // Select a RAM Bank in range from 00-03h
                if (value <= 0x3)
                {
                    GBC_MMU_CurrentERAMBankID = value;
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

            GBC_MMU_CurrentROMBankAddress = 16384 * GBC_MMU_CurrentROMBankID;
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

        GBC_MMU_CurrentROMBankAddress = 16384 * GBC_MMU_CurrentROMBankID;
    }
    // RAM Bank Number 
    else if (address <= 0x5FFF)
    {
        if (value <= 0x3)
        {
            GBC_MMU_RTC_Selected = false;

            GBC_MMU_CurrentERAMBankID = value;
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
        GBC_MMU_CurrentROMBankAddress = (GBC_MMU_CurrentROMBankID == 0) ? 0 : 16384 * GBC_MMU_CurrentROMBankID;
    }
    // High bit of ROM Bank Number
    else if (address <= 0x3FFF)
    {
        GBC_MMU_CurrentROMBankID &= ~0x100;             // Set bit 9 to 0
        GBC_MMU_CurrentROMBankID |= (value & 0x1) << 8; // Set bit 9 to value
        GBC_MMU_CurrentROMBankAddress = (GBC_MMU_CurrentROMBankID == 0) ? 0 : 16384 * GBC_MMU_CurrentROMBankID;
    }
    // RAM Bank Number
    else if (address <= 0x5FFF)
    {
        if (value <= 0xF)
        {
            GBC_MMU_CurrentERAMBankID = value;
        }
    }
}

GBC_MMU_MBC GBC_MMU_MBC_Table[6] =
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
    switch (address & 0xF000)
    {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        {
            // Memory Bank Switch
            if (GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE)
            {
                CMOD_WriteByte(address, &value);
                while (CMOD_GetStatus() == CMOD_PROCESSING);
            }
            else // Loaded from SDC
            {
                GBC_MMU_MBC_Table[GBC_MMU_MemoryBankController](address, value);
            }
            break;
        }
        case 0x8000:
        case 0x9000:
        {
            // Video RAM bank X
            if (GBC_MMU_Memory.VRAMBankID == 1)
            {
                GBC_MMU_Memory.VRAMBank1.Data[address - 0x8000] = value;
            }
            else
            {
                GBC_MMU_Memory.VRAMBank0.Data[address - 0x8000] = value;
            }
            break;
        }
        case 0xA000:
        case 0xB000:
        {
            // External RAM bank X
            if (GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE)
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
                else switch (GBC_MMU_CurrentERAMBankID)
                {
                    case 0:
                        GBC_MMU_Memory.ERAMBank0[address - 0xA000] = value;
                        break;
                    case 1:
                        GBC_MMU_Memory.ERAMBank1[address - 0xA000] = value;
                        break;
                    case 2:
                        GBC_MMU_Memory.ERAMBank2[address - 0xA000] = value;
                        break;
                    case 3:
                        GBC_MMU_Memory.ERAMBank3[address - 0xA000] = value;
                        break;
                }
            }
            break;
        }
        case 0xC000:
        {
            // Work RAM Bank 0
            GBC_MMU_Memory.WRAMBank0[address - 0xC000] = value;
            break;
        }
        case 0xD000:
        {
            // Work RAM Bank X
            switch (GBC_MMU_Memory.WRAMBankID)
            {
                case 0:
                case 1:
                    GBC_MMU_Memory.WRAMBank1[address - 0xD000] = value;
                    break;
                case 2:
                    GBC_MMU_Memory.WRAMBank2[address - 0xD000] = value;
                    break;
                case 3:
                    GBC_MMU_Memory.WRAMBank3[address - 0xD000] = value;
                    break;
                case 4:
                    GBC_MMU_Memory.WRAMBank4[address - 0xD000] = value;
                    break;
                case 5:
                    GBC_MMU_Memory.WRAMBank5[address - 0xD000] = value;
                    break;
                case 6:
                    GBC_MMU_Memory.WRAMBank6[address - 0xD000] = value;
                    break;
                case 7:
                    GBC_MMU_Memory.WRAMBank7[address - 0xD000] = value;
                    break;
                default:
                    GBC_MMU_Memory.WRAMBank1[address - 0xD000] = value;
                    break;
            }
            break;
        }
        case 0xE000:
        {
            // Shadow RAM redirection to WRAM Bank 0
            GBC_MMU_Memory.WRAMBank0[address - 0xE000] = value;
            break;
        }
        case 0xF000:
        {
            switch (address)
            {
                // Object Attribute Memory
                case 0xFE00: case 0xFE01: case 0xFE02: case 0xFE03: case 0xFE04: case 0xFE05: case 0xFE06: case 0xFE07:
                case 0xFE08: case 0xFE09: case 0xFE0A: case 0xFE0B: case 0xFE0C: case 0xFE0D: case 0xFE0E: case 0xFE0F:
                case 0xFE10: case 0xFE11: case 0xFE12: case 0xFE13: case 0xFE14: case 0xFE15: case 0xFE16: case 0xFE17:
                case 0xFE18: case 0xFE19: case 0xFE1A: case 0xFE1B: case 0xFE1C: case 0xFE1D: case 0xFE1E: case 0xFE1F:
                case 0xFE20: case 0xFE21: case 0xFE22: case 0xFE23: case 0xFE24: case 0xFE25: case 0xFE26: case 0xFE27:
                case 0xFE28: case 0xFE29: case 0xFE2A: case 0xFE2B: case 0xFE2C: case 0xFE2D: case 0xFE2E: case 0xFE2F:
                case 0xFE30: case 0xFE31: case 0xFE32: case 0xFE33: case 0xFE34: case 0xFE35: case 0xFE36: case 0xFE37:
                case 0xFE38: case 0xFE39: case 0xFE3A: case 0xFE3B: case 0xFE3C: case 0xFE3D: case 0xFE3E: case 0xFE3F:
                case 0xFE40: case 0xFE41: case 0xFE42: case 0xFE43: case 0xFE44: case 0xFE45: case 0xFE46: case 0xFE47:
                case 0xFE48: case 0xFE49: case 0xFE4A: case 0xFE4B: case 0xFE4C: case 0xFE4D: case 0xFE4E: case 0xFE4F:
                case 0xFE50: case 0xFE51: case 0xFE52: case 0xFE53: case 0xFE54: case 0xFE55: case 0xFE56: case 0xFE57:
                case 0xFE58: case 0xFE59: case 0xFE5A: case 0xFE5B: case 0xFE5C: case 0xFE5D: case 0xFE5E: case 0xFE5F:
                case 0xFE60: case 0xFE61: case 0xFE62: case 0xFE63: case 0xFE64: case 0xFE65: case 0xFE66: case 0xFE67:
                case 0xFE68: case 0xFE69: case 0xFE6A: case 0xFE6B: case 0xFE6C: case 0xFE6D: case 0xFE6E: case 0xFE6F:
                case 0xFE70: case 0xFE71: case 0xFE72: case 0xFE73: case 0xFE74: case 0xFE75: case 0xFE76: case 0xFE77:
                case 0xFE78: case 0xFE79: case 0xFE7A: case 0xFE7B: case 0xFE7C: case 0xFE7D: case 0xFE7E: case 0xFE7F:
                case 0xFE80: case 0xFE81: case 0xFE82: case 0xFE83: case 0xFE84: case 0xFE85: case 0xFE86: case 0xFE87:
                case 0xFE88: case 0xFE89: case 0xFE8A: case 0xFE8B: case 0xFE8C: case 0xFE8D: case 0xFE8E: case 0xFE8F:
                case 0xFE90: case 0xFE91: case 0xFE92: case 0xFE93: case 0xFE94: case 0xFE95: case 0xFE96: case 0xFE97:
                case 0xFE98: case 0xFE99: case 0xFE9A: case 0xFE9B: case 0xFE9C: case 0xFE9D: case 0xFE9E: case 0xFE9F:
                    GBC_MMU_Memory.OAM[address - 0xFE00] = value;
                    break;
                // Unused Memory
                case 0xFEA0: case 0xFEA1: case 0xFEA2: case 0xFEA3: case 0xFEA4: case 0xFEA5: case 0xFEA6: case 0xFEA7:
                case 0xFEA8: case 0xFEA9: case 0xFEAA: case 0xFEAB: case 0xFEAC: case 0xFEAD: case 0xFEAE: case 0xFEAF:
                case 0xFEB0: case 0xFEB1: case 0xFEB2: case 0xFEB3: case 0xFEB4: case 0xFEB5: case 0xFEB6: case 0xFEB7:
                case 0xFEB8: case 0xFEB9: case 0xFEBA: case 0xFEBB: case 0xFEBC: case 0xFEBD: case 0xFEBE: case 0xFEBF:
                case 0xFEC0: case 0xFEC1: case 0xFEC2: case 0xFEC3: case 0xFEC4: case 0xFEC5: case 0xFEC6: case 0xFEC7:
                case 0xFEC8: case 0xFEC9: case 0xFECA: case 0xFECB: case 0xFECC: case 0xFECD: case 0xFECE: case 0xFECF:
                case 0xFED0: case 0xFED1: case 0xFED2: case 0xFED3: case 0xFED4: case 0xFED5: case 0xFED6: case 0xFED7:
                case 0xFED8: case 0xFED9: case 0xFEDA: case 0xFEDB: case 0xFEDC: case 0xFEDD: case 0xFEDE: case 0xFEDF:
                case 0xFEE0: case 0xFEE1: case 0xFEE2: case 0xFEE3: case 0xFEE4: case 0xFEE5: case 0xFEE6: case 0xFEE7:
                case 0xFEE8: case 0xFEE9: case 0xFEEA: case 0xFEEB: case 0xFEEC: case 0xFEED: case 0xFEEE: case 0xFEEF:
                case 0xFEF0: case 0xFEF1: case 0xFEF2: case 0xFEF3: case 0xFEF4: case 0xFEF5: case 0xFEF6: case 0xFEF7:
                case 0xFEF8: case 0xFEF9: case 0xFEFA: case 0xFEFB: case 0xFEFC: case 0xFEFD: case 0xFEFE: case 0xFEFF:
                    // Do nothing
                    break;
                // Memory-mapped I/O
                case 0xFF00: // Joypad
                    GBC_MMU_Memory.Joypad = value;
                    Input_UpdateGBCJoypad();
                    break;
                case 0xFF01:
                case 0xFF02:
                case 0xFF03:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF04: // Timer Divider: Writing any value to this register resets it to 0
                    GBC_TIM_ResetDivider();
                    break;
                case 0xFF05:
                case 0xFF06:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF07: // Timer Control
                    value &= 0x07;

                    if (GBC_MMU_Memory.TimerRunning != (value & 0x03))
                    {
                        GBC_TIM_ResetCounter();
                    }

                    GBC_MMU_Memory.TimerControl = value;
                    break;
                case 0xFF08:
                case 0xFF09:
                case 0xFF0A:
                case 0xFF0B:
                case 0xFF0C:
                case 0xFF0D:
                case 0xFF0E:
                case 0xFF0F:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF10:
                case 0xFF11:
                case 0xFF12:
                case 0xFF13:
                case 0xFF14:
                case 0xFF15:
                case 0xFF16:
                case 0xFF17:
                case 0xFF18:
                case 0xFF19:
                case 0xFF1A:
                case 0xFF1B:
                case 0xFF1C:
                case 0xFF1D:
                case 0xFF1E:
                case 0xFF1F:
                case 0xFF20:
                case 0xFF21:
                case 0xFF22:
                case 0xFF23:
                case 0xFF24:
                case 0xFF25:
                {
                    if (!GBC_MMU_Memory.ChannelSoundsEnabled)
                    {
                        // Read-only in CGB mode
                        if (GBC_MMU_IS_CGB_MODE())
                        {
                            break;
                        }

                        // Length registers can be written in DMG mode
                        if (address == 0xFF11 || address == 0xFF16)
                        {
                            // Clear wave pattern duty
                            value &= 0x3F;
                        }
                        else if (address == 0xFF1B || address == 0xFF20)
                        {
                            // Do nothing
                        }
                        else
                        {
                            break;
                        }
                    }

                    uint8_t oldValue = GBC_MMU_Memory.IO[address - 0xFF00];
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    GBC_APU_OnWriteToSoundRegister(address, value, oldValue);
                    break;
                }
                case 0xFF26:
                    if (value & 0x80)
                    {
                        GBC_MMU_Memory.ChannelSoundsEnabled = 1;
                    }
                    else if (GBC_MMU_Memory.ChannelSoundsEnabled)
                    {
                        GBC_MMU_Memory.ChannelSoundTerminal = 0;

                        // Disabeling the sound controller by clearing Bit 7 destroys the contents of all sound registers.

                        for (long i = 0x10; i < 0x30; i++)
                        {
                            GBC_MMU_Memory.IO[i] = 0;
                        }

                        // Reinitialize sound module
                        GBC_APU_Initialize();
                    }
                    // Other 7 bits of 0xFF26 are read only
                    break;
                case 0xFF27:
                case 0xFF28:
                case 0xFF29:
                case 0xFF2A:
                case 0xFF2B:
                case 0xFF2C:
                case 0xFF2D:
                case 0xFF2E:
                case 0xFF2F:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF30: // Wave Pattern RAM Start
                case 0xFF31:
                case 0xFF32:
                case 0xFF33:
                case 0xFF34:
                case 0xFF35:
                case 0xFF36:
                case 0xFF37:
                case 0xFF38:
                case 0xFF39:
                case 0xFF3A:
                case 0xFF3B:
                case 0xFF3C:
                case 0xFF3D:
                case 0xFF3E:
                case 0xFF3F: // Wave Pattern RAM End
                    if (GBC_MMU_Memory.ChannelSound3Enabled)
                    {
                        address = GBC_APU_Channel3Phase;

                        if (GBC_MMU_IS_DMG_MODE())
                        {
                            ++address;

                            if (GBC_APU_Channel3PhaseTicks < -1)
                            {
                                break;
                            }
                        }

                        address >>= 1;
                    }

                    address &= 0x0F;

                    GBC_MMU_Memory.Channel3WavePatternRAM[address] = value;
                    break;
                case 0xFF40:
                case 0xFF41:
                case 0xFF42:
                case 0xFF43:
                case 0xFF44:
                case 0xFF45:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF46: // DMA Transfer and Start Address
                    GBC_MMU_Memory.OAMTransferStartAddress = value;

                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        // ToDo
                    }
                    else
                    {
                        // The written value specifies the transfer source address divided by 100h
                        uint16_t address = value << 8;

                        if (address >= 0x8000 && address < 0xE000)
                        {
                            for (uint32_t i = 0; i < 160; i++, address++)
                            {
                                GBC_MMU_Memory.OAM[i] = GBC_MMU_ReadByte(address);
                            }
                        }
                    }
                    break;
                case 0xFF47:
                case 0xFF48:
                case 0xFF49:
                case 0xFF4A:
                case 0xFF4B:
                case 0xFF4C:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF4D: // SpeedSwitch
                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        GBC_MMU_Memory.PrepareSpeedSwitch = (value & 0x01);
                    }
                    else
                    {
                        GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    }
                    break;
                case 0xFF4E:
                case 0xFF4F:
                case 0xFF50:
                case 0xFF51:
                case 0xFF52:
                case 0xFF53:
                case 0xFF54:
                case 0xFF55:
                case 0xFF56:
                case 0xFF57:
                case 0xFF58:
                case 0xFF59:
                case 0xFF5A:
                case 0xFF5B:
                case 0xFF5C:
                case 0xFF5D:
                case 0xFF5E:
                case 0xFF5F:
                case 0xFF60:
                case 0xFF61:
                case 0xFF62:
                case 0xFF63:
                case 0xFF64:
                case 0xFF65:
                case 0xFF66:
                case 0xFF67:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                case 0xFF68: // BackgroundPaletteIndexData
                    GBC_MMU_Memory.BackgroundPaletteIndexData = value;

                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        GBC_MMU_Memory.BackgroundPaletteData = GBC_GPU_FetchBackgroundPaletteColor(GBC_MMU_Memory.BackgroundPaletteHL,
                            GBC_MMU_Memory.BackgroundPaletteIndex, GBC_MMU_Memory.BackgroundPaletteColorIndex);
                    }
                    break;
                case 0xFF69: // BackgroundPaletteData
                    GBC_MMU_Memory.BackgroundPaletteData = value;

                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        GBC_GPU_SetBackgroundPaletteColor(GBC_MMU_Memory.BackgroundPaletteHL,
                            GBC_MMU_Memory.BackgroundPaletteIndex, GBC_MMU_Memory.BackgroundPaletteColorIndex, value);

                        if (GBC_MMU_Memory.BackgroundPaletteIndexAutoIncrement)
                        {
                            uint8_t index = GBC_MMU_Memory.BackgroundPaletteIndexData & 0x3F; ++index; index &= 0x3F;
                            GBC_MMU_Memory.BackgroundPaletteIndexData = (GBC_MMU_Memory.BackgroundPaletteIndexData & 0x80) | index;
                            GBC_MMU_Memory.BackgroundPaletteData = GBC_GPU_FetchBackgroundPaletteColor(GBC_MMU_Memory.BackgroundPaletteHL,
                                GBC_MMU_Memory.BackgroundPaletteIndex, GBC_MMU_Memory.BackgroundPaletteColorIndex);
                        }
                    }
                    break;
                case 0xFF6A: // SpritePaletteIndexData
                    GBC_MMU_Memory.SpritePaletteIndexData = value;

                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        GBC_MMU_Memory.SpritePaletteData = GBC_GPU_FetchSpritePaletteColor(GBC_MMU_Memory.SpritePaletteHL,
                            GBC_MMU_Memory.SpritePaletteIndex, GBC_MMU_Memory.SpritePaletteColorIndex);
                    }
                    break;
                case 0xFF6B: // SpritePaletteData
                    GBC_MMU_Memory.SpritePaletteData = value;

                    if (GBC_MMU_IS_CGB_MODE())
                    {
                        GBC_GPU_SetSpritePaletteColor(GBC_MMU_Memory.SpritePaletteHL,
                            GBC_MMU_Memory.SpritePaletteIndex, GBC_MMU_Memory.SpritePaletteColorIndex, value);

                        if (GBC_MMU_Memory.SpritePaletteIndexAutoIncrement)
                        {
                            uint8_t index = GBC_MMU_Memory.SpritePaletteIndexData & 0x3F; ++index; index &= 0x3F;
                            GBC_MMU_Memory.SpritePaletteIndexData = (GBC_MMU_Memory.SpritePaletteIndexData & 0x80) | index;
                            GBC_MMU_Memory.SpritePaletteData = GBC_GPU_FetchSpritePaletteColor(GBC_MMU_Memory.SpritePaletteHL,
                                GBC_MMU_Memory.SpritePaletteIndex, GBC_MMU_Memory.SpritePaletteColorIndex);
                        }
                    }
                    break;
                case 0xFF6C:
                case 0xFF6D:
                case 0xFF6E:
                case 0xFF6F:
                case 0xFF70:
                case 0xFF71:
                case 0xFF72:
                case 0xFF73:
                case 0xFF74:
                case 0xFF75:
                case 0xFF76:
                case 0xFF77:
                case 0xFF78:
                case 0xFF79:
                case 0xFF7A:
                case 0xFF7B:
                case 0xFF7C:
                case 0xFF7D:
                case 0xFF7E:
                case 0xFF7F:
                    GBC_MMU_Memory.IO[address - 0xFF00] = value;
                    break;
                // High RAM
                case 0xFF80: case 0xFF81: case 0xFF82: case 0xFF83: case 0xFF84: case 0xFF85: case 0xFF86: case 0xFF87:
                case 0xFF88: case 0xFF89: case 0xFF8A: case 0xFF8B: case 0xFF8C: case 0xFF8D: case 0xFF8E: case 0xFF8F:
                case 0xFF90: case 0xFF91: case 0xFF92: case 0xFF93: case 0xFF94: case 0xFF95: case 0xFF96: case 0xFF97:
                case 0xFF98: case 0xFF99: case 0xFF9A: case 0xFF9B: case 0xFF9C: case 0xFF9D: case 0xFF9E: case 0xFF9F:
                case 0xFFA0: case 0xFFA1: case 0xFFA2: case 0xFFA3: case 0xFFA4: case 0xFFA5: case 0xFFA6: case 0xFFA7:
                case 0xFFA8: case 0xFFA9: case 0xFFAA: case 0xFFAB: case 0xFFAC: case 0xFFAD: case 0xFFAE: case 0xFFAF:
                case 0xFFB0: case 0xFFB1: case 0xFFB2: case 0xFFB3: case 0xFFB4: case 0xFFB5: case 0xFFB6: case 0xFFB7:
                case 0xFFB8: case 0xFFB9: case 0xFFBA: case 0xFFBB: case 0xFFBC: case 0xFFBD: case 0xFFBE: case 0xFFBF:
                case 0xFFC0: case 0xFFC1: case 0xFFC2: case 0xFFC3: case 0xFFC4: case 0xFFC5: case 0xFFC6: case 0xFFC7:
                case 0xFFC8: case 0xFFC9: case 0xFFCA: case 0xFFCB: case 0xFFCC: case 0xFFCD: case 0xFFCE: case 0xFFCF:
                case 0xFFD0: case 0xFFD1: case 0xFFD2: case 0xFFD3: case 0xFFD4: case 0xFFD5: case 0xFFD6: case 0xFFD7:
                case 0xFFD8: case 0xFFD9: case 0xFFDA: case 0xFFDB: case 0xFFDC: case 0xFFDD: case 0xFFDE: case 0xFFDF:
                case 0xFFE0: case 0xFFE1: case 0xFFE2: case 0xFFE3: case 0xFFE4: case 0xFFE5: case 0xFFE6: case 0xFFE7:
                case 0xFFE8: case 0xFFE9: case 0xFFEA: case 0xFFEB: case 0xFFEC: case 0xFFED: case 0xFFEE: case 0xFFEF:
                case 0xFFF0: case 0xFFF1: case 0xFFF2: case 0xFFF3: case 0xFFF4: case 0xFFF5: case 0xFFF6: case 0xFFF7:
                case 0xFFF8: case 0xFFF9: case 0xFFFA: case 0xFFFB: case 0xFFFC: case 0xFFFD: case 0xFFFE:
                    GBC_MMU_Memory.HRAM[address - 0xFF80] = value;
                    break;
                // Interrupt Enable Register
                case 0xFFFF:
                    GBC_MMU_Memory.InterruptEnable = value;
                    break;
                default:
                    // Shadow RAM redirection to WRAM Bank X
                    switch (GBC_MMU_Memory.WRAMBankID)
                    {
                        case 0:
                        case 1:
                            GBC_MMU_Memory.WRAMBank1[address - 0xF000] = value;
                            break;
                        case 2:
                            GBC_MMU_Memory.WRAMBank2[address - 0xF000] = value;
                            break;
                        case 3:
                            GBC_MMU_Memory.WRAMBank3[address - 0xF000] = value;
                            break;
                        case 4:
                            GBC_MMU_Memory.WRAMBank4[address - 0xF000] = value;
                            break;
                        case 5:
                            GBC_MMU_Memory.WRAMBank5[address - 0xF000] = value;
                            break;
                        case 6:
                            GBC_MMU_Memory.WRAMBank6[address - 0xF000] = value;
                            break;
                        case 7:
                            GBC_MMU_Memory.WRAMBank7[address - 0xF000] = value;
                            break;
                        default:
                            GBC_MMU_Memory.WRAMBank1[address - 0xF000] = value;
                            break;
                    }
                    break;
            }
            break;
        }
    }
}

void GBC_MMU_WriteShort(uint16_t address, uint16_t value)
{
    GBC_MMU_WriteByte(address, value & 0xFF);
    GBC_MMU_WriteByte(address + 1, (value & 0xFF00) >> 8);
}

GBC_MMU_MemoryBankController_t GBC_MMU_GetMemoryBankController(void)
{
    switch (GBC_MMU_Memory.CartridgeBank0.CartridgeType)
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
