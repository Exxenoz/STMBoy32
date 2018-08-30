#include "sdram.h"


SDRAM_HandleTypeDef      hsdram;
FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
FMC_SDRAM_CommandTypeDef SDRAM_Command;



void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef  GPIO_InitObject;

    #define INITIALIZE_SDRAM_PINS(PORT, PINS)          \
    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;        \
    GPIO_InitObject.Pin       = PINS;                    \
    GPIO_InitObject.Pull      = GPIO_PULLUP;              \
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH; \
    GPIO_InitObject.Alternate = GPIO_AF12_FMC;              \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);                   \

    INITIALIZE_SDRAM_PINS(GPIOC, SDRAM_GPIOC_PINS);
    INITIALIZE_SDRAM_PINS(GPIOD, SDRAM_GPIOD_PINS);
    INITIALIZE_SDRAM_PINS(GPIOE, SDRAM_GPIOE_PINS);
    INITIALIZE_SDRAM_PINS(GPIOF, SDRAM_GPIOF_PINS);
    INITIALIZE_SDRAM_PINS(GPIOG, SDRAM_GPIOG_PINS);
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
    HAL_GPIO_DeInit(GPIOD, SDRAM_GPIOD_PINS);
    HAL_GPIO_DeInit(GPIOE, SDRAM_GPIOE_PINS);
    HAL_GPIO_DeInit(GPIOF, SDRAM_GPIOF_PINS);
    HAL_GPIO_DeInit(GPIOG, SDRAM_GPIOG_PINS);
}

static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd = 0;

    // Configure a clock configuration enable SDRAM_Command and send it.
    Command->CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);
    HAL_Delay(1);                                                       // Delay for at least 100 us.

    // Configure a PALL (precharge all) SDRAM_Command and send it.
    Command->CommandMode            = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    // Configure a Auto-Refresh SDRAM_Command and send it.
    Command->CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber      = 8;
    Command->ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    // Program the external memory mode register.
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                       SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                       SDRAM_MODEREG_CAS_LATENCY_3           |
                       SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                       SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = tmpmrd;
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    // Set the device refresh rate.
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT); 
}

static void SDRAM_FillBuffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
    uint32_t tmpIndex = 0;

    for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
    {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }
}

void SDRAM_Initialize(void)
{
    enum { BUFFER_SIZE = 1024 };


    hsdram.Instance = FMC_SDRAM_DEVICE;

    hsdram.Init.SDBank                = FMC_SDRAM_BANK2;
    hsdram.Init.ColumnBitsNumber      = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hsdram.Init.RowBitsNumber         = FMC_SDRAM_ROW_BITS_NUM_13;
    hsdram.Init.MemoryDataWidth       = SDRAM_MEMORY_WIDTH;
    hsdram.Init.InternalBankNumber    = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram.Init.CASLatency            = FMC_SDRAM_CAS_LATENCY_3;
    hsdram.Init.WriteProtection       = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram.Init.SDClockPeriod         = SDCLOCK_PERIOD;
    hsdram.Init.ReadBurst             = FMC_SDRAM_RBURST_ENABLE;
    hsdram.Init.ReadPipeDelay         = FMC_SDRAM_RPIPE_DELAY_0;

    // Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz).
    SDRAM_Timing.LoadToActiveDelay    = 2;
    SDRAM_Timing.ExitSelfRefreshDelay = 7;
    SDRAM_Timing.SelfRefreshTime      = 4;
    SDRAM_Timing.RowCycleDelay        = 7;
    SDRAM_Timing.WriteRecoveryTime    = 2;
    SDRAM_Timing.RPDelay              = 2;
    SDRAM_Timing.RCDDelay             = 2;

    // Initialize the SDRAM controller (Deadlock upon failure).
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK) while(1){}

    // Program the SDRAM external device.
    SDRAM_Initialization_Sequence(&hsdram, &SDRAM_Command);


    // Check SDRAM memory read/write access.
    uint32_t aTxBuffer[BUFFER_SIZE];
    uint32_t aRxBuffer[BUFFER_SIZE];
    SDRAM_FillBuffer(aTxBuffer, BUFFER_SIZE, 0xA244250F);
    SDRAM_FillBuffer(aRxBuffer, BUFFER_SIZE, 0xBBBBBBBB);

    // Write data to the SDRAM memory.
    for (uint32_t uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
    {
        SDRAM_WRITE_LONG(4 * uwIndex, aTxBuffer[uwIndex]);
    }

    // Read back data from the SDRAM memory.
    for (uint32_t uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
    {
        SDRAM_READ_LONG(4 * uwIndex, aRxBuffer[uwIndex]);
    }

    // Check data (Deadlock upon failure).
    for (uint32_t uwIndex = 0; (uwIndex < BUFFER_SIZE); uwIndex++)
    {
        if (aRxBuffer[uwIndex] != aTxBuffer[uwIndex])
        {
            while(1){}
        }
    }
}
