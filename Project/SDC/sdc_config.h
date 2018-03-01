#ifndef SDC_CONFIG_H
#define SDC_CONFIG_H

#define SDC_SDIO_NVIC_CHANNEL SDIO_IRQn
#define SDC_DMA_NVIC_CHANNEL  SD_SDIO_DMA_IRQn

#define SDC_DAT0_PORT         GPIOC
#define SDC_DAT0_PIN          GPIO_PIN_8
#define SDC_DAT0_PIN_SOURCE   GPIO_PinSource8

#define SDC_DAT1_PORT         GPIOC
#define SDC_DAT1_PIN          GPIO_PIN_9
#define SDC_DAT1_PIN_SOURCE   GPIO_PinSource9

#define SDC_DAT2_PORT         GPIOC
#define SDC_DAT2_PIN          GPIO_PIN_10
#define SDC_DAT2_PIN_SOURCE   GPIO_PinSource10

#define SDC_DAT3_PORT         GPIOC
#define SDC_DAT3_PIN          GPIO_PIN_11
#define SDC_DAT3_PIN_SOURCE   GPIO_PinSource11

#define SDC_CLK_PORT          GPIOC
#define SDC_CLK_PIN           GPIO_PIN_12
#define SDC_CLK_PIN_SOURCE    GPIO_PinSource12
                              
#define SDC_CMD_PORT          GPIOD
#define SDC_CMD_PIN           GPIO_PIN_2
#define SDC_CMD_PIN_SOURCE    GPIO_PinSource2

#endif //SDC_CONFIG_H
