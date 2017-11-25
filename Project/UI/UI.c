#include "ui.h"
#include "cmod.h"
#include "sdc.h"
#include "ff.h"

UI_MenuPoint_t UI_MainPage_MenuPoints[4];

char* UI_GameEntrys[UI_MAX_NUMBER_OF_GAMES];


void UI_InitializeMenuPointPadding(LCD_TextDef_t *menuPointDef)
{
    int stringLength = Fonts_GetStringLength(menuPointDef->Characters, menuPointDef->Spacing, &UI_MENU_POINT_FONT);

    menuPointDef->Padding.Left  = (UI_MENU_POINT_LENGTH - stringLength) / 2;
    menuPointDef->Padding.Right = (UI_MENU_POINT_LENGTH - stringLength) / 2;
}

void UI_InitializeMainPage(void)
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
    UI_MainPage_MenuPoints[0].X               = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[0].Y               = UI_MAINPAGE_MENU_POINT_1_Y;
    UI_MainPage_MenuPoints[0].MenuPointDef    = menuPointDef;
    UI_MainPage_MenuPoints[0].NewStateOnPress = OS_INGAME_FROM_CARTRIDGE;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_2_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[1].X               = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[1].Y               = UI_MAINPAGE_MENU_POINT_2_Y;
    UI_MainPage_MenuPoints[1].MenuPointDef    = menuPointDef;
    UI_MainPage_MenuPoints[1].NewStateOnPress = OS_SHOW_ALL;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_3_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[2].X               = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[2].Y               = UI_MAINPAGE_MENU_POINT_3_Y;
    UI_MainPage_MenuPoints[2].MenuPointDef    = menuPointDef;
    UI_MainPage_MenuPoints[2].NewStateOnPress = OS_SHOW_FAV;

    menuPointDef.Characters = UI_MAINPAGE_MENU_POINT_4_STRING;
    UI_InitializeMenuPointPadding(&menuPointDef);
    UI_MainPage_MenuPoints[3].X               = UI_MAINPAGE_MENU_POINTS_X;
    UI_MainPage_MenuPoints[3].Y               = UI_MAINPAGE_MENU_POINT_4_Y;
    UI_MainPage_MenuPoints[3].MenuPointDef    = menuPointDef;
    UI_MainPage_MenuPoints[3].NewStateOnPress = OS_OPTIONS;
}

void UI_InitializeShowAll(void)
{
    DIR test; //ToDo: Implement and move
    if (SDC_IsMounted())
    {
        FILINFO fileInfo;                                   // File information structure

        f_readdir(&test, &fileInfo);                        // Get the info of the first file in current directory
        for (int i = 0; fileInfo.fname[0] != NULL; i++)     // Continue reading until all files are read
        {
            UI_GameEntrys[i] = fileInfo.fname;              // Store the file names (equals names of the games)
            f_readdir(&test, &fileInfo);                    // Get next file info (if there is none fname is set 0)
        }
    }
}

void UI_Initialize(void)
{
    Fonts_InitializeSTMFonts();

    UI_InitializeMainPage();
    UI_InitializeShowAll();
}

void UI_DrawMainPage(int firstValidMenuPoint)
{
    // Print the Page background color
    LCD_ClearColor(UI_MAINPAGE_BACKGROUND_COLOR);

    // Draw BOOT CARTRIDGE (either en- or disabled)
    if (firstValidMenuPoint == 0) UI_MainPage_DrawMenuPoint(0);
    else                          UI_MainPage_DrawDisabledMenuPoint(0);

    // Draw the other menu points (enabled)
    UI_MainPage_DrawMenuPoint(1);
    UI_MainPage_DrawMenuPoint(2);
    UI_MainPage_DrawMenuPoint(3);
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

void UI_MainPage_DrawMenuPoint(uint16_t id)
{
    uint16_t color = UI_MENU_POINT_BACKGROUND_COLOR;
    LCD_DrawText(UI_MainPage_MenuPoints[id].X, UI_MainPage_MenuPoints[id].Y, color, &(UI_MainPage_MenuPoints[id].MenuPointDef), &UI_MENU_POINT_FONT);
}

void UI_MainPage_DrawDisabledMenuPoint(uint16_t id)
{
    uint16_t color = UI_DISABLED_MENU_POINT_BACKGROUND_COLOR;

    // Set the menu point color to disabled color, draw it and then restore default value of color
    UI_MainPage_MenuPoints[id].MenuPointDef.Color = UI_DISABLED_MENU_POINT_TEXT_COLOR;

    LCD_DrawText(UI_MainPage_MenuPoints[id].X, UI_MainPage_MenuPoints[id].Y, color, &(UI_MainPage_MenuPoints[id].MenuPointDef), &UI_MENU_POINT_FONT);

    UI_MainPage_MenuPoints[id].MenuPointDef.Color = UI_MENU_POINT_TEXT_COLOR;
}

void UI_MainPage_HightlightMenuPoint(uint16_t id)
{
    uint16_t bgColor = UI_HIGHLIGHTED_MENU_POINT_BACKGROUND_COLOR;

    // Set the menu point color to disabled color, draw it and then restore default value of color
    UI_MainPage_MenuPoints[id].MenuPointDef.Color = UI_HIGHLIGHTED_MENU_POINT_TEXT_COLOR;
    UI_MainPage_MenuPoints[id].MenuPointDef.Border.Color = UI_HIGHLIGHTED_MENU_POINT_BORDER_COLOR;

    LCD_DrawText(UI_MainPage_MenuPoints[id].X, UI_MainPage_MenuPoints[id].Y, bgColor, &(UI_MainPage_MenuPoints[id].MenuPointDef), &UI_MENU_POINT_FONT); 

    UI_MainPage_MenuPoints[id].MenuPointDef.Color = UI_MENU_POINT_TEXT_COLOR;
    UI_MainPage_MenuPoints[id].MenuPointDef.Border.Color = UI_MENU_POINT_BORDER_COLOR;
}
