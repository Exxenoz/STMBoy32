#include "common.h"

#include "ui.h"
#include "cmod.h"

#include "lcd_config.h"
#include "lcd_drawing.h"



void UI_ShowLanguage(void)
{ 
    UI_MenuPoint_t languageMP = 
    {
        .X      = UI_OPTIONS_MP_1_X,
        .Y      = UI_OPTIONS_MP_1_Y,
        .Height = UI_OPTIONS_MP_HEIGHT,
        .Length = UI_OPTIONS_MP_LENGTH_SHORT,
        .Action = OS_NO_ACTION,
    };

    switch (OS_Options.Language)
    {
        case OS_ENGLISH:
            CopyString(languageMP.Text, "ENGLISH", UI_MAX_MP_LENGTH + 1);
            break;
    }


    UI_DrawMenuPoint(&languageMP, UI_HIGHLIGHTED);
    LCD_DrawSymbol(UI_MP_1_ARROW_LEFT_X, UI_MP_1_ARROW_LEFT_Y, UI_DISABLED_MP_ARROW_COLOR, &Fonts_ArrowLeftToRight_16x31, false);
    LCD_DrawSymbol(UI_MP_1_ARROW_RIGHT_X, UI_MP_1_ARROW_RIGHT_Y, UI_DISABLED_MP_ARROW_COLOR, &Fonts_ArrowLeftToRight_16x31, true);
}

void UI_ShowBrightness(void)
{
    uint16_t arrowLeftColor  = UI_HIGHLIGHTED_MP_ARROW_COLOR;
    uint16_t arrowRightColor = UI_HIGHLIGHTED_MP_ARROW_COLOR;
    
    UI_MenuPoint_t brightnessMP = 
    {
        .X      = UI_OPTIONS_MP_3_X,
        .Y      = UI_OPTIONS_MP_3_Y,
        .Height = UI_OPTIONS_MP_HEIGHT,
        .Length = UI_OPTIONS_MP_LENGTH_MEDIUM,
        .Action = OS_NO_ACTION,
    };    
    sprintf(brightnessMP.Text, "%3d%%", OS_Options.Brightness);


    if (OS_Options.Brightness == OS_MIN_BRIGHTNESS) arrowLeftColor  = UI_DISABLED_MP_ARROW_COLOR;
    if (OS_Options.Brightness == 100)               arrowRightColor = UI_DISABLED_MP_ARROW_COLOR;

    UI_DrawMenuPoint(&brightnessMP, UI_HIGHLIGHTED);
    LCD_DrawSymbol(UI_MP_3_ARROW_LEFT_X, UI_MP_3_ARROW_LEFT_Y, arrowLeftColor, &Fonts_ArrowLeftToRight_16x31, false);
    LCD_DrawSymbol(UI_MP_3_ARROW_RIGHT_X, UI_MP_3_ARROW_RIGHT_Y, arrowRightColor, &Fonts_ArrowLeftToRight_16x31, true);
}

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_DrawOption_t option)
{
    LCD_TextDef_t menuPointDef;

    // Initialize menupoint specification
    int stringLength = Fonts_GetStringLength(menuPoint->Text, UI_MP_SPACING, &UI_MP_FONT);

    CopyString(menuPointDef.Characters, menuPoint->Text, LCD_MAX_TEXT_LENGTH + 1);
    menuPointDef.Spacing       = UI_MP_SPACING;
    menuPointDef.Border.Width  = UI_MP_BORDER_WIDTH;
    menuPointDef.Padding.Upper = ((menuPoint->Height - 2 * UI_MP_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2);
    menuPointDef.Padding.Lower = ((menuPoint->Height - 2 * UI_MP_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2) - 4;
    menuPointDef.Padding.Left  = (menuPoint->Length - 2 * UI_MP_BORDER_WIDTH - stringLength) / 2;
    menuPointDef.Padding.Right = (menuPoint->Length - 2 * UI_MP_BORDER_WIDTH - stringLength) / 2;

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
    }
}

void UI_DrawGameEntry(uint16_t x, uint16_t y, OS_GameEntry_t *gameEntry, UI_DrawOption_t option)
{
     LCD_TextDef_t gameEntryDef;

    // Initialize gameentry specifications
    int stringLen = Fonts_GetStringLengthWithoutSuffix(gameEntry->Name, UI_GE_SPACING, &UI_GE_FONT);

    CopyStringWithoutSuffix(gameEntryDef.Characters, gameEntry->Name, LCD_MAX_TEXT_LENGTH + 1);
    gameEntryDef.Spacing       = UI_GE_SPACING;
    gameEntryDef.Border.Width  = UI_GE_BORDER_WIDTH;
    gameEntryDef.Padding.Upper = ((UI_SHOWALL_GE_HEIGHT - 2 * UI_GE_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2);
    gameEntryDef.Padding.Lower = ((UI_SHOWALL_GE_HEIGHT - 2 * UI_GE_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2);
    gameEntryDef.Padding.Left  = 0;
    gameEntryDef.Padding.Right = (UI_SHOWALL_GE_LENGTH - 2 * UI_GE_BORDER_WIDTH  - stringLen);

    switch(option)
    {
        case UI_ENABLED:
            gameEntryDef.Color = UI_GE_TEXT_COLOR;
            gameEntryDef.Border.Color = UI_GE_BORDER_COLOR;
            LCD_DrawText(x, y, UI_GE_BG_COLOR, &gameEntryDef, &UI_GE_FONT);
            if (gameEntry->IsFavorite)
            {
                LCD_DrawSymbol(UI_GE_STAR_X, y + UI_GE_STAR_OFFSET_Y, UI_GE_STAR_COLOR, &Fonts_Star_32x22, false);
            }
            break;

        case UI_DISABLED:
            gameEntryDef.Color = UI_DISABLED_GE_TEXT_COLOR;
            gameEntryDef.Border.Color = UI_GE_BORDER_COLOR;
            LCD_DrawText(x, y, UI_DISABLED_GE_BG_COLOR, &gameEntryDef, &UI_GE_FONT);
            break;

        case UI_HIGHLIGHTED:
            gameEntryDef.Color = UI_HIGHLIGHTED_GE_TEXT_COLOR;
            gameEntryDef.Border.Color = UI_HIGHLIGHTED_GE_BORDER_COLOR;
            LCD_DrawText(x, y, UI_HIGHLIGHTED_GE_BG_COLOR, &gameEntryDef, &UI_GE_FONT);
            if (gameEntry->IsFavorite)
            {
                LCD_DrawSymbol(UI_GE_STAR_X, y + UI_GE_STAR_OFFSET_Y, UI_HIGHLIGHTED_GE_STAR_COLOR, &Fonts_Star_32x22, false);
            }
            break;
    }
}

// Used to update the currently selected gameentry when its favorite-status changes
void UI_ReDrawCurrGE(int currGEIndex, int currGEListID)
{
    uint16_t gameEntryY = UI_SHOWALL_GE_LIST_Y + (currGEListID * UI_SHOWALL_GE_HEIGHT);
    uint16_t gameEntryX = UI_SHOWALL_GE_LIST_X;

    UI_DrawGameEntry(gameEntryX, gameEntryY, &(OS_GameEntries[currGEIndex]), UI_HIGHLIGHTED);
}

// Used to update the displayed gameentries when a favorite got removed from the favorite list
void UI_ReDrawGEList(int currGEIndex, int currGEListID)
{
    uint16_t gameEntryY = UI_SHOWALL_GE_LIST_Y;
    uint16_t gameEntryX = UI_SHOWALL_GE_LIST_X;

    // Draw the gameentries previous to the currently selected non highlighted
    for (int i = currGEListID; i > 0; i--)
    {
        UI_DrawGameEntry(gameEntryX, gameEntryY, &(OS_GameEntries[currGEIndex - i]), UI_ENABLED);
        gameEntryY += UI_SHOWALL_GE_HEIGHT;
    }

    // Draw the currently selected gameentry highlighted (if any is left)
    if (currGEIndex != -1)
    {
        UI_DrawGameEntry(gameEntryX, gameEntryY, &(OS_GameEntries[currGEIndex]), UI_HIGHLIGHTED);
    }
    else
    {
        LCD_DrawFilledBox(gameEntryX, gameEntryY, UI_SHOWALL_GE_LENGTH, UI_SHOWALL_GE_HEIGHT, UI_GE_BG_COLOR);
    }

    // Draw the gameentries following to the currently selected non highlighted (if any)
    for (int i = 1; i < (UI_LIST_LENGTH - currGEListID); i++)
    {
        gameEntryY += UI_SHOWALL_GE_HEIGHT;
        if ((currGEIndex + i) < OS_LoadedGamesCounter)
        {
            UI_DrawGameEntry(gameEntryX, gameEntryY, &(OS_GameEntries[currGEIndex + i]), UI_ENABLED);
        }
        else
        {
            LCD_DrawFilledBox(gameEntryX, gameEntryY, UI_SHOWALL_GE_LENGTH, UI_SHOWALL_GE_HEIGHT, UI_GE_BG_COLOR);
            break;
        }
    }
}

void UI_DrawScrollBar(int currGameEntry)
{
    uint16_t x   = UI_SHOWALL_GE_LIST_X + UI_SHOWALL_GE_LENGTH;
    uint16_t y   = UI_SHOWALL_GE_LIST_Y;
    uint16_t len = UI_LIST_LENGTH * UI_SHOWALL_GE_HEIGHT;

    // Draw the scrollbar background
    LCD_DrawLine(x, y, len, UI_SCROLLBAR_WIDTH, UI_SCROLLBAR_BG_COLOR, LCD_VERTICAL);

    // If no gameentry is left in the list dont draw the scrollbar
    if (currGameEntry == -1) return;

    // Draw scrollbar depending of currently selected entry position and totalgames/displayedgames ratio
    y   += currGameEntry * len / OS_TotalGamesCounter;
    len /= OS_TotalGamesCounter;

    // If currGameEntry is the last one & (currGameEntry * len / OS_TotalGamesCounter) is not an integer correct y
    if (currGameEntry == OS_TotalGamesCounter - 1)
    {
        if ((UI_SHOWALL_GE_LIST_Y + (currGameEntry * len / OS_TotalGamesCounter)) - y != 0) y++;
    }

    LCD_DrawLine(x + 1, y, len, UI_SCROLLBAR_WIDTH - 2, UI_SCROLLBAR_FG_COLOR, LCD_VERTICAL);
}

void UI_ScrollGames(int *p_currGEIndex, int *p_currGEListID, UI_ScrollOption_t option)
{
    uint16_t gameEntryY = UI_SHOWALL_GE_LIST_Y + ((*p_currGEListID) * UI_SHOWALL_GE_HEIGHT);

    if (option == UI_SCROLLUP)
    {
        // Selected gameentry is the first displayed
        if (*p_currGEListID == 0)
        {
            // If currGE is the first of the currently loaded GameEntries and the list is scrolled up,
            // load OS_MAX_NUMBER_OF_GAMES previous to the last displayed GameEntry and set new currGEIndex
            if (*p_currGEIndex == 0)
            {
                OS_LoadGameEntries(OS_GameEntries[UI_LIST_LENGTH - 1].Name, true, false);
                *p_currGEIndex = OS_LoadedGamesCounter - UI_LIST_LENGTH;
            }

            // Adapt currGEIndex and draw previous gameentry to first displayed position
            (*p_currGEIndex)--;
            UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_HIGHLIGHTED);

            // Draw as many gameentries as fit the rest of the screen
            for (int i = 1; i < UI_LIST_LENGTH; i++)
            {
                gameEntryY += UI_SHOWALL_GE_HEIGHT;
                UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex + i]), UI_ENABLED);
            }

            return;
        }

        // Selected gameentry is neither first nor first displayed
        // Reset highlighting of currGE, adapt currGEIndex and highlight prev gameentry
        UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_ENABLED);

        gameEntryY -= UI_SHOWALL_GE_HEIGHT;

        (*p_currGEIndex)--;

        UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_HIGHLIGHTED);

        // If the selected gameentry wasn't the first displayed the listID changes
        (*p_currGEListID)--;

        return;
    }
    else
    {
        // Selected gameentry is the last displayed
        if (*p_currGEListID == (UI_LIST_LENGTH - 1))
        {
            // If currGE is the last of the currently loaded GameEntries and the list is scrolled down,
            // load OS_MAX_NUMBER_OF_GAMES following the first displayed GameEntry and set new currGEIndex
            if ((*p_currGEIndex) == (OS_LoadedGamesCounter - 1))
            {
                OS_LoadGameEntries(OS_GameEntries[*p_currGEIndex -(UI_LIST_LENGTH - 1)].Name, false, false);
                *p_currGEIndex = UI_LIST_LENGTH - 1;
            }

            // Adapt currGEIndex and draw next gameentry to last displayed position
            (*p_currGEIndex)++;
            UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_HIGHLIGHTED);

            // Draw as many gameentries as fit the rest of the screen (top down)
            for (int i = 1; i < UI_LIST_LENGTH; i++)
            {
                gameEntryY -= UI_SHOWALL_GE_HEIGHT;
                UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex - i]), UI_ENABLED);
            }

            return;
        }

        // Selected gameentry isn't the last displayed
        // Reset highlighting of currGE, adapt currGEIndex and highlight next gameentry
        UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_ENABLED);

        gameEntryY += UI_SHOWALL_GE_HEIGHT;

        (*p_currGEIndex)++;

        UI_DrawGameEntry(UI_SHOWALL_GE_LIST_X, gameEntryY, &(OS_GameEntries[*p_currGEIndex]), UI_HIGHLIGHTED);

        // If the selected gameentry wasn't the last displayed the listID changes
        (*p_currGEListID)++;
        return;
    }
}
