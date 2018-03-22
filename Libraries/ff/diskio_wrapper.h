#ifndef _DISKIO_WRAPPER_DEFINED
#define _DISKIO_WRAPPER_DEFINED

#include "integer.h"

void SDMMC1_IRQHandler(void);

BYTE disk_status_wrapper (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
);

BYTE disk_initialize_wrapper (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
);

BYTE disk_read_wrapper (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
);

BYTE disk_write_wrapper (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
);
    
BYTE disk_ioctl_wrapper (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
);

#endif
