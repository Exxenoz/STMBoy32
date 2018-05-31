#ifndef LCD_H
#define LCD_H

#include "common.h"
#include "lcd_config.h"
#include "ui_fonts.h"

#define LCD_MAX_TEXT_LENGTH 20

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
}
LCD_FRAME_RATE_DIVISION_RATIO_t;

typedef enum
{
    LCD_FRAME_RATE_61HZ  = 0x1F,
    LCD_FRAME_RATE_83HZ  = 0x17,
    LCD_FRAME_RATE_95HZ  = 0x14,
    LCD_FRAME_RATE_100HZ = 0x13,
    LCD_FRAME_RATE_106HZ = 0x12,
    LCD_FRAME_RATE_112HZ = 0x11,
    LCD_FRAME_RATE_119HZ = 0x10,
}
LCD_FRAME_RATE_t;

typedef enum
{
    LCD_HORIZONTAL,
    LCD_VERTICAL,
}
LCD_Orientation_t;

typedef struct
{
    uint16_t Width;
    uint16_t Color;
}
LCD_Border_t;

typedef struct
{
    uint16_t     Length;
    uint16_t     Height;
    uint16_t     Color;
    LCD_Border_t Border;
}
LCD_Brick_t;

typedef struct
{
    int Upper;
    int Right;
    int Lower;
    int Left;
}
LCD_Padding_t;

typedef struct
{
    char          Characters[LCD_MAX_TEXT_LENGTH];
    uint16_t      Spacing;
    uint16_t      Height;
    uint16_t      Color;
    LCD_Padding_t Padding;
    LCD_Border_t  Border;
}
LCD_TextDef_t;

#pragma pack(1)
typedef union LCD_Pixel_u
{
    uint16_t Color;

    #pragma pack(1)
    struct
    {
        unsigned int Blue  : 5; // LSB
        unsigned int Green : 6;
        unsigned int Red   : 5; // MSB
    };
}
LCD_Pixel_t;

extern bool LCD_READY_FLAG; // External declaration for main.c file

void LCD_Initialize(void);

void LCD_DimBacklight(long percent);
void LCD_ClearColor(uint16_t color);

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t width, uint16_t color, LCD_Orientation_t o);
void LCD_DrawEmptyBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t width, uint16_t color);
void LCD_DrawFilledBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t color);
void LCD_DrawBrickline(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool offset, LCD_Brick_t *p_brick);
void LCD_DrawWall(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool initOffset, LCD_Brick_t *p_brick);
void LCD_DrawText(uint16_t x0, uint16_t y0, uint16_t bgColor, LCD_TextDef_t *text, Fonts_FontDef_16_t *p_font);
void LCD_DrawSymbol(uint16_t x0, uint16_t y0, uint16_t color, Fonts_SymbolDef_32_t *p_symbol);
void LCD_DrawFrameBuffer(uint16_t* frameBuffer, uint32_t frameBufferLength, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void LCD_DrawGBCFrameBuffer(void);
void LCD_DrawGBCFrameBufferScaled(void);

void LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO_t divRatio, LCD_FRAME_RATE_t frameRate);
void LCD_SetDrawArea(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);

#endif //LCD_H
