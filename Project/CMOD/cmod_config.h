#ifndef CMOD_CONFIG_H
#define CMOD_CONFIG_H

#include "stm32f4xx_conf.h"

#define CMOD_BUS(NAME)            RCC_AHB1Periph_##NAME

// If necessary, Timer Channel can be changed inside the Initialize-Function
#define CMOD_TIM                  TIM5
#define CMOD_TIM_BUS              RCC_APB1Periph_TIM5
#define CMOD_TIM_NVIC_CHANNEL     TIM5_IRQn

#define CMOD_RESET_PORT           GPIOD
#define CMOD_RESET_PIN            GPIO_Pin_15
#define CMOD_RESET_BUS            CMOD_BUS(GPIOD)

#define CMOD_CS_PORT              GPIOD
#define CMOD_CS_PIN               GPIO_Pin_14
#define CMOD_CS_BUS               CMOD_BUS(GPIOD)

#define CMOD_RD_PORT              GPIOD
#define CMOD_RD_PIN               GPIO_Pin_13
#define CMOD_RD_BUS               CMOD_BUS(GPIOD)

#define CMOD_WR_PORT              GPIOD
#define CMOD_WR_PIN               GPIO_Pin_12
#define CMOD_WR_BUS               CMOD_BUS(GPIOD)

#define CMOD_CLK_PORT             GPIOB
#define CMOD_CLK_PIN              GPIO_Pin_9
#define CMOD_CLK_PINSOURCE        GPIO_PinSource9
#define CMOD_CLK_BUS              CMOD_BUS(GPIOB)
#define CMOD_CLK_AF               GPIO_AF_TIM4

#define CMOD_ADDR_PORT            GPIOE
#define CMOD_ADDR_PINS            GPIO_Pin_All
#define CMOD_ADDR_BUS             CMOD_BUS(GPIOE)

#define CMOD_DATA_PORT            GPIOG
#define CMOD_DATA_PINS            GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
#define CMOD_DATA_BUS             CMOD_BUS(GPIOG)

#define CMOD_DETECT_PORT          GPIOD
#define CMOD_DETECT_PIN           GPIO_Pin_11
#define CMOD_DETECT_BUS           CMOD_BUS(GPIOD)
#define CMOD_DETECT_EXTI_PORT     EXTI_PortSourceGPIOD
#define CMOD_DETECT_EXTI_PIN      EXTI_PinSource11
#define CMOD_DETECT_EXTI_LINE     EXTI_Line11
#define CMOD_DETECT_NVIC_CHANNEL  EXTI15_10_IRQn

#endif
