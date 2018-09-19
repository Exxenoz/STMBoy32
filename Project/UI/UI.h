#ifndef UI_H
#define UI_H

#include "os.h"
#include "ui_config.h"


typedef enum
{
    UI_ALLGAMES,
    UI_FAVORITES,
    UI_LASTPLAYED,
}
UI_ShowAllTabs_t;

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



void UI_ShowLanguage(void);
void UI_ShowBrightness(void);
void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_DrawOption_t option);
void UI_DrawGameEntry(uint16_t x, uint16_t y, OS_GameEntry_t *gameEntry, UI_DrawOption_t option);
void UI_ReDrawCurrGE(int currGEIndex, int currGEListID);
void UI_ReDrawGEList(int currGEIndex, int currGEListID);
void UI_DrawScrollBar(int currGameEntry);
void UI_ScrollGames(int *p_currGEIndex, int *p_currGEListID, UI_ScrollOption_t option);

#endif // UI_H
