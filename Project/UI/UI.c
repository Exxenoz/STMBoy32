#include "ui.h"
#include "cmod.h"
#include "sdc.h"
#include "ff.h"
#include "string.h"

const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS] = {
    {UI_MAINPAGE_MP_1_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_1_Y, OS_SWITCH_TO_STATE_INGAME_FC},
    {UI_MAINPAGE_MP_2_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_2_Y, OS_SWITCH_TO_STATE_SHOWALL},
    {UI_MAINPAGE_MP_3_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_3_Y, OS_SWITCH_TO_STATE_SHOWFAV},
    {UI_MAINPAGE_MP_4_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_4_Y, OS_SWITCH_TO_STATE_OPTIONS},
};


void UI_InitializeMenuPointPadding(LCD_TextDef_t *menuPointDef)
{
    int stringLength = Fonts_GetStringLength(menuPointDef->Characters, menuPointDef->Spacing, &UI_MP_FONT);

    menuPointDef->Padding.Left  = (UI_MAINPAGE_MP_LENGTH - stringLength) / 2;
    menuPointDef->Padding.Right = (UI_MAINPAGE_MP_LENGTH - stringLength) / 2;
}

void UI_DrawMainPage(int firstValidMenuPoint)
{
    // Print the Page background color
    LCD_ClearColor(UI_MAINPAGE_BG_COLOR);

    // Draw BOOT CARTRIDGE (either en- or disabled)
    if (firstValidMenuPoint == 0) UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_ENABLED);
    else                          UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_DISABLED);

    // Draw the other menu points (enabled)
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[1]), UI_ENABLED);
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[2]), UI_ENABLED);
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[3]), UI_ENABLED);
}

void UI_DrawShowAllPage(void)
{
    // YTBI
}

void UI_DrawShowFavPage(void)
{
    // YTBI
}

void UI_DrawOptionsPage(void)
{
    // YTBI
}

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_MP_Option_t option)
{
    LCD_TextDef_t menuPointDef;

    // Initialize common attributes of all menupoints
    copyString(menuPointDef.Characters, menuPoint->Text, UI_MAX_MP_LENGTH);
    menuPointDef.Spacing       = UI_MP_SPACING;
    menuPointDef.Border.Width  = UI_MP_BORDER_WIDTH;
    menuPointDef.Padding.Upper = ((UI_MAINPAGE_MP_HEIGHT - UI_MP_FONT.FontHeight) / 2);
    menuPointDef.Padding.Lower = ((UI_MAINPAGE_MP_HEIGHT - UI_MP_FONT.FontHeight) / 2) - 4;

    // Initialize UI_MainPage_MenuPoints
    UI_InitializeMenuPointPadding(&menuPointDef);

    switch(option)
    {
        case UI_ENABLED:
            menuPointDef.Color = UI_MP_TEXT_COLOR;
            menuPointDef.Border.Color = UI_MP_BORDER_COLOR;
            LCD_DrawText(menuPoint->X, menuPoint->Y, UI_MP_BG_COLOR, &menuPointDef, &UI_MP_FONT);
            break;
        case UI_DISABLED:
            menuPointDef.Color = UI_DISABLED_MP_TEXT_COLOR;
            menuPointDef.Border.Color = UI_MP_BORDER_COLOR;
            LCD_DrawText(menuPoint->X, menuPoint->Y, UI_DISABLED_MP_BG_COLOR, &menuPointDef, &UI_MP_FONT);
            break;
        case UI_HIGHLIGHTED:
            menuPointDef.Color = UI_HIGHLIGHTED_MP_TEXT_COLOR;
            menuPointDef.Border.Color = UI_HIGHLIGHTED_MP_BORDER_COLOR;
            LCD_DrawText(menuPoint->X, menuPoint->Y, UI_HIGHLIGHTED_MP_BG_COLOR, &menuPointDef, &UI_MP_FONT);
            break;
        default:
            break;
    }
}

// Compiler workaround functions

void UI_DrawMainPageMenuPoint(int id, UI_MP_Option_t option)
{
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[id]), option);
}

void UI_PerformMainPageAction(int id)
{
    OS_DoAction(UI_MainPage_MenuPoints[id].Action);
}
