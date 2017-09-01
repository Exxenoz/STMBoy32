#ifndef LCD_H
#define LCD_H

#include "common.h"
#include "lcd_config.h"
#include "lcd_regdef.h"

#define LCD_SET_RESET GPIO_SetBits(LCD_RESET_PORT, LCD_RESET_PIN)
#define LCD_RST_RESET GPIO_ResetBits(LCD_RESET_PORT, LCD_RESET_PIN)

#define LCD_SET_RS GPIO_SetBits(LCD_RS_PORT, LCD_RS_PIN)
#define LCD_RST_RS GPIO_ResetBits(LCD_RS_PORT, LCD_RS_PIN)

#define LCD_SET_CS GPIO_SetBits(LCD_CS_PORT, LCD_CS_PIN)
#define LCD_RST_CS GPIO_ResetBits(LCD_CS_PORT, LCD_CS_PIN)

#define LCD_SET_RD GPIO_SetBits(LCD_RD_PORT, LCD_RD_PIN)
#define LCD_RST_RD GPIO_ResetBits(LCD_RD_PORT, LCD_RD_PIN)

#define LCD_SET_WR GPIO_SetBits(LCD_WR_PORT, LCD_WR_PIN)
#define LCD_RST_WR GPIO_ResetBits(LCD_WR_PORT, LCD_WR_PIN)

void LCD_Initialize_Pins(void);
bool LCD_Initialize(void);

void LCD_WriteAddr(uint16_t addr);
void LCD_WriteData(uint16_t data);
void LCD_WriteCommand(uint16_t addr);
void LCD_Write(uint16_t addr, uint16_t data);
void LCD_WriteBuffer(uint16_t addr, uint16_t buffer[], long length);

uint16_t LCD_ReadData(void);
uint16_t LCD_Read(uint16_t addr);
void LCD_ReadBuffer(uint16_t addr, uint16_t buffer[], long length);

void LCD_SetColumnAddress(uint16_t startColumn, uint16_t endColumn);
void LCD_SetRowAddress(uint16_t startRow, uint16_t endRow);

#endif //LCD_H
