#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#define LCD_DISPLAY_SIZE_X      320
#define LCD_DISPLAY_SIZE_Y      240

#define LCD_DISPLAY_PIXELS      76800

#define LCD_BUS(NAME)            RCC_AHB1Periph_##NAME

// If necessary, Timer Channel can be changed inside the Initialize-Function
#define LCD_TIM                  TIM1
#define LCD_TIM_BUS              RCC_APB2Periph_TIM1
#define LCD_TIM_CHANNEL          TIM_CHANNEL_4
#define ENABLE_LCD_TIM()         __TIM1_CLK_ENABLE()

#define LCD_RESET_PORT           GPIOD
#define LCD_RESET_PIN            GPIO_PIN_6

#define LCD_RS_PORT              GPIOD
#define LCD_RS_PIN               GPIO_PIN_7

#define LCD_CS_PORT              GPIOD
#define LCD_CS_PIN               GPIO_PIN_8

#define LCD_RD_PORT              GPIOD
#define LCD_RD_PIN               GPIO_PIN_9

#define LCD_WR_PORT              GPIOD
#define LCD_WR_PIN               GPIO_PIN_10

#define LCD_BACKLIT_PORT         GPIOA
#define LCD_BACKLIT_PIN          GPIO_PIN_11

#define INPUT_FRAME_PORT         GPIOB
#define INPUT_FRAME_PIN          GPIO_PIN_0

#define LCD_DATA_PORT            GPIOF
#define LCD_DATA_BUS             RCC_AHB1Periph_GPIOF

#endif //LCD_CONFIG_H
