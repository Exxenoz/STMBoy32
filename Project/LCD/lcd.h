#ifndef LCD_H
#define LCD_H

#include "common.h"
#include "lcd_config.h"
#include "lcd_regdef.h"

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

bool LCD_Initialize(void);

void LCD_DimBacklight(long percent);

void LCD_WriteAddr(uint16_t addr);
void LCD_WriteData(uint16_t data);
void LCD_WriteCommand(uint16_t addr);
void LCD_Write(uint16_t addr, uint16_t data);
void LCD_WriteBuffer(uint16_t addr, uint16_t buffer[], long length);

uint16_t LCD_ReadData(void);
uint16_t LCD_Read(uint16_t addr);
void LCD_ReadBuffer(uint16_t addr, uint16_t buffer[], long length);

void LCD_SetDrawAreaHorizontal(uint16_t startColumn, uint16_t endColumn);
void LCD_SetDrawAreaVertical(uint16_t startRow, uint16_t endRow);
void LCD_SetDrawArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void LCD_ClearColor(uint16_t color);
void LCD_PrintKaro(uint16_t color, uint16_t offset); // Debug
void LCD_DrawFrameBuffer(void);

void EXTI0_IRQHandler(void);

#endif //LCD_H
