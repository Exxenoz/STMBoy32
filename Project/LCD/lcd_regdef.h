#ifndef LCD_REGDEF_H
#define LCD_REGDEF_H

enum LCD_RegisterAddresses_e // (Commands)
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

enum LCD_Parameters_e  // Some commands expect data that consists of multiple parameters
{
    LCD_REG_VCOM_CONTROL_PARAM1               = 0x35, // VCOMH =  4,025
    LCD_REG_VCOM_CONTROL_PARAM2               = 0x3E, // VCOML = -0,950

    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM1   = 0x0A, // Interval Scan mode, Source&Vcom +&- polarity = AGND
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM2   = 0x82, // LC-Type = white, Scan direction = G1->G320 (even/odd parallel), Shift direction = S1->S720, Scan cycle = 5 frames
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM3   = 0x27, // Lines to drive LCD at interval of 8 lines = 320
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM4   = 0x00, // Apparently of no use at all

// Gray scale voltages to adjust the gamma characteristics of the TFT panel
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM1  = 0x1f,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM2  = 0x1a,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM3  = 0x18,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM4  = 0x0a,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM5  = 0x0f,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM6  = 0x06,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM7  = 0x45,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM8  = 0x87,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM9  = 0x32,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM10 = 0x0a,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM11 = 0x07,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM12 = 0x02,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM13 = 0x07,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM14 = 0x05,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM15 = 0x00,

    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM1  = 0x00,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM2  = 0x25,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM3  = 0x27,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM4  = 0x05,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM5  = 0x10,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM6  = 0x09,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM7  = 0x3A,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM8  = 0x78,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM9  = 0x4D,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM10 = 0x05,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM11 = 0x18,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM12 = 0x0D,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM13 = 0x38,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM14 = 0x3A,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM15 = 0x1F,
};

uint16_t LCD_REG_VCOM_CONTROL_DATA[2] =
{
    LCD_REG_VCOM_CONTROL_PARAM1,
    LCD_REG_VCOM_CONTROL_PARAM2,
};

uint16_t LCD_REG_DISPLAY_FUNCTION_CONTROL_DATA[4] =
{
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM1,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM2,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM3,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM4,
};

uint16_t LCD_REG_POSITIVE_GAMMA_CORRECTION_DATA[15] =
{
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM1,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM2,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM3,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM4,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM5,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM6,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM7,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM8,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM9,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM10,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM11,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM12,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM13,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM14,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM15,
};

uint16_t LCD_REG_NEGATIVE_GAMMA_CORRECTION_DATA[15] =
{
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM1,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM2,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM3,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM4,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM5,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM6,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM7,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM8,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM9,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM10,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM11,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM12,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM13,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM14,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM15,
};

enum LCD_CommandData_e
{
    LCD_REG_POWER_CONTROL1_DATA = 0x09, // GVDD = 3,3V
    LCD_REG_POWER_CONTROL2_DATA = 0x13, // Stepup Factor => AVDD = 2VCI, VGH = 6VCI, VGL = -3VCI
    LCD_REG_VCOM_CONTROL2_DATA  = 0xBE, // Offset => VCOMH = VMH - 2, VCOML = VML - 2
    LCD_REG_PIXEL_FORMAT_DATA   = 0x55, // RGB & MCU Interface Format = 16 bit / pixel
    LCD_REG_ENTRY_MODE_DATA     = 0x07, // DeepStandby off, Low Voltage detection off, G1~G320 = normal display   
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
