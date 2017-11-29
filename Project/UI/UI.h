#ifndef UI_H
#define UI_H

#include "common.h"
#include "ui_fonts.h"
#include "lcd.h"
#include "os.h"

// Max values
#define UI_NUMBER_OF_MAINPAGE_MPS 4                 // Total number of MainPage menupoints
#define UI_MAX_MP_LENGTH          15                // Max char length of a menupoint

// Page Backgrounds
#define UI_MAINPAGE_BG_COLOR      0xFFC0
#define UI_OPTIONS_BG_COLOR       0xFFC0

// Scrollbar specifications
#define UI_SCROLLBAR_WIDTH        15
#define UI_SCROLLBAR_BG_COLOR     0xFFFF
#define UI_SCROLLBAR_FG_COLOR     0x0000

// Brickwall specifications (measurements in pixel)
#define UI_WALL_WIDTH1            66
#define UI_WALL_WIDTH2            33
#define UI_BRICK_COLOR            0xFFFF
#define UI_BRICK_LENGTH           40
#define UI_BRICK_HEIGHT           15
#define UI_BRICK_BORDER_COLOR     0x0000
#define UI_BRICK_BORDER_WIDTH     1

// General measurements (in pixel)
#define UI_MAINPAGE_MP_LENGTH     265
#define UI_MAINPAGE_MP_HEIGHT     36
#define UI_OPTIONS_MP_LENGTH      0
#define UI_OPTIONS_MP_HEIGHT      0
#define UI_GE_LENGTH1             (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_WIDTH1)
#define UI_GE_LENGTH2             (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_WIDTH2 - UI_SCROLLBAR_WIDTH)
#define UI_GE_HEIGHT              34

// Mainpage menupoint texts
#define UI_MAINPAGE_MP_1_STRING   "BOOT CARTRIDGE"
#define UI_MAINPAGE_MP_2_STRING   "SHOW ALL GAMES"
#define UI_MAINPAGE_MP_3_STRING   "SHOW FAVORITES"
#define UI_MAINPAGE_MP_4_STRING   "OPTIONS"

// Mainpage menupoint coordinates (in pixel)
#define UI_MAINPAGE_MPS_X   29
#define UI_MAINPAGE_MP_1_Y  35
#define UI_MAINPAGE_MP_2_Y  81
#define UI_MAINPAGE_MP_3_Y  127
#define UI_MAINPAGE_MP_4_Y  173 

// General menupoint specifications (measurements in pixel)
#define UI_MP_SPACING       0
#define UI_MP_BORDER_WIDTH  2
#define UI_MP_BG_COLOR      0xFFFF
#define UI_MP_TEXT_COLOR    0x0000
#define UI_MP_BORDER_COLOR  0x0000

#define UI_DISABLED_MP_BG_COLOR         0xC618
#define UI_DISABLED_MP_TEXT_COLOR       0x7BD0
#define UI_HIGHLIGHTED_MP_BG_COLOR      0x0000
#define UI_HIGHLIGHTED_MP_TEXT_COLOR    0xFFFF
#define UI_HIGHLIGHTED_MP_BORDER_COLOR  0xFFFF

// Game-entry specifications (measurements in pixel)
#define UI_GE_SPACING       0
#define UI_GE_BORDER_WIDTH  2
#define UI_GE_BG_COLOR      0xFFFF
#define UI_GE_TEXT_COLOR    0x0000
#define UI_GE_BORDER_COLOR  0x0000

#define UI_GE_STAR_X        300
#define UI_GE_STAR_OFFSET_Y 4

#define UI_GE_STAR_COLOR                0x0000
#define UI_DISABLED_GE_BG_COLOR         0xC618
#define UI_DISABLED_GE_TEXT_COLOR       0x7BD0
#define UI_HIGHLIGHTED_GE_BG_COLOR      0x0000
#define UI_HIGHLIGHTED_GE_STAR_COLOR    0xFFFF
#define UI_HIGHLIGHTED_GE_TEXT_COLOR    0xFFFF
#define UI_HIGHLIGHTED_GE_BORDER_COLOR  0xFFFF

// Fonts
#define UI_MP_FONT Fonts_STMFont_16x24
#define UI_GE_FONT Fonts_STMFont_16x24

typedef enum
{
    UI_ENABLED,
    UI_DISABLED,
    UI_HIGHLIGHTED
}
UI_DrawOption_t;

typedef enum
{
    UI_SCROLLUP,
    UI_SCROLLDOWN,
}
UI_ScrollOption_t;

typedef struct
{
    char        Text[UI_MAX_MP_LENGTH + 1];  // MenuPoint Text
    uint16_t    X;                       // MenuPoint upperLeftCorner x coordinate
    uint16_t    Y;                       // MenuPoint upperLeftCorner y coordinate
    uint16_t    Height;                  // MenuPoint height
    uint16_t    Length;                  // MenuPoint length
    OS_Action_t Action;                  // Action to be performed when selection is confirmed
}
UI_MenuPoint_t;

extern const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS];


void UI_DrawMainPage(int firstValidMenuPoint);
bool UI_DrawShowAllPage(void);
void UI_DrawShowFavPage(void);
void UI_DrawOptionsPage(void);

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_DrawOption_t option);
void UI_DrawGameEntry(uint16_t x, uint16_t y, OS_GameEntry_t *gameEntry, UI_DrawOption_t option, bool lastPlayed);
void UI_DrawScrollBar(int currentGameEntry);
bool UI_ScrollGames(int currGE, int firstDisplayedGE, UI_ScrollOption_t option);

// Compiler workaround functions
void UI_DrawMainPageMenuPoint(int id, UI_DrawOption_t option);
void UI_PerformMainPageAction(int id);

#endif // UI_H
