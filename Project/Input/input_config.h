#ifndef INPUT_CONFIG_H
#define INPUT_CONFIG_H

#include "stm32f4xx_conf.h"

#define INPUT_LOCK_TIM                      TIM4
#define INPUT_LOCK_TIM_BUS                  RCC_APB1Periph_TIM4

#define INPUT_POLLING_TIM                   TIM3
#define INPUT_POLLING_TIM_BUS               RCC_APB1Periph_TIM3
#define INPUT_POLLING_TIM_NVIC_CHANNEL      TIM3_IRQn

#define INPUT_BUS_ALL                       RCC_AHB1Periph_GPIOB
#define INPUT_PORT_ALL                      GPIOB

#define INPUT_A_PORT                        GPIOB
#define INPUT_A_PIN                         GPIO_Pin_1

#define INPUT_B_PORT                        GPIOB
#define INPUT_B_PIN                         GPIO_Pin_2

#define INPUT_SELECT_PORT                   GPIOB
#define INPUT_SELECT_PIN                    GPIO_Pin_3

#define INPUT_START_PORT                    GPIOB
#define INPUT_START_PIN                     GPIO_Pin_4

#define INPUT_FADE_RIGHT_PORT               GPIOB
#define INPUT_FADE_RIGHT_PIN                GPIO_Pin_5

#define INPUT_FADE_LEFT_PORT                GPIOB
#define INPUT_FADE_LEFT_PIN                 GPIO_Pin_6

#define INPUT_FADE_TOP_PORT                 GPIOB
#define INPUT_FADE_TOP_PIN                  GPIO_Pin_7

#define INPUT_FADE_BOTTOM_PORT              GPIOB
#define INPUT_FADE_BOTTOM_PIN               GPIO_Pin_8

#endif //INPUT_CONFIG_H
