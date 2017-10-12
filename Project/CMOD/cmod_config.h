#ifndef CMOD_CONFIG_H
#define CMOD_CONFIG_H

#define CMOD_BUS(NAME)    RCC_AHB1Periph_##NAME

#define CMOD_RESET_PORT   GPIOG
#define CMOD_RESET_PIN    GPIO_Pin_8
#define CMOD_RESET_BUS    CMOD_BUS(GPIOG)

#define CMOD_CS_PORT      GPIOG
#define CMOD_CS_PIN       GPIO_Pin_7
#define CMOD_CS_BUS       CMOD_BUS(GPIOG)

#define CMOD_RD_PORT      GPIOG
#define CMOD_RD_PIN       GPIO_Pin_6
#define CMOD_RD_BUS       CMOD_BUS(GPIOG)

#define CMOD_WR_PORT      GPIOG
#define CMOD_WR_PIN       GPIO_Pin_5
#define CMOD_WR_BUS       CMOD_BUS(GPIOG)

#define CMOD_CLK_PORT     GPIOB
#define CMOD_CLK_PIN      GPIO_Pin_9
#define CMOD_CLK_BUS      CMOD_BUS(GPIOB)

#define CMOD_ADDR_PORT    GPIOE
#define CMOD_ADDR_BUS     CMOD_BUS(GPIOE)

#define CMOD_DATA_PORT    GPIOC
#define CMOD_DATA_BUS     CMOD_BUS(GPIOC)

#endif
