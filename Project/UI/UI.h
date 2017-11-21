#ifndef UI_H
#define UI_H

#include "common.h"
#include "ui_fonts.h"
#include "lcd.h"

#define UI_MAINPAGE_BACKGROUND_COLOR    0xFFC0
#define UI_MENU_POINT_BACKGROUND_COLOR  0xFFFF
#define UI_MENU_POINT_TEXT_COLOR        0x0000
#define UI_MENU_POINT_SPACING           0x0000
#define UI_MENU_POINT_BORDER_COLOR      0x0000
#define UI_MENU_POINT_BORDER_WIDTH      0x0002
#define UI_MENU_POINT_LENGTH            261        // Length of a menu point in pixel
#define UI_MENU_POINT_HEIGHT            32         // Length of a menu point in pixel

#define UI_MENU_POINT_DISABLED_BACKGROUND_COLOR    0xC618
#define UI_MENU_POINT_DISABLED_TEXT_COLOR          0x7BD0

#define UI_MENU_POINT_HIGHLIGHTED_BACKGROUND_COLOR 0x0000 // ToDo: Implement
#define UI_MENU_POINT_HIGHLIGHTED_TEXT_COLOR       0x0000 // ToDo: Implement

#define UI_MENU_POINT_1_STRING "BOOT CARTRIDGE"
#define UI_MENU_POINT_2_STRING "SHOW ALL GAMES"
#define UI_MENU_POINT_3_STRING "SHOW FAVORITES"
#define UI_MENU_POINT_4_STRING "OPTIONS"

#define UI_MENU_POINT_FONT  Fonts_STMFont_16x24

typedef struct
{
    int ID;      // ToDo: Change to MenuPoint text
    uint16_t x;  // MenuPoint upperLeftCorner x coordinate
    uint16_t y;  // MenuPoint upperLeftCorner y coordinate
}
UI_MenuPoint_t;

void UI_HightlightMenuPoint(UI_MenuPoint_t menuPoint);

void UI_DrawMainPage(void);
void UI_DrawShowAll(void);
void UI_DrawShowFav(void);
void UI_DrawOptions(void);

#endif // UI_H
