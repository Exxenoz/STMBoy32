 #ifndef CMOD_CONFIG_H
#define CMOD_CONFIG_H


#define CMOD_BUS(NAME)            RCC_AHB1Periph_##NAME

#define CMOD_TIM                  TIM5
#define CMOD_TIM_CHANNEL          TIM_CHANNEL_4
#define CMOD_TIM_NVIC_CHANNEL     TIM5_IRQn

#define CMOD_RESET_PORT           GPIOE
#define CMOD_RESET_PIN            GPIO_PIN_5

#define CMOD_CS_PORT              GPIOE
#define CMOD_CS_PIN               GPIO_PIN_4

#define CMOD_RD_PORT              GPIOE
#define CMOD_RD_PIN               GPIO_PIN_3

#define CMOD_WR_PORT              GPIOE
#define CMOD_WR_PIN               GPIO_PIN_2

#define CMOD_CLK_PORT             GPIOD
#define CMOD_CLK_PIN              GPIO_PIN_12
#define CMOD_CLK_PINSOURCE        GPIO_PinSource12
#define CMOD_CLK_AF               GPIO_AF_TIM4

#define CMOD_ADDR_PORT            GPIOB
#define CMOD_ADDR_PINS            GPIO_PIN_All

#define CMOD_DATA_PORT            GPIOI
#define CMOD_DATA_PINS            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7

#endif
