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
void LCD_Write(uint16_t addr, uint16_t data);

uint16_t LCD_ReadData(void);
uint16_t LCD_Read(uint16_t addr);

typedef enum LCD_BusyFlag_e
{
    LCD_BUSYFLAG_NONE  = 0,
    LCD_BUSYFLAG_WRITE = 1,
    LCD_BUSYFLAG_READ  = 2,
}
LCD_BusyFlag;

LCD_BusyFlag LCD_BusyFlags = 0;

#endif //LCD_H
