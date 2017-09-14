#ifndef INPUT_CONFIG_H
#define INPUT_CONFIG_H

#include "stm32f4xx_conf.h"

#define INPUT_FRAME_PORT                    GPIOB
#define INPUT_FRAME_PIN                     GPIO_Pin_0
#define INPUT_FRAME_EXTI_PORT               EXTI_PortSourceGPIOB
#define INPUT_FRAME_EXTI_PIN                EXTI_PinSource0
#define INPUT_FRAME_EXTI_LINE               EXTI_Line0
#define INPUT_FRAME_NVIC_CHANNEL            EXTI0_IRQn

#define INPUT_A_PORT                        GPIOB
#define INPUT_A_PIN                         GPIO_Pin_1
#define INPUT_A_EXTI_PORT                   EXTI_PortSourceGPIOB
#define INPUT_A_EXTI_PIN                    EXTI_PinSource1
#define INPUT_A_EXTI_LINE                   EXTI_Line1
#define INPUT_A_NVIC_CHANNEL                EXTI1_IRQn

#define INPUT_B_PORT                        GPIOB
#define INPUT_B_PIN                         GPIO_Pin_2
#define INPUT_B_EXTI_PORT                   EXTI_PortSourceGPIOB
#define INPUT_B_EXTI_PIN                    EXTI_PinSource2
#define INPUT_B_EXTI_LINE                   EXTI_Line2
#define INPUT_B_NVIC_CHANNEL                EXTI2_IRQn

#define INPUT_START_PORT                    GPIOB
#define INPUT_START_PIN                     GPIO_Pin_3
#define INPUT_START_EXTI_PORT               EXTI_PortSourceGPIOB
#define INPUT_START_EXTI_PIN                EXTI_PinSource3
#define INPUT_START_EXTI_LINE               EXTI_Line3
#define INPUT_START_NVIC_CHANNEL            EXTI3_IRQn

#define INPUT_SELECT_PORT                   GPIOB
#define INPUT_SELECT_PIN                    GPIO_Pin_4
#define INPUT_SELECT_EXTI_PORT              EXTI_PortSourceGPIOB
#define INPUT_SELECT_EXTI_PIN               EXTI_PinSource4
#define INPUT_SELECT_EXTI_LINE              EXTI_Line4
#define INPUT_SELECT_NVIC_CHANNEL           EXTI4_IRQn

#define INPUT_FADE_TOP_PORT                 GPIOB
#define INPUT_FADE_TOP_PIN                  GPIO_Pin_5
#define INPUT_FADE_TOP_EXTI_PORT            EXTI_PortSourceGPIOB
#define INPUT_FADE_TOP_EXTI_PIN             EXTI_PinSource5
#define INPUT_FADE_TOP_EXTI_LINE            EXTI_Line5
#define INPUT_FADE_TOP_NVIC_CHANNEL         EXTI9_5_IRQn

#define INPUT_FADE_RIGHT_PORT               GPIOB
#define INPUT_FADE_RIGHT_PIN                GPIO_Pin_6
#define INPUT_FADE_RIGHT_EXTI_PORT          EXTI_PortSourceGPIOB
#define INPUT_FADE_RIGHT_EXTI_PIN           EXTI_PinSource6
#define INPUT_FADE_RIGHT_EXTI_LINE          EXTI_Line6
#define INPUT_FADE_RIGHT_NVIC_CHANNEL       EXTI9_5_IRQn

#define INPUT_FADE_BOTTOM_PORT              GPIOB
#define INPUT_FADE_BOTTOM_PIN               GPIO_Pin_7
#define INPUT_FADE_BOTTOM_EXTI_PORT         EXTI_PortSourceGPIOB
#define INPUT_FADE_BOTTOM_EXTI_PIN          EXTI_PinSource7
#define INPUT_FADE_BOTTOM_EXTI_LINE         EXTI_Line7
#define INPUT_FADE_BOTTOM_NVIC_CHANNEL      EXTI9_5_IRQn

#define INPUT_FADE_LEFT_PORT                GPIOB
#define INPUT_FADE_LEFT_PIN                 GPIO_Pin_8
#define INPUT_FADE_LEFT_EXTI_PORT           EXTI_PortSourceGPIOB
#define INPUT_FADE_LEFT_EXTI_PIN            EXTI_PinSource8
#define INPUT_FADE_LEFT_EXTI_LINE           EXTI_Line8
#define INPUT_FADE_LEFT_NVIC_CHANNEL        EXTI9_5_IRQn

#endif //INPUT_CONFIG_H
