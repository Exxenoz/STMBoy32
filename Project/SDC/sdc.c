#include "sdc.h"
#include "led.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"


static SPI_HandleTypeDef SDC_SPIHandle;

FATFS SDC_FatFS;                 // Global FatFS object.
char  SDPath[4];                 // SD card logical drive path.
bool  SDC_Mounted = false;       // Global mounted state.



void SDC_Initialize(void)
{
    //BSP_SD_Init();
}

bool SDC_Mount(void)
{
    if (SDC_Mounted)
    {
        SDC_Unmount();
    }

    if(FATFS_LinkDriver(&SD_Driver, SDPath) != FR_OK)
    {
        return false;
    }

    if (f_mount(&SDC_FatFS, (TCHAR const*)SDPath, 1) != FR_OK)
    {
        return false;
    }

    return SDC_Mounted = true;
}

bool SDC_IsMounted(void)
{
    return SDC_Mounted;
}

void SDC_Unmount(void)
{
    f_mount(NULL, (TCHAR const*)SDPath, 1);
    FATFS_UnLinkDriver(SDPath);

    SDC_Mounted = false;
}

/**
  * @brief  Initializes SPI MSP.
  * @retval None
  */
void SPIx_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef          GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef  SPI_PeriphClkInit;

    /* Configure SPI SCK */
    GPIO_InitStruct.Pin       = SDC_SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = SDC_SPIx_SCK_AF;
    HAL_GPIO_Init(SDC_SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* Configure SPI MISO and MOSI */
    GPIO_InitStruct.Pin       = SDC_SPIx_MOSI_PIN;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = SDC_SPIx_MISO_MOSI_AF;
    HAL_GPIO_Init(SDC_SPIx_MISO_MOSI_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = SDC_SPIx_MISO_PIN;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(SDC_SPIx_MISO_MOSI_GPIO_PORT, &GPIO_InitStruct);


    SPI_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI123;
    SPI_PeriphClkInit.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    HAL_RCCEx_PeriphCLKConfig(&SPI_PeriphClkInit);
}

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
void SPIx_Init(void)
{
  if(HAL_SPI_GetState(&SDC_SPIHandle) == HAL_SPI_STATE_RESET)
  {
    // SPI baudrate is set to 12,5 MHz maximum (PLL1_Q/SPI_BaudRatePrescaler = 200/16 = 12,5 MHz) to verify these constraints:
    // - SD card SPI interface max baudrate is 25MHz for write/read.

    SDC_SPIHandle.Instance                        = SDC_SPIx;
    SDC_SPIHandle.Init.BaudRatePrescaler          = SPI_BAUDRATEPRESCALER_16;
    SDC_SPIHandle.Init.Direction                  = SPI_DIRECTION_2LINES;
    SDC_SPIHandle.Init.CLKPhase                   = SPI_PHASE_2EDGE;
    SDC_SPIHandle.Init.CLKPolarity                = SPI_POLARITY_HIGH;
    SDC_SPIHandle.Init.CRCCalculation             = SPI_CRCCALCULATION_DISABLE;
    SDC_SPIHandle.Init.CRCPolynomial              = 7;
    SDC_SPIHandle.Init.DataSize                   = SPI_DATASIZE_8BIT;
    SDC_SPIHandle.Init.FirstBit                   = SPI_FIRSTBIT_MSB;
    SDC_SPIHandle.Init.NSS                        = SPI_NSS_SOFT;
    SDC_SPIHandle.Init.TIMode                     = SPI_TIMODE_DISABLE;
    SDC_SPIHandle.Init.Mode                       = SPI_MODE_MASTER;
    SDC_SPIHandle.Init.MasterKeepIOState          = SPI_MASTER_KEEP_IO_STATE_ENABLE;  /* Recommanded setting to avoid glitches */
    SDC_SPIHandle.Init.FifoThreshold              = SPI_FIFO_THRESHOLD_01DATA;
    SDC_SPIHandle.Init.CRCLength                  = SPI_CRC_LENGTH_8BIT;
    SDC_SPIHandle.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    SDC_SPIHandle.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    SDC_SPIHandle.Init.NSSPolarity                = SPI_NSS_POLARITY_LOW;
    SDC_SPIHandle.Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
    SDC_SPIHandle.Init.MasterSSIdleness           = 0x00000000;
    SDC_SPIHandle.Init.MasterInterDataIdleness    = 0x00000000;
    SDC_SPIHandle.Init.MasterReceiverAutoSusp     = 0x00000000;

    SPIx_MspInit(&SDC_SPIHandle);
    HAL_SPI_Init(&SDC_SPIHandle);
  }
}

/**
  * @brief  SPI Write and Read data
  * @param  DataIn: Data to write
  * @param  DataOut: Data to read
  * @param  DataLength: Data size
  * @retval None
  */
void SPIx_WriteReadData(const uint8_t *DataIn, uint8_t *DataOut, uint16_t DataLength)
{
    /* Check the communication status */
    if(HAL_SPI_TransmitReceive(&SDC_SPIHandle, (uint8_t*) DataIn, DataOut, DataLength, SDC_SPIx_TIMEOUT) != HAL_OK)
    {
        SPIx_Error();
    }
}

/**
  * @brief  SPI error treatment function
  * @retval None
  */
void SPIx_Error (void)
{
    /* De- and Re-Initialize the SPI communication BUS */
    HAL_SPI_DeInit(&SDC_SPIHandle);
    SPIx_Init();
}


/******************************************************************************/
/*                                   LINK SD                                  */
/******************************************************************************/

/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for
  *         data transfer).
  * @retval None
  */
void SD_IO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    uint8_t           counter;


    /* Configure SD_CS_PIN pin: SD Card CS pin */
    GPIO_InitStruct.Pin   = SD_CS_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SD_CS_GPIO_PORT, &GPIO_InitStruct);

    
    /*------------Put SD in SPI mode--------------*/
    /* SD SPI Config */
    SPIx_Init();
    SD_CS_HIGH();

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles       */
    for (counter = 0; counter <= 9; counter++)
    {
        SD_IO_WriteByte(SD_DUMMY_BYTE);
    }
}

/**
  * @brief  Set the SD_CS pin.
  * @param  val: SD CS value.
  * @retval None
  */
void SD_IO_CSState(uint8_t val)
{
    if(val == 1)
    {
        SD_CS_HIGH();
    }
    else
    {
        SD_CS_LOW();
    }
}

/**
  * @brief  SD Write and Read data
  * @param  DataIn: Data to write
  * @param  DataOut: Data to read
  * @param  DataLength: Data size
  * @retval None
  */
void SD_IO_WriteReadData(const uint8_t *DataIn, uint8_t *DataOut, uint16_t DataLength)
{
    SPIx_WriteReadData(DataIn, DataOut, DataLength);
}

/**
  * @brief  Writes a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
uint8_t SD_IO_WriteByte(uint8_t Data)
{
    uint8_t tmp;

    SPIx_WriteReadData(&Data, &tmp, 1);
    
    return tmp;
}
