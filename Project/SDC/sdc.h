#ifndef SDC_H
#define SDC_H

#include "common.h"
#include "sdc_config.h"
#include "ff.h"

void SDC_Initialize(void);
bool SDC_Mount(void);
bool SDC_IsMounted(void);
void SDC_Unmount(void);

void    SD_IO_Init(void);
void    SD_IO_CSState(uint8_t state);
void    SD_IO_WriteReadData(const uint8_t *DataIn, uint8_t *DataOut, uint16_t DataLength);
uint8_t SD_IO_WriteByte(uint8_t Data);

#endif //SDC_H
