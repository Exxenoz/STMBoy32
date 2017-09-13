#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#include "stm32f4xx_conf.h"

#define LCD_DISPLAY_SIZE_X 320
#define LCD_DISPLAY_SIZE_Y 240

#define LCD_HALF_DISPLAY_SIZE_X 160
#define LCD_HALF_DISPLAY_SIZE_Y 120

#define LCD_DISPLAY_PIXELS 76800 // LCD_DISPLAY_SIZE_X * LCD_DISPLAY_SIZE_Y

#define LCD_RESET_PORT GPIOG
#define LCD_RESET_PIN  GPIO_Pin_0

#define LCD_RS_PORT    GPIOG
#define LCD_RS_PIN     GPIO_Pin_1

#define LCD_CS_PORT    GPIOG
#define LCD_CS_PIN     GPIO_Pin_2

#define LCD_RD_PORT    GPIOG
#define LCD_RD_PIN     GPIO_Pin_3

#define LCD_WR_PORT    GPIOG
#define LCD_WR_PIN     GPIO_Pin_4

#define LCD_DATA_PORT  GPIOF

#endif //LCD_CONFIG_H
