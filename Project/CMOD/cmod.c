#include "cmod.h"
#include "sdc.h"
#include "ff.h"
#include "string.h"


TIM_OC_InitTypeDef CMOD_TIM_OCInitObject;
TIM_HandleTypeDef CMOD_TimerHandle = { 
    .Instance = CMOD_TIM,
    .Channel  = HAL_TIM_ACTIVE_CHANNEL_1
};

uint8_t        CMOD_ROMBankX[16384];

CMOD_Action_t  CMOD_Action       = CMOD_NOACTION;
CMOD_Status_t  CMOD_Status       = CMOD_WAITING;
uint16_t       CMOD_Address      = 0x0000;
uint8_t        *CMOD_DataOut     = NULL;
uint8_t        *CMOD_DataIn      = NULL;
int            CMOD_BytesToRead  = 0;
int            CMOD_BytesRead    = 0;
int            CMOD_BytesToWrite = 0;
int            CMOD_BytesWritten = 0;
bool           CMOD_Initialized  = false;

bool CMOD_CartridgeInserted = false;



CMOD_Status_t CMOD_GetStatus(void)
{
    return CMOD_Status; 
}

void CMOD_EnableInterrupt(void)
{
    CMOD_TIM->DIER |= (uint16_t)TIM_IT_UPDATE;
}

// Check for a Cartridge by trying to read the first byte of the Nintendo Logo
bool CMOD_CheckForCartridge(void)
{
    if (!CMOD_Initialized)
    {
        CMOD_CartridgeInserted = false;
    }
    else
    {
        uint8_t data = 0x00;

        CMOD_ReadByte(0x0104, &data);
        while (CMOD_Status == CMOD_PROCESSING);

        if (data == 0xCE)
        {
            CMOD_CartridgeInserted = true;
        }
        else
        {
            CMOD_CartridgeInserted = false;
        }
    }


    return CMOD_CartridgeInserted;
}

void CMOD_ResetCartridge(void)
{
    CMOD_RST_RESET;                     // Reset Cartridge
    for (int i = 0; i < 100; i++);      // Wait some ns
    CMOD_SET_RESET;
}

void CMOD_ReadByte(uint16_t address, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_RD;                        // RD goes low for a read at 30ns (in GB)

    CMOD_Action      = CMOD_READ;
    CMOD_Address     = address;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = 1;
    CMOD_BytesRead   = 0;
    CMOD_Status      = CMOD_PROCESSING;

    CMOD_DATA_MODE_IN;  
    CMOD_EnableInterrupt();
}

void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_RD;                        // RD goes low for a read at 30ns (in GB)

    CMOD_Action      = CMOD_READ;
    CMOD_Address     = startingAddress;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = bytes;
    CMOD_BytesRead   = 0;
    CMOD_Status      = CMOD_PROCESSING;

    CMOD_DATA_MODE_IN;  
    CMOD_EnableInterrupt();
}

void CMOD_WriteByte(uint16_t address, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_WR;                        // WR goes low for a write at ? (in GB)

    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = address;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = 1;
    CMOD_BytesWritten = 0;
    CMOD_Status       = CMOD_PROCESSING;

    CMOD_DATA_MODE_OUT;  
    CMOD_EnableInterrupt();
}

void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_WR;                        // WR goes low for a write at ? (in GB)

    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = startingAddress;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = bytes;
    CMOD_BytesWritten = 0;
    CMOD_Status       = CMOD_PROCESSING;

    CMOD_DATA_MODE_OUT;
    CMOD_EnableInterrupt();
}

// Manually switch the currently active Memory Bank of the Cartridge (to be able to save it)
bool CMOD_SwitchMB(GBC_MMU_MemoryBankController_t mbc, uint16_t bank)
{
    if (mbc == GBC_MMU_MBC_NONE)
    {
        // No switching Required
        return true;
    }
    else if (mbc == GBC_MMU_MBC1)
    {
        uint8_t lower5Bit = bank & 0x1F;
        uint8_t upper2Bit = (bank & 0x60) >> 5; 

        CMOD_WriteByte(0x2100, &lower5Bit);      // Write lower 5 bit of rom bank number into 2000-3FFF
        CMOD_WriteByte(0x6100, 0x00);            // Set 4000-5FFF to Rom mode
        CMOD_WriteByte(0x4100, &upper2Bit);      // Write bit 6 & 7 of rom bank number into 4000-5FFF
        while (CMOD_Status == CMOD_PROCESSING);
    }
    else if (mbc == GBC_MMU_MBC2 || mbc == GBC_MMU_MBC3)
    {
        // Convert bank to a 8bit number (16bit are needed as parameter for MBC5)
        uint8_t bankNumber8Bit = bank;
        // Switch Bank by writing into 2000-3FFF (for MBC2 lsb of upper byte must be 1)
        CMOD_WriteByte(0x2100, &bankNumber8Bit);
        while (CMOD_Status == CMOD_PROCESSING);
    }
    else if (mbc == GBC_MMU_MBC5)
    {
        uint8_t lower8Bit = bank;
        uint8_t upperBit  = (bank & 0x100) >> 8; 

        CMOD_WriteByte(0x2100, &lower8Bit);      // Write lower 8 bit of bank number into 2000-2FFF
        CMOD_WriteByte(0x3100, &upperBit);       // Write upper 1 bit of bank number into 3000-3FFF
        while (CMOD_Status == CMOD_PROCESSING);
    }
    else
    {
        return false;
    }

    return true;
}

// Convert the 11 title bytes to a null terminated string
void CMOD_GetFileName(char* name)
{
    int i;

    for (i = 0; i < 11 && GBC_MMU_Memory.Title[i] != 0x00; i++)
    {
        if (GBC_MMU_Memory.Title[i] == 0x5F)        // Convert underscores to spaces for the sake of consistency
        {
            name[i] = ' ';
            continue;
        }

        name[i] = (char) GBC_MMU_Memory.Title[i];
    }
    name[i++] = '.';
    
    if (GBC_MMU_Memory.CGBFlag & (GBC_MMU_CGB_FLAG_SUPPORTED | GBC_MMU_CGB_FLAG_ONLY))
    {
        name[i++] = 'g';
        name[i++] = 'b';
        name[i++] = 'c';  
    }
    else
    {
        name[i++] = 'g';
        name[i++] = 'b';
    }
    name[i] = '\0';
}

CMOD_SaveResult_t CMOD_SaveCartridge(bool overrideExisting)
{
    // If no SD Card is detected return nocard
    if (!SDC_Mount() || !CMOD_CheckForCartridge())
    {
        return CMOD_NOCARD;
    }

    // Get MBC, if it's mbc1 make sure Rom Mode is selected
    GBC_MMU_MemoryBankController_t mbc = GBC_MMU_GetMemoryBankController();
    if (mbc == GBC_MMU_MBC1)
    {
        uint8_t romMode = 0x00;
        CMOD_WriteByte(0x6001, &romMode);
    }

    FIL      file;
    BYTE     mode = FA_WRITE;
    char     name[16];
    uint8_t  romBanks;
    uint8_t  romSize      = GBC_MMU_Memory.ROMSize;
    uint32_t bytesWritten = 0;

    // Number of ROM banks equals 2^(ROMSize+1) or 0 for ROMSize = 0
    // ROMSize of 0x52, 0x53 or 0x54 equals 72,80 and 96 => 2^(2/3/4 + 1) +64 banks
    romBanks = romSize == 0 ? 2 : (0x02 << (romSize & 0x0F));
    if ((romSize & 0x50) == 0x50)
    {
       romBanks += 64;
    }

    CMOD_GetFileName(name);
    
    // Specify behaviour if file already exists
    if (overrideExisting)
    {
        mode |= FA_CREATE_ALWAYS;
    }
    else
    {
        mode |= FA_CREATE_NEW;
    }

    // Create/open a file as specified by mode 
    FRESULT openResult = f_open(&file, name, mode);
    if (openResult == FR_OK)
    {
        // Write Bank 0, if failed close and delete (if something has been written) the file
        if (f_write(&file, GBC_MMU_Memory.CartridgeBank0, 16384, &bytesWritten) != FR_OK || bytesWritten != 16384)
        {
            f_close(&file);
            f_unlink(name);
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
                while (CMOD_Status == CMOD_PROCESSING);
            }

            // Write Bank x to the end of the file, if failed close and delete (if something has been written) the file
            if (f_lseek(&file, x * 16384) != FR_OK ||
                f_write(&file, CMOD_ROMBankX, 16384, &bytesWritten) != FR_OK || bytesWritten != 16384)
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

void CMOD_Initialize_CLK(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = CMOD_CLK_PIN;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(CMOD_CLK_PORT, &GPIO_InitObject);

    CMOD_TimerHandle.Init.Prescaler         = 0;                      // Tim5 runs with 100Mhz(?) -> keep this rate
    CMOD_TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    CMOD_TimerHandle.Init.Period            = 95;                     // Count 'til 96(-1) -> 1,05Mhz PWM
    CMOD_TimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    CMOD_TimerHandle.Init.RepetitionCounter = 0;

    CMOD_TIM_OCInitObject.Pulse        = 47; 
    CMOD_TIM_OCInitObject.OCMode       = TIM_OCMODE_PWM1;
    CMOD_TIM_OCInitObject.OCFastMode   = TIM_OCFAST_DISABLE;
    CMOD_TIM_OCInitObject.OCPolarity   = TIM_OCPOLARITY_LOW;
    CMOD_TIM_OCInitObject.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    CMOD_TIM_OCInitObject.OCIdleState  = TIM_OCIDLESTATE_RESET;
    CMOD_TIM_OCInitObject.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&CMOD_TimerHandle, &CMOD_TIM_OCInitObject, CMOD_TIM_CHANNEL) != HAL_OK)
    {
        //Error_Handler();
    }
    
    HAL_TIM_Base_Init(&CMOD_TimerHandle);
    HAL_TIM_Base_Start(&CMOD_TimerHandle);

    HAL_NVIC_SetPriority(CMOD_TIM_NVIC_CHANNEL, INTERRUPT_PRIORITY_2, INTERRUPT_PRIORITY_2);
    HAL_NVIC_EnableIRQ(CMOD_TIM_NVIC_CHANNEL);
}

void CMOD_Initialize(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)       \
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;    \
    GPIO_InitObject.Pin   = PIN;                     \
    GPIO_InitObject.Pull  = GPIO_NOPULL;              \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(CMOD_RESET_PORT, CMOD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_CS_PORT,    CMOD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_RD_PORT,    CMOD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_WR_PORT,    CMOD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_ADDR_PORT,  CMOD_ADDR_PINS);
    INITIALIZE_OUTPUT_PIN(CMOD_DATA_PORT,  CMOD_DATA_PINS);

    CMOD_Initialize_CLK();

    CMOD_Initialized = true;
}

void CMOD_HandleRead(void)
{
    CMOD_SET_CS;                                            // CS rises at 0ns (after CLK Flank)

    if (CMOD_BytesRead != 0)                                // If a Byte was read, store it
    {                                                       // Gameboy stops driving Data Bus at 0ns -> Data ready
        CMOD_DataIn[CMOD_BytesRead -1] = CMOD_GET_DATA();   // Store the nt byte at CMOD_DataIn[n-1]
    }

    if (CMOD_BytesRead == CMOD_BytesToRead)                 // Read all Data?
    {
        CMOD_SET_RD;                                        // RD rises after a read at 140ns (in GB)

        CMOD_Status     = CMOD_DATA_READY;                  // -> Data Ready
        CMOD_Action     = CMOD_NOACTION;                    // All actions finished
        CMOD_TIM->DIER &= (uint16_t)~TIM_IT_UPDATE;         // Disable Interrupt until needed again
        return;
    }

    CMOD_SET_ADDR(CMOD_Address + CMOD_BytesRead);           // Address changes at 140ns (in GB)
    CMOD_RST_CS;                                            // CS goes low at 240ns (in GB)
    CMOD_BytesRead++;                                       // Increase BytesRead in good faith
}

void CMOD_HandleWrite(void)
{
    CMOD_SET_CS;                                            // CS rises at 0ns (after CLK Flank)

    if (CMOD_BytesWritten == CMOD_BytesToWrite)             // All Bytes written?
    {
        CMOD_SET_WR;                                        // WR rises after a write 20ns before the CLK rises (in GB)

        CMOD_Status     = CMOD_WRITE_COMPLETE;              // -> Write complete
        CMOD_Action     = CMOD_NOACTION;                    // All actions finished
        CMOD_TIM->DIER &= (uint16_t)~TIM_IT_UPDATE;         // Disable Interrupt until needed again
        return;
    }

    CMOD_SET_ADDR(CMOD_Address + CMOD_BytesWritten);        // Address changes at 140ns (in GB)
    CMOD_RST_CS;                                            // CS goes low at 240ns (in GB)
    CMOD_BytesWritten++;                                    // Increase BytesWritten in good faith
    CMOD_SET_DATA(CMOD_DataOut[CMOD_BytesWritten - 1]);     // Data Bus is driven at 480ns (falling CLK) (in GB)
}

void CMOD_HandleNOP(void)
{
    return;
}

void (*CMOD_OperationTable[3])(void) =
{
    CMOD_HandleRead,
    CMOD_HandleWrite,
    CMOD_HandleNOP
};

void TIM5_IRQHandler(void)
{
  if (((CMOD_TIM->SR   & TIM_IT_UPDATE) != (uint16_t)RESET) && 
      ((CMOD_TIM->DIER & TIM_IT_UPDATE) != (uint16_t)RESET))    // ITStatus == SET?
  {
      CMOD_OperationTable[CMOD_Action]();                       // Handle Read / Write / NOP

      CMOD_TIM->SR = (uint16_t)~TIM_IT_UPDATE;                  // ClearITPendingBit
  }
}
