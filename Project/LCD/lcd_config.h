#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#include "stm32f4xx_conf.h"

#define LCD_DISPLAY_SIZE_X 320
#define LCD_DISPLAY_SIZE_Y 240

#define LCD_HALF_DISPLAY_SIZE_X 160
#define LCD_HALF_DISPLAY_SIZE_Y 120

#define LCD_DISPLAY_PIXELS 76800 // LCD_DISPLAY_SIZE_X * LCD_DISPLAY_SIZE_Y

#define LCD_RESET_PORT   GPIOD
#define LCD_RESET_PIN    GPIO_Pin_6

#define LCD_RS_PORT      GPIOD
#define LCD_RS_PIN       GPIO_Pin_7

#define LCD_CS_PORT      GPIOD
#define LCD_CS_PIN       GPIO_Pin_8

#define LCD_RD_PORT      GPIOD
#define LCD_RD_PIN       GPIO_Pin_9

#define LCD_WR_PORT      GPIOD
#define LCD_WR_PIN       GPIO_Pin_10

#define LCD_BACKLIT_PORT GPIOA
#define LCD_BACKLIT_PIN  GPIO_Pin_11
#define LCD_BACKLIT_PIN_SOURCE GPIO_PinSource11

#define LCD_DATA_PORT    GPIOF

#endif //LCD_CONFIG_H
