#ifndef SDC_H
#define SDC_H

#include "common.h"
#include "sdc_config.h"

void SDC_Initialize(void);

void SDC_GPIOConfig(void);
void SDC_SDIOConfig(void);
void SDC_InterruptConfig(void);
void SDC_DMAConfig(void);

#endif //SDC_H
