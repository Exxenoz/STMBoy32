#ifndef SDC_H
#define SDC_H

#include "common.h"
#include "sdc_config.h"

void SDC_Initialize(void);

// Interrupt Handler
void SDIO_IRQHandler(void);
void SD_SDIO_DMA_IRQHANDLER(void);

#endif //SDC_H
