#ifndef UI_H
#define UI_H

#include "common.h"
#include "ui_fonts.h"
#include "lcd.h"
#include "os.h"

#define UI_MAX_NUMBER_OF_GAMES          2000

#define UI_MAINPAGE_BACKGROUND_COLOR    0xFFC0
#define UI_MENU_POINT_BACKGROUND_COLOR  0xFFFF
#define UI_MENU_POINT_TEXT_COLOR        0x0000
#define UI_MENU_POINT_SPACING           0x0000
#define UI_MENU_POINT_BORDER_COLOR      0x0000
#define UI_MENU_POINT_BORDER_WIDTH      0x0002
#define UI_MENU_POINT_LENGTH            261        // Length of a menu point in pixel
#define UI_MENU_POINT_HEIGHT            32         // Length of a menu point in pixel

#define UI_DISABLED_MENU_POINT_BACKGROUND_COLOR    0xC618
#define UI_DISABLED_MENU_POINT_TEXT_COLOR          0x7BD0

#define UI_HIGHLIGHTED_MENU_POINT_BACKGROUND_COLOR 0x0000  // ToDo: Implement
#define UI_HIGHLIGHTED_MENU_POINT_BORDER_COLOR     0xFFFF  // ToDo: Implement
#define UI_HIGHLIGHTED_MENU_POINT_TEXT_COLOR       0xFFFF  // ToDo: Implement

#define UI_MAINPAGE_MENU_POINT_1_STRING  "BOOT CARTRIDGE"
#define UI_MAINPAGE_MENU_POINT_2_STRING  "SHOW ALL GAMES"
#define UI_MAINPAGE_MENU_POINT_3_STRING  "SHOW FAVORITES"
#define UI_MAINPAGE_MENU_POINT_4_STRING  "OPTIONS"

// Menu Point Coordinates
#define UI_MAINPAGE_MENU_POINTS_X   29
#define UI_MAINPAGE_MENU_POINT_1_Y  35
#define UI_MAINPAGE_MENU_POINT_2_Y  81
#define UI_MAINPAGE_MENU_POINT_3_Y  127
#define UI_MAINPAGE_MENU_POINT_4_Y  173

#define UI_MENU_POINT_FONT  Fonts_STMFont_16x24

typedef struct
{
    uint16_t X;                    // MenuPoint upperLeftCorner x coordinate
    uint16_t Y;                    // MenuPoint upperLeftCorner y coordinate
    LCD_TextDef_t MenuPointDef;    // MenuPoint content
    OS_State_t    NewStateOnPress; // New state after pressing menu point
}
UI_MenuPoint_t;

extern UI_MenuPoint_t UI_MainPage_MenuPoints[4];
extern char* UI_GameEntrys[UI_MAX_NUMBER_OF_GAMES];


void UI_Initialize(void);

void UI_DrawMainPage(int firstValidMenuPoint);
void UI_DrawShowAllPage(void);
void UI_DrawShowFavPage(void);
void UI_DrawOptionsPage(void);

void UI_MainPage_DrawMenuPoint(uint16_t id);
void UI_MainPage_DrawDisabledMenuPoint(uint16_t id);
void UI_MainPage_HightlightMenuPoint(uint16_t id);

#endif // UI_H
