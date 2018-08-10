#ifndef LED_CONFIG_H
#define LED_CONFIG_H


#define LED_BUS(NAME)         RCC_AHB1Periph_##NAME

#define LED_RED_BUS           LED_BUS(GPIOA)
#define LED_RED_PORT          GPIOA
#define LED_RED_PIN           GPIO_PIN_8
#define LED_RED_PIN_SOURCE    GPIO_PinSource8

#define LED_GREEN_BUS         LED_BUS(GPIOA)
#define LED_GREEN_PORT        GPIOA
#define LED_GREEN_PIN         GPIO_PIN_9
#define LED_GREEN_PIN_SOURCE  GPIO_PinSource9

#define LED_BLUE_BUS          LED_BUS(GPIOA)
#define LED_BLUE_PORT         GPIOA
#define LED_BLUE_PIN          GPIO_PIN_10
#define LED_BLUE_PIN_SOURCE   GPIO_PinSource10

#endif //LED_CONFIG_H
