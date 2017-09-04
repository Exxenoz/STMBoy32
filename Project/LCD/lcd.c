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

    LCD_WriteCommand(LCD_REG_SOFTWARE_RESET);
    // Wait ~120ms (ILI9341 Datasheet p. 90)
    for (long i = 0; i < 500000; i++);

    LCD_WriteCommand(LCD_REG_DISPLAY_OFF);

    uint16_t data[64];
    data[0] = 0;
    data[1] = 0x83;
    data[2] = 0x30;
    LCD_WriteBuffer(0xCF, data, 3);

    data[0] = 0x64;
    data[1] = 0x03;
    data[2] = 0x12;
    data[3] = 0x81;
    LCD_WriteBuffer(0xED, data, 4);

    data[0] = 0x85;
    data[1] = 0x01;
    data[2] = 0x79;
    LCD_WriteBuffer(0xE8, data, 3);

    data[0] = 0x39;
    data[1] = 0x2C;
    data[2] = 0x00;
    data[3] = 0x34;
    data[4] = 0x02;
    LCD_WriteBuffer(0xCB, data, 5);

    LCD_Write(0xF7, 0x20);

    data[0] = 0x00;
    data[1] = 0x00;
    LCD_WriteBuffer(0xEA, data, 2);

    LCD_Write(LCD_REG_POWER_CONTROL_1, 0x26);
    LCD_Write(LCD_REG_POWER_CONTROL_2, 0x11);

    data[0] = 0x35;
    data[1] = 0x3E;
    LCD_WriteBuffer(LCD_REG_VCOM_CONTROL_1, data, 2);
    LCD_Write(LCD_REG_VCOM_CONTROL_2, 0xBE);

    LCD_Write(LCD_REG_MEMORY_ACCESS_CONTROL, 0x48);
    LCD_Write(LCD_REG_PIXEL_FORMAT_SET, 0x55);

    data[0] = 0x00;
    data[1] = 0x1B;
    LCD_WriteBuffer(LCD_REG_FRAME_RATE_CONTROL, data, 2);

    LCD_Write(0xF2, 0x08); // Gamma Function Disable... probably?

    LCD_Write(LCD_REG_GAMMA_SET, 0x01);

    data[0]  = 0x1f;
    data[1]  = 0x1a;
    data[2]  = 0x18;
    data[3]  = 0x0a;
    data[4]  = 0x0f;
    data[5]  = 0x06;
    data[6]  = 0x45;
    data[7]  = 0x87;
    data[8]  = 0x32;
    data[9]  = 0x0a;
    data[10] = 0x07;
    data[11] = 0x02;
    data[12] = 0x07;
    data[13] = 0x05;
    data[14] = 0x00;
    LCD_WriteBuffer(LCD_REG_POSITIVE_GAMMA_CORRECTION, data, 15);

    data[0]  = 0x00;
    data[1]  = 0x25;
    data[2]  = 0x27;
    data[3]  = 0x05;
    data[4]  = 0x10;
    data[5]  = 0x09;
    data[6]  = 0x3A;
    data[7]  = 0x78;
    data[8]  = 0x4D;
    data[9]  = 0x05;
    data[10] = 0x18;
    data[11] = 0x0D;
    data[12] = 0x38;
    data[13] = 0x3A;
    data[14] = 0x1F;
    LCD_WriteBuffer(LCD_REG_NEGATIVE_GAMMA_CORRECTION, data, 15);

    data[0]  = 0x00;
    data[1]  = 0x00;
    data[2]  = 0x00;
    data[3]  = 0xEF;
    LCD_WriteBuffer(LCD_REG_SET_COLUMN_ADDRESS, data, 4);

    data[0]  = 0x00;
    data[1]  = 0x00;
    data[2]  = 0x01;
    data[3]  = 0x3F;
    LCD_WriteBuffer(LCD_REG_SET_ROW_ADDRESS, data, 4);

    LCD_Write(LCD_REG_ENTRY_MODE_SET, 0x07);

    data[0]  = 0x0A;
    data[1]  = 0x82;
    data[2]  = 0x27;
    data[3]  = 0x00;
    LCD_WriteBuffer(LCD_REG_DISPLAY_FUNCTION_CONTROL, data, 4);

    LCD_WriteCommand(LCD_REG_SLEEP_OUT);
    LCD_WriteCommand(LCD_REG_DISPLAY_ON);
    
    return false;
}

void LCD_WriteAddr(uint16_t addr)
{
    LCD_DATA_PORT->ODR = addr;
    LCD_RST_RS; // Set to command
    LCD_RST_WR; // Set to write
    LCD_SET_WR;
    LCD_SET_RS; // Set to data
}

void LCD_WriteData(uint16_t data)
{
    LCD_DATA_PORT->ODR = data;
    LCD_RST_WR; // Set to write
    LCD_SET_WR;
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
