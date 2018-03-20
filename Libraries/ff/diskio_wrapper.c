#include "diskio.h"
#include "diskio_wrapper.h"
#include "stm32h7xx_hal_sd.h"

SD_HandleTypeDef SD_HandleObject;

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
    if (HAL_SD_Init(&SD_HandleObject) == HAL_OK)
    {
        return RES_OK;
    }

    return RES_ERROR;
}

BYTE disk_read_wrapper (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    if (HAL_SD_ReadBlocks(&SD_HandleObject, buff, sector << 9, count, 1000) == HAL_OK)
    {
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
    if (HAL_SD_WriteBlocks(&SD_HandleObject, (uint8_t*)buff, sector << 9, count, 1000) == HAL_OK)
    {
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
            *(DWORD *)buff = cardInfo.BlockNbr;
            res = RES_OK;
            break;
        }

        // Get R/W sector size (WORD)
        case GET_SECTOR_SIZE:
        {
            HAL_SD_CardInfoTypeDef cardInfo;
            HAL_SD_GetCardInfo(&SD_HandleObject, &cardInfo);
            *(WORD *)buff = cardInfo.BlockSize;
            res = RES_OK;
            break;
        }
        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
        {
            HAL_SD_CardInfoTypeDef cardInfo;
            HAL_SD_GetCardInfo(&SD_HandleObject, &cardInfo);
            *(DWORD*)buff = cardInfo.BlockSize;
            break;
        }
        default:
        {
            res = RES_PARERR;
        }
    }

    return res;
}
