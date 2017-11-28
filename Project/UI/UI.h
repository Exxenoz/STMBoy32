#ifndef UI_H
#define UI_H

#include "common.h"
#include "ui_fonts.h"
#include "lcd.h"
#include "os.h"

#define UI_NUMBER_OF_MAINPAGE_MPS 4       // Total number of MainPage menupoints
#define UI_MAX_MP_LENGTH          15      // Max char length of a menupoint

// Page Backgrounds
#define UI_MAINPAGE_BG_COLOR      0xFFC0
#define UI_SHOWALL_BG_COLOR       0x0000   // YTBI
#define UI_FAVORITES_BG_COLOR     0x0000   // YTBI
#define UI_OPTIONS_BG_COLOR       0x0000   // YTBI

// Menupoint Measurements
#define UI_MAINPAGE_MP_LENGTH     261      // Length of a Mainpage menupoint in pixel
#define UI_MAINPAGE_MP_HEIGHT     32       // Length of a Mainpage menupoint in pixel
#define UI_GAME_ENTRY_LENGTH      0        // YTBI
#define UI_GAME_ENTRY_HEIGHT      0        // YTBI
#define UI_OPTIONS_MP_LENGTH      0        // YTBI
#define UI_OPTIONS_MP_HEIGHT      0        // YTBI

// Menupoint Texts
#define UI_MAINPAGE_MP_1_STRING   "BOOT CARTRIDGE"
#define UI_MAINPAGE_MP_2_STRING   "SHOW ALL GAMES"
#define UI_MAINPAGE_MP_3_STRING   "SHOW FAVORITES"
#define UI_MAINPAGE_MP_4_STRING   "OPTIONS"

// Menupoint Coordinates
#define UI_MAINPAGE_MPS_X   29
#define UI_MAINPAGE_MP_1_Y  35
#define UI_MAINPAGE_MP_2_Y  81
#define UI_MAINPAGE_MP_3_Y  127
#define UI_MAINPAGE_MP_4_Y  173

// MenuPoint Colors
#define UI_MP_BG_COLOR      0xFFFF
#define UI_MP_TEXT_COLOR    0x0000
#define UI_MP_BORDER_COLOR  0x0000
#define UI_MP_BORDER_WIDTH  0x0002
#define UI_MP_SPACING       0x0000

#define UI_DISABLED_MP_BG_COLOR         0xC618
#define UI_DISABLED_MP_TEXT_COLOR       0x7BD0
#define UI_HIGHLIGHTED_MP_BG_COLOR      0x0000  // YTBI
#define UI_HIGHLIGHTED_MP_TEXT_COLOR    0xFFFF  // YTBI
#define UI_HIGHLIGHTED_MP_BORDER_COLOR  0xFFFF  // YTBI

// Fonts
#define UI_MP_FONT Fonts_STMFont_16x24

typedef enum
{
    UI_ENABLED,
    UI_DISABLED,
    UI_HIGHLIGHTED
}
UI_Option_t;

typedef struct
{
    char        Text[UI_MAX_MP_LENGTH];  // MenuPoint Text
    uint16_t    X;                       // MenuPoint upperLeftCorner x coordinate
    uint16_t    Y;                       // MenuPoint upperLeftCorner y coordinate
    uint16_t    Height;                  // MenuPoint height
    uint16_t    Length;                  // MenuPoint length
    OS_Action_t Action;                  // Action to be performed when selection is confirmed
}
UI_MenuPoint_t;

extern const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS];


void UI_DrawMainPage(int firstValidMenuPoint);
void UI_DrawShowAllPage(void);
void UI_DrawShowFavPage(void);
void UI_DrawOptionsPage(void);

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_Option_t option);

// Compiler workaround functions
void UI_DrawMainPageMenuPoint(int id, UI_Option_t option);
void UI_PerformMainPageAction(int id);

#endif // UI_H
