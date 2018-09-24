#include <string.h>

#include "cmod.h"
#include "cmod_init.h"
#include "cmod_access.h"

#include "os.h"
#include "ff.h"
#include "led.h"
#include "sdc.h"


uint8_t CMOD_ROMBankX[16384];



// Check for a Cartridge by trying to read the first byte of the Nintendo Logo
bool CMOD_CheckForCartridge(void)
{
    uint8_t data = 0x00;

    if (!CMOD_Initialized)
    {
        CMOD_Initialize();
    }

    CMOD_ReadByte(0x0104, &data);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    if (data == 0xCE)
    {
        return true;
    }

    return false;
}


/******************************************************************************/
/*                            CMOD save Functions                             */
/******************************************************************************/

// Convert the 11 title bytes to a null terminated string
void CMOD_GetFileName(char* name)
{
    if (CMOD_GetStatus() == CMOD_TURNED_OFF) return;


    // Read the title.
    CMOD_ReadBytes(0x0134, 11, (uint8_t*)name);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    // Convert underscores to spaces for the sake of consistency.
    int i;
    for (i = 0; i < 11 && name[i] != 0x00; i++)
    {
        if (name[i] == 0x5F)
        {
            name[i] = ' ';
            continue;
        }
    }
    name[i++] = '.';

    uint8_t gameType;
    CMOD_ReadByte(0x0143, &gameType);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    if (gameType != GBC_MMU_CGB_FLAG_SUPPORTED && gameType != GBC_MMU_CGB_FLAG_ONLY)
    {
        name[i++] = 'g';
        name[i++] = 'b';
    }
    else
    {
        name[i++] = 'g';
        name[i++] = 'b';
        name[i++] = 'c';  
    }
    name[i] = '\0';
}

// Manually switch the currently active Memory Bank of the Cartridge (to be able to save it).
bool CMOD_SwitchMB(GBC_MMU_MemoryBankController_t mbc, uint16_t bank)
{
    if (CMOD_GetStatus() == CMOD_TURNED_OFF) return false;


    if (mbc != GBC_MMU_MBC_NONE) // Otherwise no switching required.
    {
        if (mbc == GBC_MMU_MBC1)
        {
            uint8_t lower5Bit = bank & 0x1F;
            uint8_t upper2Bit = (bank & 0x60) >> 5; 

            CMOD_WriteByte(0x2100, &lower5Bit);      // Write lower 5 bit of rom bank number into 2000-3FFF
            CMOD_WriteByte(0x6100, 0x00);            // Set 4000-5FFF to Rom mode
            CMOD_WriteByte(0x4100, &upper2Bit);      // Write bit 6 & 7 of rom bank number into 4000-5FFF
            while (CMOD_GetStatus() == CMOD_PROCESSING);
        }
        else if (mbc == GBC_MMU_MBC2 || mbc == GBC_MMU_MBC3)
        {
            // Convert bank to a 8bit number (16bit are needed as parameter for MBC5) and switch Bank by writing into 2000-3FFF (for MBC2 lsb of upper byte must be 1).
            uint8_t bankNumber8Bit = bank;

            CMOD_WriteByte(0x2100, &bankNumber8Bit);
            while (CMOD_GetStatus() == CMOD_PROCESSING);
        }
        else if (mbc == GBC_MMU_MBC5)
        {
            uint8_t lower8Bit = bank;
            uint8_t upperBit  = (bank & 0x100) >> 8; 

            CMOD_WriteByte(0x2100, &lower8Bit);      // Write lower 8 bit of bank number into 2000-2FFF
            CMOD_WriteByte(0x3100, &upperBit);       // Write upper 1 bit of bank number into 3000-3FFF
            while (CMOD_GetStatus() == CMOD_PROCESSING);
        }
        else
        {
            return false;
        }
    }

    return true;
}

CMOD_SaveResult_t CMOD_SaveCartridge(bool overrideExisting)
{
    if (CMOD_GetStatus() == CMOD_TURNED_OFF)       return CMOD_DISABLED;
    if (!SDC_Mount() || !CMOD_CheckForCartridge()) return CMOD_NOCARD;


    if (!CMOD_Initialized)
    {
        CMOD_Initialize();
    }



    FIL      file;
    BYTE     mode = FA_WRITE;

    char     name[16];
    uint8_t  type;
    uint8_t  romSize;
    uint8_t  romBanks;
    uint32_t bytesWritten = 0;


    // Get Cartridge type and determine MBC type, if it's mbc1 make sure Rom Mode is selected.
    CMOD_ReadByte(0x0147, &type);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    GBC_MMU_MemoryBankController_t mbc = GBC_MMU_GetMemoryBankController(type);
    if (mbc == GBC_MMU_MBC1)
    {
        uint8_t romMode = 0x00;
        CMOD_WriteByte(0x6001, &romMode);
    }

    // Get Cartridges rom size and calculate number of banks from it.
    CMOD_ReadByte(0x0148, &romSize);
    while (CMOD_GetStatus() == CMOD_PROCESSING);

    romBanks = romSize == 0 ? 2 : (0x02 << (romSize & 0x0F));    // Number of ROM banks equals 2^(ROMSize+1).
    if (romSize == 0x52 || romSize == 0x53 || romSize == 0x54)   // ROMSize of 0x52, 0x53 or 0x54 equals 72,80 and 96 => 2^(2/3/4 + 1) +64 banks.
    {
       romBanks += 64;
    }

    CMOD_GetFileName(name);
    
    // Specify behaviour if file already exists.
    if (overrideExisting)
    {
        mode |= FA_CREATE_ALWAYS;
    }
    else
    {
        mode |= FA_CREATE_NEW;
    }

    // Create/open a file as specified by mode.
    char path[OS_MAX_PATH_LENGTH];
    CopyString(path, OS_GAME_PATH, OS_MAX_PATH_LENGTH);
    AppendString(path, name, OS_MAX_PATH_LENGTH);

    volatile FRESULT openResult = f_open(&file, path, mode);
    if (openResult == FR_OK)
    {
        // Write Bank 0, if it fails close and delete the file.
        CMOD_ReadBytes(0x0000, 16384, CMOD_ROMBankX);
        while (CMOD_GetStatus() == CMOD_PROCESSING);

        if (f_write(&file, CMOD_ROMBankX, 16384, &bytesWritten) != FR_OK || bytesWritten != 16384)
        {
            f_close(&file);
            f_unlink(name);

            CMOD_TurnOFF();
            return CMOD_FAILED;
        }

        // Write other Banks
        for (int x = 1; x < romBanks; x++)
        {
            // If mbc 1 is used in the cartridge banks 0x20, 0x40 & 0x60 don't exist -> write 0s instead
            if (mbc == GBC_MMU_MBC1 && (x == 0x20 || x == 0x40 || x == 0x60))
            {
                memset(CMOD_ROMBankX, 0, 16384);
            }
            // Otherwise switch the memory bank accordingly and read it
            else
            {
                CMOD_SwitchMB(mbc, x);

                CMOD_ReadBytes(0x4000, 16384, CMOD_ROMBankX);
                while (CMOD_GetStatus() == CMOD_PROCESSING);
            }

            // Write Bank x to the end of the file, if failed close and delete (if something has been written) the file
            if (f_lseek(&file, x * 16384) != FR_OK || f_write(&file, CMOD_ROMBankX, 16384, &bytesWritten) != FR_OK || bytesWritten != 16384)
            {
                f_close(&file);
                f_unlink(name);

                return CMOD_FAILED;
            }
        }

        f_close(&file);
        return CMOD_SUCCESS;
    }
    else if (openResult == FR_EXIST)
    {
        return CMOD_EXISTS;
    }

    return CMOD_FAILED;
}
