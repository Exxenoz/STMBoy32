#include "ui.h"
#include "ui_pages.h"

#include "lcd_types.h"
#include "lcd_drawing.h"


const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS] =
{
    {UI_MAINPAGE_MP_1_STRING, UI_MAINPAGE_MPS_X,     UI_MAINPAGE_MP_1_Y,
     UI_MAINPAGE_MP_HEIGHT,   UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_INGAME_FC},

    {UI_MAINPAGE_MP_2_STRING, UI_MAINPAGE_MPS_X,     UI_MAINPAGE_MP_2_Y,
     UI_MAINPAGE_MP_HEIGHT,   UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_SHOWALL},

    {UI_MAINPAGE_MP_3_STRING, UI_MAINPAGE_MPS_X,     UI_MAINPAGE_MP_3_Y,
     UI_MAINPAGE_MP_HEIGHT,   UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_OPTIONS},
};

const UI_MenuPoint_t UI_ShowAll_MenuPoints[UI_NUMBER_OF_SHOWALL_MPS] =
{
    {UI_SHOWALL_MP_1_STRING, UI_SHOWALL_MPS_X,     UI_SHOWALL_MPS_Y,
     UI_SHOWALL_MP_HEIGHT,   UI_SHOWALL_MP_LENGTH, OS_NO_ACTION},

    {UI_SHOWALL_MP_2_STRING, UI_SHOWALL_MPS_X,     UI_SHOWALL_MPS_Y,
     UI_SHOWALL_MP_HEIGHT,   UI_SHOWALL_MP_LENGTH, OS_NO_ACTION},

    {UI_SHOWALL_MP_3_STRING, UI_SHOWALL_MPS_X,     UI_SHOWALL_MPS_Y,
     UI_SHOWALL_MP_HEIGHT,   UI_SHOWALL_MP_LENGTH, OS_NO_ACTION},
};

UI_MenuPoint_t UI_Options_MenuPoints[UI_NUMBER_OF_OPTIONS_MPS] =
{
    {UI_OPTIONS_MP_1_STRING, UI_OPTIONS_MP_1_X,           UI_OPTIONS_MP_1_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_SHORT,  OS_EDIT_LANGUAGE},

    {UI_OPTIONS_MP_2_STRING, UI_OPTIONS_MP_2_X,           UI_OPTIONS_MP_2_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_SHORT,  OS_NO_ACTION},

    {UI_OPTIONS_MP_3_STRING, UI_OPTIONS_MP_3_X,           UI_OPTIONS_MP_3_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_MEDIUM, OS_EDIT_BRIGHTNESS},

    {UI_OPTIONS_MP_4_STRING, UI_OPTIONS_MP_4_X,           UI_OPTIONS_MP_4_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_MEDIUM, OS_TOGGLE_OPTION_SCALING},

    {UI_OPTIONS_MP_5_STRING, UI_OPTIONS_MP_5_X,           UI_OPTIONS_MP_5_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_MEDIUM, OS_TOGGLE_OPTION_DIRECT_BOOT},

    {UI_OPTIONS_MP_6_STRING, UI_OPTIONS_MP_6_X,           UI_OPTIONS_MP_6_Y,
     UI_OPTIONS_MP_HEIGHT,   UI_OPTIONS_MP_LENGTH_LONG,   OS_SAVE_CARTRIDGE},
};

const UI_MenuPoint_t UI_Options_alternate_MP_6 =     
{
    UI_OPTIONS_ALTERNATE_MP_6_STRING,
    UI_OPTIONS_ALTERNATE_MP_6_X,
    UI_OPTIONS_MP_6_Y,
    UI_OPTIONS_MP_HEIGHT,
    UI_OPTIONS_MP_LENGTH_SHORT,
    OS_SWITCH_TO_STATE_MAINPAGE
};

const UI_MenuPoint_t UI_Options_regular_MP_6 =     
{
    UI_OPTIONS_MP_6_STRING,
    UI_OPTIONS_MP_6_X,
    UI_OPTIONS_MP_6_Y,
    UI_OPTIONS_MP_HEIGHT,
    UI_OPTIONS_MP_LENGTH_LONG,
    OS_SAVE_CARTRIDGE
};



void UI_DrawMainPage(int selectedMP, bool isCartridgeInserted)
{
    // Draw the Page background.
    if (SICK_BRICK_DESIGN)
    {
        LCD_Brick_t brick;
        brick.Color        = UI_BRICK_COLOR;
        brick.Height       = UI_BRICK_HEIGHT;
        brick.Length       = UI_BRICK_LENGTH;
        brick.Border.Color = UI_BRICK_BORDER_COLOR;
        brick.Border.Width = UI_BRICK_BORDER_WIDTH;

        LCD_DrawWall(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y, false, &brick);
    }
    else
    {
        LCD_ClearColor(UI_MAINPAGE_BG_COLOR);
    }

    // Draw BOOT CARTRIDGE either heighlighted or disabled.
    if   (isCartridgeInserted == true) UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_ENABLED);
    else                               UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_DISABLED);

    // Draw the other menu points (enabled).
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[1]), UI_ENABLED);
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[2]), UI_ENABLED);

    // Highlight the currently selected menu point.
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[selectedMP]), UI_HIGHLIGHTED);
}

// Draws the initial Show all Page and returns whether lastPlayed game is valid (selectable)
void UI_DrawShowAllPage(UI_ShowAllTabs_t design)
{
    // Print the List background color (only noticable when there are less elements then would fit on the screen)
    LCD_ClearColor(UI_GE_BG_COLOR);

    // Draw the Background of ShowAllPage (brickwall)
    LCD_Brick_t brick;
    brick.Color        = UI_BRICK_COLOR;
    brick.Height       = UI_BRICK_HEIGHT;
    brick.Length       = UI_BRICK_LENGTH;
    brick.Border.Color = UI_BRICK_BORDER_COLOR;
    brick.Border.Width = UI_BRICK_BORDER_WIDTH;

    LCD_DrawWall(0, 0, UI_WALL_1_WIDTH, UI_WALL_1_HEIGHT, false, &brick);
    LCD_DrawWall(0, UI_WALL_1_HEIGHT + 1, UI_WALL_2_WIDTH, UI_WALL_2_HEIGHT, true, &brick);
    LCD_DrawWall(LCD_DISPLAY_SIZE_X - UI_WALL_1_WIDTH, 0, UI_WALL_1_WIDTH, UI_WALL_1_HEIGHT, false, &brick);
    LCD_DrawWall(LCD_DISPLAY_SIZE_X - UI_WALL_2_WIDTH, UI_WALL_1_HEIGHT + 1, UI_WALL_2_WIDTH, UI_WALL_2_HEIGHT, true, &brick);

    // Draw the MP specified by design and load corresponding gameentries
    switch (design)
    {
        case UI_ALLGAMES:
            UI_DrawMenuPoint(&(UI_ShowAll_MenuPoints[0]), UI_ENABLED);
            OS_LoadGameEntries("A", false, false);
            break;

        case UI_FAVORITES:
            UI_DrawMenuPoint(&(UI_ShowAll_MenuPoints[1]), UI_ENABLED);
            OS_LoadGameEntries("A", false, true);
            break;

        case UI_LASTPLAYED:
            UI_DrawMenuPoint(&(UI_ShowAll_MenuPoints[2]), UI_ENABLED);
            OS_LoadLastPlayed();
            break;
    }

    // If no game was loaded return
    if (OS_LoadedGamesCounter == 0) return;

    // Draw the initial Scrollbar.
    UI_DrawScrollBar(0);

    // Draw an offset between the menupoint and the gameentry list.
    LCD_DrawLine(UI_SHOWALL_MPS_X, UI_SHOWALL_MP_HEIGHT - 1, UI_SHOWALL_MP_LENGTH, UI_UPPER_LIST_PADDING, 0x0000, LCD_HORIZONTAL); 

    // Draw as many gameentries as possible (first one highlighted).
    int gameEntryY = UI_SHOWALL_GE_LIST_Y;
    
    UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[0]), UI_HIGHLIGHTED);
    for (int i = 1; i < UI_LIST_LENGTH && i < OS_LoadedGamesCounter; i++)
    {
        gameEntryY += UI_SHOWALL_GE_HEIGHT;
        UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[i]), UI_ENABLED);
    }
}

void UI_DrawOptionsPage(int selectedMP, bool isCartridgeInserted)
{
    // Draw the Page background.
    if (SICK_BRICK_DESIGN)
    {
        LCD_Brick_t brick;
        brick.Color        = UI_BRICK_COLOR;
        brick.Height       = UI_BRICK_HEIGHT;
        brick.Length       = UI_BRICK_LENGTH;
        brick.Border.Color = UI_BRICK_BORDER_COLOR;
        brick.Border.Width = UI_BRICK_BORDER_WIDTH;

        LCD_DrawWall(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y, false, &brick);
    }
    else
    {
        LCD_ClearColor(UI_OPTIONS_BG_COLOR);
    }

    // Draw menupoints.
    if   (OS_EditLanguageMode)  UI_ShowLanguage();
    else                        UI_DrawMenuPoint(&(UI_Options_MenuPoints[0]), UI_ENABLED);

    UI_DrawMenuPoint(&(UI_Options_MenuPoints[1]), UI_DISABLED);

    if   (OS_EditBrightnessMode)  UI_ShowBrightness();
    else                          UI_DrawMenuPoint(&(UI_Options_MenuPoints[2]), UI_ENABLED);

    UI_DrawMenuPoint(&(UI_Options_MenuPoints[3]), UI_ENABLED);
    UI_DrawMenuPoint(&(UI_Options_MenuPoints[4]), UI_ENABLED);
    UI_DrawMenuPoint(&(UI_Options_MenuPoints[5]), UI_ENABLED);


    // If the last menupoint is SAVE CARTRIDGE and no cartridge is present disable the menupoint.
    if (OS_LastState != OS_INGAME_FROM_SDC && OS_LastState != OS_INGAME_FROM_CARTRIDGE && !isCartridgeInserted)
    {
        UI_DrawMenuPoint(&(UI_Options_MenuPoints[5]), UI_DISABLED);
    }

    // Check enabled options.
    if (OS_Options.DrawScaled)  LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
    if (OS_Options.AutoBoot)    LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
    
    // Highlight the currently selected menupoint and its checkmark (if necessary).
    if (!OS_EditLanguageMode && !OS_EditBrightnessMode)  UI_DrawMenuPoint(&(UI_Options_MenuPoints[selectedMP]), UI_HIGHLIGHTED);
    if (selectedMP == 3 && OS_Options.DrawScaled)        LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
    if (selectedMP == 4 && OS_Options.AutoBoot)          LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
}
