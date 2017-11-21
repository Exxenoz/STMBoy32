#include "ui.h"
#include "cmod.h"

void UI_InitializeMenuPointSpacing(LCD_Text_t *menuPointText)
{
    int stringLength = Fonts_GetStringLength(menuPointText->textString, menuPointText->textSpacing, &UI_MENU_POINT_FONT);

    menuPointText->spacingLeft  = (261 - stringLength) / 2;
    menuPointText->spacingRight = (261 - stringLength) / 2;
}

void UI_HightlightMenuPoint(UI_MenuPoint_t menuPoint)
{
    uint16_t bgColor = UI_MENU_POINT_HIGHLIGHTED_BACKGROUND_COLOR;

    // WIP
    // LCD_DrawText(menuPoint.x, menuPoint.y, bgColor, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT); 
}

void UI_DrawMainPage(void)
{
    LCD_Text_t   menuPointText;
    LCD_Border_t menuPointBorder;

    menuPointText.textColor   = UI_MENU_POINT_TEXT_COLOR;
    menuPointText.spacingTop  = ((32 - UI_MENU_POINT_FONT.FontHeight) / 2);
    menuPointText.spacingBot  = ((32 - UI_MENU_POINT_FONT.FontHeight) / 2) - 6;
    menuPointText.textSpacing = 0;

    menuPointBorder.width = UI_MENU_POINT_BORDER_WIDTH;
    menuPointBorder.color = UI_MENU_POINT_BORDER_COLOR;

    LCD_ClearColor(UI_MAINPAGE_BACKGROUND_COLOR);


    menuPointText.textString   = "BOOT CARTRIDGE";
    UI_InitializeMenuPointSpacing(&menuPointText);
    if (CMOD_Detect())
    {
        LCD_DrawText(29, 35, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT); 
    }
    else
    {
        menuPointText.textColor = UI_MENU_POINT_DISABLED_TEXT_COLOR;
        LCD_DrawText(29, 35, UI_MENU_POINT_DISABLED_BACKGROUND_COLOR, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT); 
        menuPointText.textColor = UI_MENU_POINT_TEXT_COLOR;
    }

    menuPointText.textString   = "SHOW ALL GAMES";
    UI_InitializeMenuPointSpacing(&menuPointText);
    LCD_DrawText(29, 81, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT);

    menuPointText.textString   = "SHOW FAVORITES";
    UI_InitializeMenuPointSpacing(&menuPointText);
    LCD_DrawText(29, 127, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT);

    menuPointText.textString   = "OPTIONS";
    UI_InitializeMenuPointSpacing(&menuPointText);
    LCD_DrawText(29, 173, UI_MENU_POINT_BACKGROUND_COLOR, &menuPointBorder, &menuPointText, &UI_MENU_POINT_FONT);
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
