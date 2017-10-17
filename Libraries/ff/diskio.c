/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "stm324x9i_eval_sdio_sd.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv)
    {
        case DEV_RAM :
        {
            if (SD_Detect() != SD_PRESENT)
            {
                return RES_NOTRDY;
            }

            // ToDo: Check for write protection and send RES_WRPRT if needed

            return RES_OK;
        }
        case DEV_MMC :
            return RES_NOTRDY;

        case DEV_USB :
            return RES_NOTRDY;
	}

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv)
    {
        case DEV_RAM :
        {
            SD_Error status = SD_Init();

            if (status == SD_OK)
            {
                return RES_OK;
            }

            return RES_ERROR;
        }
        case DEV_MMC :
            return RES_NOTRDY;

        case DEV_USB :
            return RES_NOTRDY;
	}

	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv)
    {
        case DEV_RAM :
        {
            SD_Error status = SD_ReadMultiBlocks(buff, sector << 9, 512, count);

            if (status == SD_OK)
            {
                SDTransferState state;

                status = SD_WaitReadOperation();

                while ((state = SD_GetStatus()) == SD_TRANSFER_BUSY);

                if ((state == SD_TRANSFER_ERROR) || (status != SD_OK))
                {
                    return RES_ERROR;
                }
                else
                {
                    return RES_OK;
                }			
            }

            return RES_ERROR;
        }
        case DEV_MMC :
            return RES_NOTRDY;

        case DEV_USB :
            return RES_NOTRDY;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv)
    {
        case DEV_RAM :
        {
            SD_Error result = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9, 512, count); // 4GB Compliant

            if (result == SD_OK)
            {
                SDTransferState state;

                result = SD_WaitWriteOperation(); // Check if the Transfer is finished

                while ((state = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

                if ((state == SD_TRANSFER_ERROR) || (result != SD_OK))
                {
                    return RES_ERROR;
                }
                else
                {
                    return RES_OK;
                }
            }

            return RES_ERROR;
        }
        case DEV_MMC :
            return RES_NOTRDY;

        case DEV_USB :
            return RES_NOTRDY;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv)
    {
        case DEV_RAM :
            switch (cmd)
            {
                case GET_SECTOR_SIZE :     // Get R/W sector size (WORD) 
                    *(WORD *) buff = 512;
                    break;
                case GET_BLOCK_SIZE :      // Get erase block size in unit of sector (DWORD)
                    *(DWORD *) buff = 32;
                    break;
                case CTRL_SYNC :
                    break;
            }

            return RES_OK;

        case DEV_MMC :
            return RES_NOTRDY;

        case DEV_USB :
            return RES_NOTRDY;
	}

	return RES_PARERR;
}

