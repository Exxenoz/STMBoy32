#include "ui.h"
#include "cmod.h"

void UI_InitializeMenuPointPadding(LCD_TextDef_t *menuPointDef)
{
    int stringLength = Fonts_GetStringLength(menuPointDef->Characters, menuPointDef->Spacing, &UI_MENU_POINT_FONT);

    menuPointDef->Padding.Left  = (UI_MENU_POINT_LENGTH - stringLength) / 2;
    menuPointDef->Padding.Right = (UI_MENU_POINT_LENGTH - stringLength) / 2;
}

void UI_HightlightMenuPoint(UI_MenuPoint_t menuPoint)
{
    uint16_t bgColor = UI_MENU_POINT_HIGHLIGHTED_BACKGROUND_COLOR;

    // WIP
    // LCD_DrawText(menuPoint.x, menuPoint.y, bgColor, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT); 
}

void UI_DrawMainPage(void)
{
    LCD_TextDef_t menuPointDef;

    // Initialize common attributes of all menu points
    menuPointDef.Spacing = UI_MENU_POINT_SPACING;
    menuPointDef.Color   = UI_MENU_POINT_TEXT_COLOR;
    menuPointDef.Border.Width = UI_MENU_POINT_BORDER_WIDTH;
    menuPointDef.Border.Color = UI_MENU_POINT_BORDER_COLOR;
    menuPointDef.Padding.Upper  = ((UI_MENU_POINT_HEIGHT - UI_MENU_POINT_FONT.FontHeight) / 2);
    menuPointDef.Padding.Lower  = ((UI_MENU_POINT_HEIGHT - UI_MENU_POINT_FONT.FontHeight) / 2) - 6;

    // Print the Page background color
    LCD_ClearColor(UI_MAINPAGE_BACKGROUND_COLOR);

    // Draw the menu points
    menuPointDef.Characters = UI_MENU_POINT_1_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    if (CMOD_Detect())
    {
        LCD_DrawText(29, 35, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointDef, &UI_MENU_POINT_FONT); 
    }
    else
    {
        menuPointDef.Color = UI_MENU_POINT_DISABLED_TEXT_COLOR;
        LCD_DrawText(29, 35, UI_MENU_POINT_DISABLED_BACKGROUND_COLOR, &menuPointDef, &UI_MENU_POINT_FONT); 
        menuPointDef.Color = UI_MENU_POINT_TEXT_COLOR;
    }

    menuPointDef.Characters   = UI_MENU_POINT_2_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    LCD_DrawText(29, 81, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointDef, &UI_MENU_POINT_FONT);

    menuPointDef.Characters   = UI_MENU_POINT_3_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    LCD_DrawText(29, 127, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointDef, &UI_MENU_POINT_FONT);

    menuPointDef.Characters   = UI_MENU_POINT_4_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    LCD_DrawText(29, 173, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointDef, &UI_MENU_POINT_FONT);
}

void UI_DrawShowAll(void)
{
    
}

void UI_DrawShowFav(void)
{
    
}

void UI_DrawOptions(void)
{
    
}
