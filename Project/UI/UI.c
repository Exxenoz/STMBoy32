#include "ui.h"
#include "cmod.h"

UI_MenuPoint_t UI_MainPage_MenuPoints[4];


void UI_InitializeMenuPointPadding(LCD_TextDef_t *menuPointDef)
{
    int stringLength = Fonts_GetStringLength(menuPointDef->Characters, menuPointDef->Spacing, &UI_MENU_POINT_FONT);

    menuPointDef->Padding.Left  = (UI_MENU_POINT_LENGTH - stringLength) / 2;
    menuPointDef->Padding.Right = (UI_MENU_POINT_LENGTH - stringLength) / 2;
}

void UI_Initialize(void)
{
    LCD_TextDef_t menuPointDef;

    // Initialize common attributes of all menu points
    menuPointDef.Color          = UI_MENU_POINT_TEXT_COLOR;
    menuPointDef.Spacing        = UI_MENU_POINT_SPACING;
    menuPointDef.Border.Width   = UI_MENU_POINT_BORDER_WIDTH;
    menuPointDef.Border.Color   = UI_MENU_POINT_BORDER_COLOR;
    menuPointDef.Padding.Upper  = ((UI_MENU_POINT_HEIGHT - UI_MENU_POINT_FONT.FontHeight) / 2);
    menuPointDef.Padding.Lower  = ((UI_MENU_POINT_HEIGHT - UI_MENU_POINT_FONT.FontHeight) / 2) - 6;

    // Initialize UI_MainPage_MenuPoints
    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_1_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[0].menuPointDef = menuPointDef;
    UI_MainPage_MenuPoints[0].x = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[0].y = UI_MAINPAGE_MENU_POINT_1_Y;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_2_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[1].menuPointDef = menuPointDef;
    UI_MainPage_MenuPoints[1].x = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[1].y = UI_MAINPAGE_MENU_POINT_2_Y;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_3_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[2].menuPointDef = menuPointDef;
    UI_MainPage_MenuPoints[2].x = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[2].y = UI_MAINPAGE_MENU_POINT_3_Y;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_4_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[3].menuPointDef = menuPointDef;
    UI_MainPage_MenuPoints[3].x = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[3].y = UI_MAINPAGE_MENU_POINT_4_Y;
}

void UI_DrawMenuPoint(UI_MenuPoint_t *menuPoint)
{
    uint16_t color = UI_MENU_POINT_BACKGROUND_COLOR;
    LCD_DrawText(menuPoint->x, menuPoint->y, color, &(menuPoint->menuPointDef), &UI_MENU_POINT_FONT);
}

void UI_DrawDisabledMenuPoint(UI_MenuPoint_t *menuPoint)
{
    uint16_t color = UI_DISABLED_MENU_POINT_BACKGROUND_COLOR;

    menuPoint->menuPointDef.Color = UI_DISABLED_MENU_POINT_TEXT_COLOR;
    LCD_DrawText(menuPoint->x, menuPoint->y, color, &(menuPoint->menuPointDef), &UI_MENU_POINT_FONT);
    menuPoint->menuPointDef.Color = UI_MENU_POINT_TEXT_COLOR;
}

void UI_HightlightMenuPoint(UI_MenuPoint_t *menuPoint)
{
    uint16_t bgColor = UI_HIGHLIGHTED_MENU_POINT_BACKGROUND_COLOR;
    menuPoint->menuPointDef.Color = UI_HIGHLIGHTED_MENU_POINT_TEXT_COLOR;
    menuPoint->menuPointDef.Border.Color = UI_HIGHLIGHTED_MENU_POINT_BORDER_COLOR;
    LCD_DrawText(menuPoint->x, menuPoint->y, bgColor, &(menuPoint->menuPointDef), &UI_MENU_POINT_FONT); 
}

void UI_DrawMainPage(void)
{
    // Print the Page background color
    LCD_ClearColor(UI_MAINPAGE_BACKGROUND_COLOR);

    // Draw the menu points, if no cartridge is detected draw BOOT CARTRIDGE in Disabled-Colors
    if (CMOD_Detect()) UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]));
    else               UI_DrawDisabledMenuPoint(&(UI_MainPage_MenuPoints[0]));

    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[1]));
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[2]));
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[3]));
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
