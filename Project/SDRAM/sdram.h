#ifndef SDRAM_H
#define SDRAM_H

#include "common.h"


#define REFRESH_COUNT                            ((uint32_t)0x0603)

#define WRITE_READ_ADDR                          ((uint32_t)0x0800)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_BANK_ADDR                          ((uint32_t)0xC0000000)
#define SDRAM_MEMORY_WIDTH                       FMC_SDRAM_MEM_BUS_WIDTH_16

#define SDCLOCK_PERIOD                           FMC_SDRAM_CLOCK_PERIOD_2

#define SDRAM_GPIOC_PINS                         GPIO_PIN_0 | GPIO_PIN_3
#define SDRAM_GPIOD_PINS                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15
#define SDRAM_GPIOE_PINS                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15
#define SDRAM_GPIOF_PINS                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15
#define SDRAM_GPIOG_PINS                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_4 | GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_15

#define SDRAM_READ_BYTE(ADDR)             ((*(__IO uint8_t*)  (ADDR)))
#define SDRAM_READ_SHORT(ADDR)            ((*(__IO uint16_t*) (ADDR)))
#define SDRAM_READ_LONG(ADDR)             ((*(__IO uint32_t*) (ADDR)))
#define SDRAM_WRITE_BYTE(ADDR, BYTE)      ((*(__IO uint8_t*)  (ADDR)) = BYTE )
#define SDRAM_WRITE_SHORT(ADDR, BYTES)    ((*(__IO uint16_t*) (ADDR)) = BYTES)
#define SDRAM_WRITE_LONG(ADDR, BYTES)     ((*(__IO uint32_t*) (ADDR)) = BYTES)



void SDRAM_Initialize(void);
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram);
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram);

#endif
