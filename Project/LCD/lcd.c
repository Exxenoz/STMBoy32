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
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    LCD_Initialize_Pins();
    
    for (uint16_t i = 0; i < 50000; i++);
    LCD_RST_RESET;
    for (uint16_t i = 0; i < 50000; i++);
    LCD_SET_RESET;
    for (long i = 0; i < 50000; i++);
    
    uint16_t data[16] = {0};
    data[0] = 0x00;
    LCD_WriteBuffer(LCD_REG_WRITE_CTRL_DISPLAY, data, 1);

    return false;
}

void LCD_WriteAddr(uint16_t addr)
{
    LCD_DATA_PORT->ODR = addr;
    LCD_RST_RS; // Set to command
    LCD_RST_WR; // Set to write
    NOP; NOP; NOP;
    LCD_SET_WR;
    NOP; NOP; NOP;
    LCD_SET_RS; // Set to data
    LCD_DATA_PORT->ODR = 0x00;
}

void LCD_WriteData(uint16_t data)
{
    LCD_DATA_PORT->ODR = data;
    LCD_RST_WR; // Set to write
    NOP; NOP; NOP;
    LCD_SET_WR;
    NOP; NOP; NOP;
    LCD_DATA_PORT->ODR = 0x00;
}

void LCD_WriteCommand(uint16_t addr)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_SET_CS;
}

void LCD_Write(uint16_t addr, uint16_t data)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_WriteData(data);
    LCD_SET_CS;
}

void LCD_WriteBuffer(uint16_t addr, uint16_t buffer[], long length)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    for (long i = 0; i < length; i++)
    {
        LCD_WriteData(buffer[i]);
    }
    LCD_SET_CS;
}

uint16_t LCD_ReadData(void)
{
    LCD_DATA_PORT->MODER = GPIO_INPUT_MODE;
    LCD_RST_RD;
    for (uint16_t i = 0; i < 42; i++);
    uint16_t data = LCD_DATA_PORT->IDR;
    LCD_SET_RD;
    for (uint16_t i = 0; i < 10; i++);
    LCD_DATA_PORT->MODER = GPIO_OUTPUT_MODE;
    return data;
}

uint16_t LCD_Read(uint16_t addr)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    uint16_t data = LCD_ReadData();
    LCD_SET_CS;
    return data;
}

void LCD_ReadBuffer(uint16_t addr, uint16_t buffer[], long length)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    for (long i = 0; i < length; i++)
    {
        buffer[i] = LCD_ReadData();
    }
    LCD_SET_CS;
}

void LCD_SetColumnAddress(uint16_t startColumn, uint16_t endColumn)
{
    uint16_t data[4];
    data[0] = startColumn >> 8;
    data[1] = startColumn & 0xFF;
    data[2] = endColumn >> 8;
    data[3] = endColumn & 0xFF;
    LCD_WriteBuffer(LCD_REG_SET_COLUMN_ADDRESS, data, 4);
}

void LCD_SetRowAddress(uint16_t startRow, uint16_t endRow)
{
    uint16_t data[4];
    data[0] = startRow >> 8;
    data[1] = startRow & 0xFF;
    data[2] = endRow >> 8;
    data[3] = endRow & 0xFF;
    LCD_WriteBuffer(LCD_REG_SET_ROW_ADDRESS, data, 4);
}
