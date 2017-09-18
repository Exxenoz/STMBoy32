#ifndef LCD_REGDEF_H
#define LCD_REGDEF_H

enum LCD_RegisterAddress
{
    LCD_REG_SOFTWARE_RESET            = 0x01,
    LCD_REG_READ_DISPLAY_INFO         = 0x04,
    LCD_REG_SLEEP_OUT                 = 0x11,
    LCD_REG_GAMMA_SET                 = 0x26,
    LCD_REG_DISPLAY_OFF               = 0x28,
    LCD_REG_DISPLAY_ON                = 0x29,
    LCD_REG_TEARING_EFFECT_LINE_ON    = 0x35,
    LCD_REG_MEMORY_ACCESS_CONTROL     = 0x36,
    LCD_REG_WRITE_CTRL_DISPLAY        = 0x53,
    LCD_REG_COLUMN_ADDRESS_SET        = 0x2A,
    LCD_REG_PAGE_ADDRESS_SET          = 0x2B,
    LCD_REG_MEMORY_WRITE              = 0x2C,
    LCD_REG_PIXEL_FORMAT_SET          = 0x3A,
    LCD_REG_FRAME_RATE_CONTROL        = 0xB1,
    LCD_REG_BLANKING_PORCH_CONTROL    = 0xB5,
    LCD_REG_DISPLAY_FUNCTION_CONTROL  = 0xB6,
    LCD_REG_ENTRY_MODE_SET            = 0xB7,
    LCD_REG_POWER_CONTROL_1           = 0xC0,
    LCD_REG_POWER_CONTROL_2           = 0xC1,
    LCD_REG_VCOM_CONTROL_1            = 0xC5,
    LCD_REG_VCOM_CONTROL_2            = 0xC7,
    LCD_REG_READ_ID4                  = 0xD3,
    LCD_REG_POSITIVE_GAMMA_CORRECTION = 0xE0,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION = 0xE1,
};

typedef union // 0x36 - LCD_REG_MEMORY_ACCESS_CONTROL
{
    struct
    {
        unsigned int                        : 2; // LSB
        unsigned int HorizontalRefreshOrder : 1;
        unsigned int RGBBGROrder            : 1;
        unsigned int VerticalRefreshOrder   : 1;
        unsigned int RowColumnExchange      : 1;
        unsigned int ColumnAddressOrder     : 1;
        unsigned int RowAddressOrder        : 1;
        unsigned int                        : 8; // MSB
    };

    uint16_t Data;
}
MemoryAccessControlData_t;

typedef union // 0x2A - LCD_REG_COLUMN_ADDRESS_SET
{
    struct
    {
        unsigned int StartColumnHigh : 16; // LSB
        unsigned int StartColumnLow  : 16;
        unsigned int EndColumnHigh   : 16;
        unsigned int EndColumnLow    : 16; // MSB
    };

    uint16_t Data[4];
}
ColumnAddressSetData_t;

typedef union // 0x2B - LCD_REG_PAGE_ADDRESS_SET
{
    struct
    {
        unsigned int StartPageHigh : 16; // LSB
        unsigned int StartPageLow  : 16;
        unsigned int EndPageHigh   : 16;
        unsigned int EndPageLow    : 16; // MSB
    };

    uint16_t Data[4];
}
PageAddressSetData_t;

enum
{
    LCD_FRAME_RATE_CONTROL_DATA_DIVISION_RATIO_1 = 0x00,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_61HZ  = 0x1F,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_83HZ  = 0x17,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_95HZ = 0x14,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_100HZ = 0x13,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_106HZ = 0x12,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_112HZ = 0x11,
    LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_119HZ = 0x10,
};

typedef union // 0xB1 - LCD_REG_FRAME_RATE_CONTROL
{
    struct
    {
        unsigned int DivisionRatio :  2; // LSB
        unsigned int               : 14;
        unsigned int FrameRate     :  5;
        unsigned int               : 11; // MSB
    };

    uint16_t Data[2];
}
LCD_FrameRateControlData_t;

typedef union // 0xB6 - LCD_REG_DISPLAY_FUNCTION_CONTROL
{
    struct
    {
        unsigned int                   : 16; // LSB
        unsigned int                   : 7;
        unsigned int LiquidCrystalType : 1;
        unsigned int                   : 8;
        unsigned int                   : 16;
        unsigned int                   : 16; // MSB
    };

    uint16_t Data[4];
}
DisplayFunctionControlData_t;

#endif //LCD_REGDEF_H
