#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#include "input.h"


/******************************************************************************/
/*            							 		  LCD Constants            								 	  */
/******************************************************************************/

#define LCD_DISPLAY_SIZE_X        			320
#define LCD_DISPLAY_SIZE_Y        			240

#define LCD_DISPLAY_PIXELS        		  76800

#define LCD_DMA_MAX_SIZE_PER_TRANSFER   50000	// Do not change; must be lower or equal to 2^16 - 2


/******************************************************************************/
/*            							 		  LCD Commands            								 	  */
/******************************************************************************/

#define LCD_SET_RESET       				    LCD_RESET_PORT->BSRRL |= LCD_RESET_PIN
#define LCD_RST_RESET       				    LCD_RESET_PORT->BSRRH |= LCD_RESET_PIN

#define LCD_SET_RS          				    LCD_RS_PORT->BSRRL |= LCD_RS_PIN
#define LCD_RST_RS          				    LCD_RS_PORT->BSRRH |= LCD_RS_PIN

#define LCD_SET_CS          				    LCD_CS_PORT->BSRRL |= LCD_CS_PIN
#define LCD_RST_CS          				    LCD_CS_PORT->BSRRH |= LCD_CS_PIN

#define LCD_SET_RD          				    LCD_RD_PORT->BSRRL |= LCD_RD_PIN
#define LCD_RST_RD          				    LCD_RD_PORT->BSRRH |= LCD_RD_PIN

#define LCD_SET_WR          				    LCD_WR_PORT->BSRRL |= LCD_WR_PIN
#define LCD_RST_WR          				    LCD_WR_PORT->BSRRH |= LCD_WR_PIN

#define LCD_SET_READY_FLAG  				    LCD_READY_FLAG = true;
#define LCD_RST_READY_FLAG  				    LCD_READY_FLAG = false;


/******************************************************************************/
/*                         LCD Peripheral Definitions                         */
/******************************************************************************/

#define LCD_PORT_BACKLIGHT        GPIOA
#define LCD_PIN_BACKLIGHT         GPIO_PIN_11
#define LCD_ALT_BACKLIGHT         GPIO_AF1_TIM1
#define LCD_TIM_BACKLIGHT         TIM1
#define LCD_TIM_BACKLIGHT_CHANNEL TIM_CHANNEL_4

#define LCD_RESET_PORT            GPIOD
#define LCD_RESET_PIN             GPIO_PIN_6

#define LCD_RS_PORT               GPIOD
#define LCD_RS_PIN                GPIO_PIN_7

#define LCD_CS_PORT               GPIOD
#define LCD_CS_PIN                GPIO_PIN_8

#define LCD_RD_PORT               GPIOD
#define LCD_RD_PIN                GPIO_PIN_9

#define LCD_WR_PORT               GPIOG
#define LCD_WR_PORT_ODR_BYTE      0
#define LCD_WR_PIN                GPIO_PIN_0
#define LCD_WR_SET_TIM_CHANNEL    TIM_CHANNEL_1
#define LCD_WR_RST_TIM_CHANNEL    TIM_CHANNEL_3

#define LCD_DATA_PORT             GPIOF
#define LCD_DATA_WR_TIM           TIM8
#define LCD_DATA_TIM_CHANNEL      TIM_CHANNEL_2

#endif //LCD_CONFIG_H
