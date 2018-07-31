#ifndef UI_H
#define UI_H

#include "os.h"
#include "common.h"
#include "ui_fonts.h"
#include "lcd_drawing.h"


// Switch brick design on/off
#define SICK_BRICK_DESIGN         true
  
// Total number of menupoints
#define UI_NUMBER_OF_MAINPAGE_MPS 3
#define UI_NUMBER_OF_SHOWALL_MPS  3

// Max char length of a menupoint
#define UI_MAX_MP_LENGTH          15

//Number of gameentries (GEs) fitting the display
#define UI_LIST_LENGTH            ((int)((LCD_DISPLAY_SIZE_Y - UI_SHOWALL_MP_HEIGHT) / UI_SHOWALL_GE_HEIGHT))

// Spacing between the Showall page menupoint and the list displaying the gameentries
#define UI_UPPER_LIST_PADDING     (UI_SHOWALL_GE_LIST_Y - UI_SHOWALL_MP_HEIGHT)

// Page Backgrounds
#define UI_MAINPAGE_BG_COLOR      0xFFC0
#define UI_OPTIONS_BG_COLOR       0xFFC0

// Scrollbar specifications
#define UI_SCROLLBAR_WIDTH        15
#define UI_SCROLLBAR_BG_COLOR     0xBBEB
#define UI_SCROLLBAR_FG_COLOR     0x0000

// Brickwall specifications (measurements in pixel)
#define UI_WALL_1_WIDTH           66
#define UI_WALL_2_WIDTH           33
#define UI_WALL_1_HEIGHT          UI_SHOWALL_MP_HEIGHT
#define UI_WALL_2_HEIGHT          (LCD_DISPLAY_SIZE_Y - UI_WALL_1_HEIGHT)
#define UI_BRICK_COLOR            0xFFFF
#define UI_BRICK_LENGTH           10
#define UI_BRICK_HEIGHT           4
#define UI_BRICK_BORDER_COLOR     0x0000
#define UI_BRICK_BORDER_WIDTH     1

// General measurements (in pixel)
#define UI_MAINPAGE_MP_LENGTH     265
#define UI_MAINPAGE_MP_HEIGHT     36
#define UI_OPTIONS_MP_LENGTH      0
#define UI_OPTIONS_MP_HEIGHT      0
#define UI_SHOWALL_MP_LENGTH      (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_1_WIDTH)
#define UI_SHOWALL_MP_HEIGHT      34
#define UI_SHOWALL_GE_LENGTH      (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_2_WIDTH - UI_SCROLLBAR_WIDTH)
#define UI_SHOWALL_GE_HEIGHT      34

// Mainpage menupoint texts
#define UI_MAINPAGE_MP_1_STRING   "BOOT CARTRIDGE"
#define UI_MAINPAGE_MP_2_STRING   "SHOW ALL GAMES"
#define UI_MAINPAGE_MP_3_STRING   "OPTIONS"

// ShowAllpage menupoint texts
#define UI_SHOWALL_MP_1_STRING   "|ALL GAMES|"
#define UI_SHOWALL_MP_2_STRING   "|FAVORITES|"
#define UI_SHOWALL_MP_3_STRING   "|LAST PLAYED|"

// Mainpage menupoint coordinates (in pixel)
#define UI_MAINPAGE_MP_SPACING 15
#define UI_MAINPAGE_MPS_X      29
#define UI_MAINPAGE_MP_1_Y     53
#define UI_MAINPAGE_MP_2_Y     UI_MAINPAGE_MP_1_Y + UI_MAINPAGE_MP_HEIGHT + UI_MAINPAGE_MP_SPACING
#define UI_MAINPAGE_MP_3_Y     UI_MAINPAGE_MP_2_Y + UI_MAINPAGE_MP_HEIGHT + UI_MAINPAGE_MP_SPACING

// ShowAllPage menupoint coordinates (in pixel)
#define UI_SHOWALL_MPS_X  UI_WALL_1_WIDTH
#define UI_SHOWALL_MPS_Y  0

// ShowAllpage List coordinates
#define UI_SHOWALL_GE_LIST_X  UI_WALL_2_WIDTH
#define UI_SHOWALL_GE_LIST_Y  (LCD_DISPLAY_SIZE_Y - UI_LIST_LENGTH * UI_SHOWALL_GE_HEIGHT)

// General menupoint specifications (measurements in pixel)
#define UI_MP_SPACING       				    0
#define UI_MP_BORDER_WIDTH  				    0
#define UI_MP_BG_COLOR      				    0xFFFF
#define UI_MP_TEXT_COLOR    				    0x0000
#define UI_MP_BORDER_COLOR  				    0x0000

#define UI_DISABLED_MP_BG_COLOR         0xC618
#define UI_DISABLED_MP_TEXT_COLOR       0x7BD0
#define UI_HIGHLIGHTED_MP_BG_COLOR      0x0000
#define UI_HIGHLIGHTED_MP_TEXT_COLOR    0xFFFF
#define UI_HIGHLIGHTED_MP_BORDER_COLOR  0xFFFF

// Game-entry specifications (measurements in pixel)
#define UI_GE_SPACING       					  0
#define UI_GE_BORDER_WIDTH  					  0
#define UI_GE_BG_COLOR      					  0xFFFF
#define UI_GE_TEXT_COLOR    					  0x0000
#define UI_GE_BORDER_COLOR  					  0x0000

#define UI_GE_STAR_X        (LCD_DISPLAY_SIZE_X - UI_WALL_2_WIDTH - UI_SCROLLBAR_WIDTH - 35)
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
    UI_ALLGAMES,
    UI_FAVORITES,
    UI_LASTPLAYED,
}
UI_ShowAllDesign_t;

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

#pragma pack(1)
typedef struct
{
    char        Text[UI_MAX_MP_LENGTH + 1];  // MenuPoint Text
    uint16_t    X;                           // MenuPoint upperLeftCorner x coordinate
    uint16_t    Y;                           // MenuPoint upperLeftCorner y coordinate
    uint16_t    Height;                      // MenuPoint height
    uint16_t    Length;                      // MenuPoint length
    OS_Action_t Action;                      // Action to be performed when selection is confirmed
}
UI_MenuPoint_t;


extern const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS];



void UI_DrawMainPage(int firstValidMenuPoint);
void UI_DrawShowAllPage(UI_ShowAllDesign_t design);
void UI_DrawOptionsPage(void);

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_DrawOption_t option);
void UI_DrawGameEntry(uint16_t x, uint16_t y, OS_GameEntry_t *gameEntry, UI_DrawOption_t option);
void UI_ReDrawCurrGE(int currGEIndex, int currGEListID);
void UI_ReDrawGEList(int currGEIndex, int currGEListID);
void UI_DrawScrollBar(int currGameEntry);
void UI_ScrollGames(int *p_currGEIndex, int *p_currGEListID, UI_ScrollOption_t option);

#endif // UI_H
