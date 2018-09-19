#ifndef SDC_H
#define SDC_H

#include "common.h"
#include "sdc_config.h"
#include "ff.h"


#define SD_DUMMY_BYTE            0xFF
#define SD_NO_RESPONSE_EXPECTED  0x80



void SDC_Initialize(void);
bool SDC_Mount(void);
void SDC_Unmount(void);
bool SDC_IsMounted(void);

void SPIx_Init(void);
void SPIx_MspInit(SPI_HandleTypeDef *hspi);
void SPIx_WriteReadData(const uint8_t *DataIn, uint8_t *DataOut, uint16_t DataLength);
void SPIx_Error(void);

void    SD_IO_Init(void);
void    SD_IO_CSState(uint8_t state);
void    SD_IO_WriteReadData(const uint8_t *DataIn, uint8_t *DataOut, uint16_t DataLength);
uint8_t SD_IO_WriteByte(uint8_t Data);

#endif //SDC_H
