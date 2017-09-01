#ifndef LCD_REGDEF_H
#define LCD_REGDEF_H

enum LCD_RegisterAddress
{
    LCD_REG_SOFTWARE_RESET     = 0x01,
    LCD_REG_READ_DISPLAY_INFO  = 0x04,
    LCD_REG_SET_COLUMN_ADDRESS = 0x2A,
    LCD_REG_SET_ROW_ADDRESS    = 0x2B,
    LCD_REG_WRITE_CTRL_DISPLAY = 0x53,
    LCD_REG_READ_ID4           = 0xD3
};

#endif //LCD_REGDEF_H
