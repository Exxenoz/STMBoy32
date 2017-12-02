#ifndef SDC_H
#define SDC_H

#include "common.h"
#include "sdc_config.h"
#include "ff.h"


void SDC_Initialize(void);
bool SDC_Mount(void);
bool SDC_IsMounted(void);
void SDC_Unmount(void);

// Interrupt Handler
void SDIO_IRQHandler(void);
void SD_SDIO_DMA_IRQHANDLER(void);

#endif //SDC_H
