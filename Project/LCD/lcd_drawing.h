#ifndef LCD_DRAWING_H
#define LCD_DRAWING_H

#include "common.h"
#include "gbc_gpu.h"
#include "ui_fonts.h"

#include "lcd.h"
#include "lcd_types.h"
#include "lcd_config.h"



void LCD_ClearColor(uint16_t color);

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t width, uint16_t color, LCD_Orientation_t o);
void LCD_DrawEmptyBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t width, uint16_t color);
void LCD_DrawFilledBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t color);
void LCD_DrawBrickline(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool offset, LCD_Brick_t *p_brick);
void LCD_DrawWall(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool initOffset, LCD_Brick_t *p_brick);
void LCD_DrawText(uint16_t x0, uint16_t y0, uint16_t bgColor, LCD_TextDef_t *text, Fonts_FontDef_16_t *p_font);
void LCD_DrawSymbol(uint16_t x0, uint16_t y0, uint16_t color, Fonts_SymbolDef_32_t *p_symbol);

#endif //LCD_DRAWING_H
