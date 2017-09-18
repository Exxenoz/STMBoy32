#ifndef SDC_CONFIG_H
#define SDC_CONFIG_H

#include "stm32f4xx_conf.h"

#define SDC_DAT0_PORT         GPIOC
#define SDC_DAT0_PIN          GPIO_Pin_8
#define SDC_DAT0_PIN_SOURCE   GPIO_PinSource8

#define SDC_DAT1_PORT         GPIOC
#define SDC_DAT1_PIN          GPIO_Pin_9
#define SDC_DAT1_PIN_SOURCE   GPIO_PinSource9

#define SDC_DAT2_PORT         GPIOC
#define SDC_DAT2_PIN          GPIO_Pin_10
#define SDC_DAT2_PIN_SOURCE   GPIO_PinSource10

#define SDC_DAT3_PORT         GPIOC
#define SDC_DAT3_PIN          GPIO_Pin_11
#define SDC_DAT3_PIN_SOURCE   GPIO_PinSource11

#define SDC_CLK_PORT         GPIOC
#define SDC_CLK_PIN          GPIO_Pin_12
#define SDC_CLK_PIN_SOURCE   GPIO_PinSource12

#define SDC_CMD_PORT         GPIOD
#define SDC_CMD_PIN          GPIO_Pin_2
#define SDC_CMD_PIN_SOURCE   GPIO_PinSource2

#endif //SDC_CONFIG_H
