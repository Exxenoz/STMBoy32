#include "sdram.h"

#define WRITE_READ_ADDR     ((uint32_t)0x0800)

SDRAM_HandleTypeDef      hsdram;
FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef  GPIO_Init_Structure;

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* Common GPIO configuration */
    GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull      = GPIO_PULLUP;
    GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

    /* GPIOD configuration */
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_14 | GPIO_PIN_15;


    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* GPIOE configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
      
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOF configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;

    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    /* GPIOG configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |\
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
    /* Configure FMC as alternate function  */
    /*HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3| GPIO_PIN_4 | GPIO_PIN_5     |\
                         GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
                         GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3| GPIO_PIN_4 | GPIO_PIN_7     |\
                         GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
                         GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4     |\
                         GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                         GPIO_PIN_5 | GPIO_PIN_10);*/
}

static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd =0;
    /* Step 1:  Configure a clock configuration enable command */
    Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command->CommandMode = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 4 : Configure a Auto-Refresh command */
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber = 8;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT); 
}

static void SDRAM_FillBuffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
    {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }
}

void SDRAM_Initialize(void)
{
    /*##-1- Configure the SDRAM device #########################################*/
    /* SDRAM device configuration */
    hsdram.Instance = FMC_SDRAM_DEVICE;

    /* Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) */
    SDRAM_Timing.LoadToActiveDelay    = 2;
    SDRAM_Timing.ExitSelfRefreshDelay = 7;
    SDRAM_Timing.SelfRefreshTime      = 4;
    SDRAM_Timing.RowCycleDelay        = 7;
    SDRAM_Timing.WriteRecoveryTime    = 2;
    SDRAM_Timing.RPDelay              = 2;
    SDRAM_Timing.RCDDelay             = 2;

    hsdram.Init.SDBank             = FMC_SDRAM_BANK2;
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
    hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

    /* Initialize the SDRAM controller */
    if(HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
    {
        /* Initialization Error */
        while (1) {}
    }

    /* Program the SDRAM external device */
    SDRAM_Initialization_Sequence(&hsdram, &command);

    /*##-2- SDRAM memory read/write access #####################################*/

    enum { BUFFER_SIZE = 1024 };

    uint32_t aTxBuffer[BUFFER_SIZE];
    uint32_t aRxBuffer[BUFFER_SIZE];

    /* Fill the buffer to write */
    SDRAM_FillBuffer(aTxBuffer, BUFFER_SIZE, 0xA244250F);

    /* Fill the Read buffer */
    SDRAM_FillBuffer(aRxBuffer, BUFFER_SIZE, 0xBBBBBBBB);

    /* Counter index */
    uint32_t uwIndex = 0;

    /* Write data to the SDRAM memory */
    for (uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
    {
        *(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex) = aTxBuffer[uwIndex];
    }

    /* Read back data from the SDRAM memory */
    for (uwIndex = 0; uwIndex < BUFFER_SIZE; uwIndex++)
    {
        aRxBuffer[uwIndex] = *(__IO uint32_t*) (SDRAM_BANK_ADDR + WRITE_READ_ADDR + 4*uwIndex);
    }

    /*##-3- Checking data integrity ############################################*/

    typedef enum { PASSED = 0, FAILED = !PASSED } TestStatus_t;

    /* Status variables */
    __IO TestStatus_t uwWriteReadStatus = PASSED;

    for (uwIndex = 0; (uwIndex < BUFFER_SIZE) && (uwWriteReadStatus == PASSED); uwIndex++)
    {
        if (aRxBuffer[uwIndex] != aTxBuffer[uwIndex])
        {
            uwWriteReadStatus = FAILED;
        }
    }

    if (uwWriteReadStatus != PASSED)
    {
        /* KO */
        while(1) {}
    }
}
