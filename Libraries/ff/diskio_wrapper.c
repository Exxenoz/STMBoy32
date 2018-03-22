#include "diskio.h"
#include "diskio_wrapper.h"
#include "stm32h7xx_hal_sd.h"

#define SD_DEFAULT_BLOCK_SIZE 512

#if defined(SDMMC_DATATIMEOUT)
#define SD_TIMEOUT SDMMC_DATATIMEOUT
#elif defined(SD_DATATIMEOUT)
#define SD_TIMEOUT SD_DATATIMEOUT
#else
#define SD_TIMEOUT 30 * 1000
#endif

SD_HandleTypeDef SD_HandleObject;

/**
  * @brief  This function handles SD interrupt request.
  * @param  None
  * @retval None
  */
void SDMMC1_IRQHandler(void)
{
    HAL_SD_IRQHandler(&SD_HandleObject);
}

BYTE disk_status_wrapper (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    HAL_SD_CardStatusTypedef cardStatus;
    if (HAL_SD_GetCardStatus(&SD_HandleObject, &cardStatus) == HAL_OK)
    {
        return RES_OK;
    }

    return RES_ERROR;
}

BYTE disk_initialize_wrapper (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    /* uSD device interface configuration */
    SD_HandleObject.Instance = SDMMC1;

    SD_HandleObject.Init.ClockDiv            = 0; // Unused
    SD_HandleObject.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    SD_HandleObject.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    SD_HandleObject.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    SD_HandleObject.Init.BusWide             = SDMMC_BUS_WIDE_4B;

    /*if(SD_IsDetected() != SD_PRESENT)
    {
        return SD_NOT_PRESENT;
    }*/

    /* Msp SD initialization */
    GPIO_InitTypeDef gpio_init_structure;

    /* Enable SDIO clock */
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

    /* D0(PC8), D1(PC9), D2(PC10), D3(PC11), CK(PC12), CMD(PD2) */
    /* Common GPIO configuration */
    gpio_init_structure.Alternate = GPIO_AF12_SDIO1;

    /* GPIOC configuration */
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);

    /* GPIOD configuration */
    gpio_init_structure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);

    __HAL_RCC_SDMMC1_FORCE_RESET();
    __HAL_RCC_SDMMC1_RELEASE_RESET();

    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

    /* HAL SD initialization */
    if(HAL_SD_Init(&SD_HandleObject) != HAL_OK)
    {
        return RES_ERROR;
    }

    return RES_OK;
}

BYTE disk_read_wrapper (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    if (HAL_SD_ReadBlocks(&SD_HandleObject, buff, (uint32_t)sector, count, SD_TIMEOUT) == HAL_OK)
    {
        /* wait until the read operation is finished */
        while (HAL_SD_GetCardState(&SD_HandleObject) == HAL_SD_CARD_TRANSFER)
        {
        }

        return RES_OK;
    }

    return RES_ERROR;
}

BYTE disk_write_wrapper (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    if (HAL_SD_WriteBlocks(&SD_HandleObject, (uint8_t*)buff, (uint32_t)sector, count, SD_TIMEOUT) == HAL_OK)
    {
        /* wait until the write operation is finished */
        while (HAL_SD_GetCardState(&SD_HandleObject) == HAL_SD_CARD_TRANSFER)
        {
        }

        return RES_OK;
    }

    return RES_ERROR;
}

BYTE disk_ioctl_wrapper (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    switch (cmd)
    {
        // Make sure that no pending write process
        case CTRL_SYNC:
        {
            res = RES_OK;
            break;
        }

        // Get number of sectors on the disk (DWORD)
        case GET_SECTOR_COUNT:
        {
            HAL_SD_CardInfoTypeDef cardInfo;
            HAL_SD_GetCardInfo(&SD_HandleObject, &cardInfo);
            *(DWORD *)buff = cardInfo.LogBlockNbr;
            res = RES_OK;
            break;
        }

        // Get R/W sector size (WORD)
        case GET_SECTOR_SIZE:
        {
            HAL_SD_CardInfoTypeDef cardInfo;
            HAL_SD_GetCardInfo(&SD_HandleObject, &cardInfo);
            *(WORD *)buff = cardInfo.LogBlockSize;
            res = RES_OK;
            break;
        }
        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
        {
            HAL_SD_CardInfoTypeDef cardInfo;
            HAL_SD_GetCardInfo(&SD_HandleObject, &cardInfo);
            *(DWORD*)buff = cardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
            res = RES_OK;
            break;
        }
        default:
        {
            res = RES_PARERR;
        }
    }

    return res;
}
