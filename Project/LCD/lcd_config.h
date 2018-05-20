#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#define LCD_DISPLAY_SIZE_X      320
#define LCD_DISPLAY_SIZE_Y      240

#define LCD_DISPLAY_PIXELS      76800

// If necessary, Timer Channel can be changed inside the Initialize-Function
#define LCD_DMA_TIM               TIM1
#define LCD_DMA_M_TIM_CHANNEL     TIM_CHANNEL_1
#define LCD_DMA_S1_TIM_CHANNEL    TIM_CHANNEL_2
#define LCD_DMA_S2_TIM_CHANNEL    TIM_CHANNEL_3

#define LCD_PORT_WR               GPIOA
#define LCD_PIN_WR                GPIO_PIN_3
#define LCD_ALT_WR                GPIO_AF2_TIM5
#define LCD_TIM_WR                TIM5
#define LCD_TIM_CHANNEL_WR        TIM_CHANNEL_4

#define LCD_PORT_BACKLIGHT        GPIOA
#define LCD_PIN_BACKLIGHT         GPIO_PIN_11
#define LCD_ALT_BACKLIGHT         GPIO_AF1_TIM1
#define LCD_TIM_BACKLIGHT         TIM1
#define LCD_TIM_BACKLIGHT_CHANNEL TIM_CHANNEL_4

#define INPUT_FRAME_PORT          GPIOB
#define INPUT_FRAME_PIN           GPIO_PIN_0

#define LCD_RESET_PORT            GPIOD
#define LCD_RESET_PIN             GPIO_PIN_6

#define LCD_RS_PORT               GPIOD
#define LCD_RS_PIN                GPIO_PIN_7

#define LCD_CS_PORT               GPIOD
#define LCD_CS_PIN                GPIO_PIN_8

#define LCD_RD_PORT               GPIOD
#define LCD_RD_PIN                GPIO_PIN_9

#define LCD_PORT_DATA             GPIOF
#define LCD_TIM_DATA              TIM4
#define LCD_TIM_CHANNEL_DATA      TIM_CHANNEL_1

#endif //LCD_CONFIG_H
