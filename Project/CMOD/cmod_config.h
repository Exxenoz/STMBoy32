#ifndef CMOD_CONFIG_H
#define CMOD_CONFIG_H

#define CMOD_BUS(NAME)    RCC_AHB1Periph_##NAME

#define CMOD_RESET_PORT   GPIOD
#define CMOD_RESET_PIN    GPIO_Pin_15
#define CMOD_RESET_BUS    CMOD_BUS(GPIOD)

#define CMOD_CS_PORT      GPIOD
#define CMOD_CS_PIN       GPIO_Pin_14
#define CMOD_CS_BUS       CMOD_BUS(GPIOD)

#define CMOD_RD_PORT      GPIOD
#define CMOD_RD_PIN       GPIO_Pin_13
#define CMOD_RD_BUS       CMOD_BUS(GPIOD)

#define CMOD_WR_PORT      GPIOD
#define CMOD_WR_PIN       GPIO_Pin_12
#define CMOD_WR_BUS       CMOD_BUS(GPIOD)

#define CMOD_DETECT_PORT  GPIOD
#define CMOD_DETECT_PIN   GPIO_Pin_11
#define CMOD_DETECT_BUS   CMOD_BUS(GPIOD)

#define CMOD_CLK_PORT     GPIOB
#define CMOD_CLK_PIN      GPIO_Pin_9
#define CMOD_CLK_BUS      CMOD_BUS(GPIOB)

#define CMOD_ADDR_PORT    GPIOE
#define CMOD_ADDR_PINS    GPIO_Pin_All
#define CMOD_ADDR_BUS     CMOD_BUS(GPIOE)

#define CMOD_DATA_PORT    GPIOG
#define CMOD_DATA_PINS    GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
#define CMOD_DATA_BUS     CMOD_BUS(GPIOG)

#endif
