#ifndef LCD_H
#define LCD_H

#include "common.h"
#include "lcd_config.h"

#define LCD_SET_RESET       LCD_RESET_PORT->BSRRL |= LCD_RESET_PIN
#define LCD_RST_RESET       LCD_RESET_PORT->BSRRH |= LCD_RESET_PIN
                            
#define LCD_SET_RS          LCD_RS_PORT->BSRRL |= LCD_RS_PIN
#define LCD_RST_RS          LCD_RS_PORT->BSRRH |= LCD_RS_PIN
                            
#define LCD_SET_CS          LCD_CS_PORT->BSRRL |= LCD_CS_PIN
#define LCD_RST_CS          LCD_CS_PORT->BSRRH |= LCD_CS_PIN
                            
#define LCD_SET_RD          LCD_RD_PORT->BSRRL |= LCD_RD_PIN
#define LCD_RST_RD          LCD_RD_PORT->BSRRH |= LCD_RD_PIN
                            
#define LCD_SET_WR          LCD_WR_PORT->BSRRL |= LCD_WR_PIN
#define LCD_RST_WR          LCD_WR_PORT->BSRRH |= LCD_WR_PIN

#define LCD_SET_READY_FLAG  LCD_READY_FLAG = true;
#define LCD_RST_READY_FLAG  LCD_READY_FLAG = false;

typedef enum
{
    LCD_FRAME_RATE_DIVISION_RATIO1 = 0x00,
    LCD_FRAME_RATE_DIVISION_RATIO2 = 0x01,
}LCD_FRAME_RATE_DIVISION_RATIO_t;

typedef enum
{
    LCD_FRAME_RATE_61HZ  = 0x1F,
    LCD_FRAME_RATE_83HZ  = 0x17,
    LCD_FRAME_RATE_95HZ  = 0x14,
    LCD_FRAME_RATE_100HZ = 0x13,
    LCD_FRAME_RATE_106HZ = 0x12,
    LCD_FRAME_RATE_112HZ = 0x11,
    LCD_FRAME_RATE_119HZ = 0x10,
}LCD_FRAME_RATE_t;

extern bool LCD_READY_FLAG; // External declaration for main.c file


void LCD_Initialize(void);

void LCD_DimBacklight(long percent);

void LCD_ClearColor(uint16_t color);
void LCD_DrawFrameBuffer(void);
void LCD_DrawFrameBufferScaled(void);

void LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO_t divRatio, LCD_FRAME_RATE_t frameRate);
void LCD_SetDrawArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void EXTI0_IRQHandler(void);

#endif //LCD_H
