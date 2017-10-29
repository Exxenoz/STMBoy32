#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#include "stm32f4xx_conf.h"

#define LCD_DISPLAY_SIZE_X      160 //320
#define LCD_DISPLAY_SIZE_Y      144 //240

#define LCD_DISPLAY_PIXELS      23040 //76800   // LCD_DISPLAY_SIZE_X * LCD_DISPLAY_SIZE_Y

#define LCD_HALF_DISPLAY_SIZE_X 80 //160
#define LCD_HALF_DISPLAY_SIZE_Y 72 //120

#define LCD_BUS(NAME)            RCC_AHB1Periph_##NAME

// If necessary, Timer Channel can be changed inside the Initialize-Function
#define LCD_TIM                  TIM1
#define LCD_TIM_BUS              RCC_APB2Periph_TIM1

#define LCD_RESET_PORT           GPIOD
#define LCD_RESET_PIN            GPIO_Pin_6
#define LCD_RESET_BUS            LCD_BUS(GPIOD)

#define LCD_RS_PORT              GPIOD
#define LCD_RS_PIN               GPIO_Pin_7
#define LCD_RS_BUS               LCD_BUS(GPIOD)

#define LCD_CS_PORT              GPIOD
#define LCD_CS_PIN               GPIO_Pin_8
#define LCD_CS_BUS               LCD_BUS(GPIOD)

#define LCD_RD_PORT              GPIOD
#define LCD_RD_PIN               GPIO_Pin_9
#define LCD_RD_BUS               LCD_BUS(GPIOD)

#define LCD_WR_PORT              GPIOD
#define LCD_WR_PIN               GPIO_Pin_10
#define LCD_WR_BUS               LCD_BUS(GPIOD)

#define LCD_BACKLIT_PORT         GPIOA
#define LCD_BACKLIT_PIN          GPIO_Pin_11
#define LCD_BACKLIT_PIN_SOURCE   GPIO_PinSource11
#define LCD_BACKLIT_BUS          RCC_AHB1Periph_GPIOA

#define INPUT_FRAME_PORT         GPIOB
#define INPUT_FRAME_PIN          GPIO_Pin_0
#define INPUT_FRAME_BUS          LCD_BUS(GPIOB)
#define INPUT_FRAME_EXTI_PORT    EXTI_PortSourceGPIOB
#define INPUT_FRAME_EXTI_PIN     EXTI_PinSource0
#define INPUT_FRAME_EXTI_LINE    EXTI_Line0
#define INPUT_FRAME_NVIC_CHANNEL EXTI0_IRQn

#define LCD_DATA_PORT           GPIOF
#define LCD_DATA_BUS            RCC_AHB1Periph_GPIOF

#endif //LCD_CONFIG_H
