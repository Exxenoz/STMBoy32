#include "lcd.h"

void LCD_Initialize_Pins(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)  \
    GPIO_InitObject.GPIO_Mode = GPIO_Mode_OUT; \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP; \
    GPIO_InitObject.GPIO_Pin = PIN;              \
    GPIO_InitObject.GPIO_PuPd = GPIO_PuPd_NOPULL; \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;\
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LCD_RESET_PORT, LCD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RS_PORT, LCD_RS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_CS_PORT, LCD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RD_PORT, LCD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_WR_PORT, LCD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_DATA_PORT, GPIO_Pin_All);

    LCD_SET_RESET;
    LCD_SET_RS;
    LCD_SET_CS;
    LCD_SET_RD;
    LCD_SET_WR;
}

bool LCD_Initialize(void)
{
    return false;
}

void LCD_WriteAddr(uint16_t addr)
{
    LCD_RST_RS; // Set to command
    LCD_BusyFlags = LCD_BUSYFLAG_WRITE;
    // ToDo: Set Interrupt
    while (LCD_BusyFlags);
    // ToDo: More stuff...
    LCD_SET_RS; // Set to data
}

void LCD_WriteData(uint16_t data)
{

}

void LCD_Write(uint16_t addr, uint16_t data)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    // ToDo: More stuff...
}

uint16_t LCD_ReadData(void)
{
    return 0;
}

uint16_t LCD_Read(uint16_t addr)
{
    return 0;
}
