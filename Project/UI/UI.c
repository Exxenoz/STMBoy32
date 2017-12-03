#include "ui.h"

const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS] = {
    {UI_MAINPAGE_MP_1_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_1_Y,
     UI_MAINPAGE_MP_HEIGHT, UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_INGAME_FC},

    {UI_MAINPAGE_MP_2_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_2_Y,
     UI_MAINPAGE_MP_HEIGHT, UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_SHOWALL},

    {UI_MAINPAGE_MP_3_STRING, UI_MAINPAGE_MPS_X, UI_MAINPAGE_MP_3_Y,
     UI_MAINPAGE_MP_HEIGHT, UI_MAINPAGE_MP_LENGTH, OS_SWITCH_TO_STATE_OPTIONS},
};


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

    // Highlight the first valid menupoint
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[firstValidMenuPoint]), UI_HIGHLIGHTED);
}

// Draws the initial Show all Page and returns whether lastPlayed game is valid (selectable)
bool UI_DrawShowAllPage(void)
{
    bool lastPlayedGameValid;

    OS_InitializeGameEntries();

    // Print the List background color (only noticable when therre are less elements then would fit on the screen)
    LCD_ClearColor(UI_GE_BG_COLOR);

    // Draw the Background of ShowAllPage (brickwall)
    LCD_Brick_t brick;
    brick.Color        = UI_BRICK_COLOR;
    brick.Height       = UI_BRICK_HEIGHT;
    brick.Length       = UI_BRICK_LENGTH;
    brick.Border.Color = UI_BRICK_BORDER_COLOR;
    brick.Border.Width = UI_BRICK_BORDER_WIDTH;

    LCD_DrawWall(0, 0, UI_WALL_WIDTH1, UI_SHOWALL_GE_HEIGHT, &brick);
    LCD_DrawWall(0, UI_SHOWALL_GE_HEIGHT, UI_WALL_WIDTH2, LCD_DISPLAY_SIZE_Y, &brick);
    LCD_DrawWall(LCD_DISPLAY_SIZE_X - UI_WALL_WIDTH1, 0, UI_WALL_WIDTH1, UI_SHOWALL_GE_HEIGHT, &brick);
    LCD_DrawWall(LCD_DISPLAY_SIZE_X - UI_WALL_WIDTH2, UI_SHOWALL_GE_HEIGHT, UI_WALL_WIDTH2, LCD_DISPLAY_SIZE_Y, &brick);

    // Draw last played Game either highlighted or disabled (if not found/valid) centered to the top of the screen
    OS_GameEntry_t lastPlayedGame = OS_GetGameEntry(OS_InitOptions.lastPlayed);
    if (OS_InitOptions.lastPlayed[0] == '\0' || lastPlayedGame.Name[0] == '\0')
    {
        UI_DrawGameEntry(UI_WALL_WIDTH1, 0, &lastPlayedGame, UI_DISABLED, true);
        lastPlayedGameValid = false;
    }
    else
    {
        UI_DrawGameEntry(UI_WALL_WIDTH1, 0, &lastPlayedGame, UI_HIGHLIGHTED, true);
        lastPlayedGameValid = true;
    }

    // Fill lower part of the screen with as many Gameentries as possible
    int gameEntryY;
    for (int i = 1; i < UI_LIST_LENGTH && i <= OS_GamesLoaded; i++)
    {
        gameEntryY = UI_UPPER_LIST_PADDING + i * UI_SHOWALL_GE_HEIGHT;
        UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[i-1]), UI_ENABLED, false);
    }

    // Draw the offset between lastPlayedGame and first Gameentry in border color
    LCD_DrawLine(UI_WALL_WIDTH2, UI_SHOWALL_GE_HEIGHT, UI_SHOWALL_GE_LENGTH, UI_UPPER_LIST_PADDING, UI_GE_BG_COLOR, LCD_HORIZONTAL); 

    // Draw the initial Scrollbar
    UI_DrawScrollBar(0);

    // Highlight first Gameentry if lastPlayedGame isn't valid
    if (!lastPlayedGameValid)
    {
        UI_DrawGameEntry(UI_WALL_WIDTH2, UI_SHOWALL_GE_HEIGHT, &(OS_GameEntries[0]), UI_HIGHLIGHTED, false);
    }

    return lastPlayedGameValid;
}

void UI_DrawShowFavPage(void)
{
    // YTBI
}

void UI_DrawOptionsPage(void)
{
    // YTBI
}

void UI_DrawMenuPoint(const UI_MenuPoint_t *menuPoint, UI_DrawOption_t option)
{
    LCD_TextDef_t menuPointDef;

    // Initialize menupoint specification
    int stringLength = Fonts_GetStringLength(menuPoint->Text, UI_MP_SPACING, &UI_MP_FONT);

    copyString(menuPointDef.Characters, menuPoint->Text, LCD_MAX_TEXT_LENGTH + 1);
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

void UI_DrawGameEntry(uint16_t x, uint16_t y, OS_GameEntry_t *gameEntry, UI_DrawOption_t option, bool lastPlayed)
{
     LCD_TextDef_t gameEntryDef;

    // Initialize gameentry specifications
    int stringLen = Fonts_GetStringLength(gameEntry->Name, UI_GE_SPACING, &UI_GE_FONT);

    copyString(gameEntryDef.Characters, gameEntry->Name, LCD_MAX_TEXT_LENGTH + 1);
    gameEntryDef.Spacing       = UI_GE_SPACING;
    gameEntryDef.Border.Width  = UI_GE_BORDER_WIDTH;
    gameEntryDef.Padding.Upper = ((UI_SHOWALL_GE_HEIGHT - 2 * UI_GE_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2);
    gameEntryDef.Padding.Lower = ((UI_SHOWALL_GE_HEIGHT - 2 * UI_GE_BORDER_WIDTH - UI_MP_FONT.FontHeight) / 2) - 4;
    if (lastPlayed)
    {
        gameEntryDef.Padding.Left  = ((UI_SHOWALL_MP_LENGTH - 2 * UI_GE_BORDER_WIDTH - stringLen) / 2);
        gameEntryDef.Padding.Right = ((UI_SHOWALL_MP_LENGTH - 2 * UI_GE_BORDER_WIDTH - stringLen) / 2);
    }
    else
    {
        gameEntryDef.Padding.Left  = 0;
        gameEntryDef.Padding.Right = (UI_SHOWALL_GE_LENGTH - 2 * UI_GE_BORDER_WIDTH  - stringLen);
    }

    switch(option)
    {
        case UI_ENABLED:
            gameEntryDef.Color = UI_GE_TEXT_COLOR;
            gameEntryDef.Border.Color = UI_GE_BORDER_COLOR;
            LCD_DrawText(x, y, UI_GE_BG_COLOR, &gameEntryDef, &UI_GE_FONT);
            if (gameEntry->IsFavorite)
            {
                LCD_DrawStar(UI_GE_STAR_X, y + UI_GE_STAR_OFFSET_Y, UI_GE_STAR_COLOR);
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
                LCD_DrawStar(UI_GE_STAR_X, y + UI_GE_STAR_OFFSET_Y, UI_HIGHLIGHTED_GE_STAR_COLOR);
            }
            break;
    }
}

void UI_DrawScrollBar(int currentGameEntry)
{
    // Draw the scrollbar background
    uint16_t X   = UI_WALL_WIDTH2 + UI_SHOWALL_GE_LENGTH;
    uint16_t Y   = UI_SHOWALL_GE_HEIGHT + UI_UPPER_LIST_PADDING;
    uint16_t Len = (UI_LIST_LENGTH - 1) * UI_SHOWALL_GE_HEIGHT;
    double ratio = (UI_LIST_LENGTH - 1) / OS_GamesLoaded;

    LCD_DrawLine(X, Y, Len, UI_SCROLLBAR_WIDTH, UI_SCROLLBAR_BG_COLOR, LCD_VERTICAL);

    // Draw scrollbar depending of currently selected entry position and totalgames/displayedgames ratio
    X++;
    Y   = UI_SHOWALL_GE_HEIGHT + UI_UPPER_LIST_PADDING + (LCD_DISPLAY_SIZE_Y * currentGameEntry / OS_GamesLoaded);
    Len = LCD_DISPLAY_SIZE_Y * (ratio < 1 ? ratio : 1);

    LCD_DrawLine(X, Y, Len, UI_SCROLLBAR_WIDTH - 2, UI_SCROLLBAR_FG_COLOR, LCD_VERTICAL);
}

bool UI_ScrollGames(int currGE, int firstDisplayedGE, UI_ScrollOption_t option)
{
    // Scrollable list consists of gameEntriesFullyFitting - 1 displayed games
    // If currGE is -1 lastPlayedGame is selected but currGameDisplayID & gameEntryY refer to the listed games
    int currGameDisplayID = (currGE == -1 ? 0 : currGE) % (UI_LIST_LENGTH - 1);
    int lastDisplayedGE   = firstDisplayedGE + (UI_LIST_LENGTH - 2);
    int gameEntryY        = (currGameDisplayID + 1) * UI_SHOWALL_GE_HEIGHT + UI_UPPER_LIST_PADDING;

    if (option == UI_SCROLLUP)
    {
        // If currently first gameentry is selected highlight lastplayed
        if (currGE == 0)
        {
            // Reset highlighting of first gameentry and highlight lastPlayed instead
            UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE]), UI_ENABLED, false);

            OS_GameEntry_t lastPlayedGame = OS_GetGameEntry(OS_InitOptions.lastPlayed);
            UI_DrawGameEntry(UI_WALL_WIDTH1, 0, &lastPlayedGame, UI_HIGHLIGHTED, true);

            // List didn't scroll
            return false;
        }

        // Selected gameentry isn't the first but the first displayed
        if (currGE == firstDisplayedGE)
        {
            // Draw previous gameentry to first displayed position
            UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE - 1]), UI_HIGHLIGHTED, false);

            // Draw as many gameentries as fit the screen
            // (-2 because gameEntriesFullyFitting includes lastPlayed and first was already drawn)
            for (int i = 1; i < (UI_LIST_LENGTH - 1); i++)
            {
                gameEntryY += UI_SHOWALL_GE_HEIGHT;
                UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE - 1 + i]), UI_ENABLED, false);
            }

            // List scrolled
            return true;
        }

        // Selected gameentry is neither first nor first displayed
        // Reset highlighting of currGE and highlight prev gameentry
        UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE]), UI_ENABLED, false);
        gameEntryY -= UI_SHOWALL_GE_HEIGHT;
        UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE - 1]), UI_HIGHLIGHTED, false);

        // List didn't scroll
        return false;
    }
    else
    {
        // If currently lastPlayed is selected highlight first gameentry
        if (currGE == -1)
        {
            // Reset highlighting of lastPlayed and highlight first gameentry instead
            OS_GameEntry_t lastPlayedGame = OS_GetGameEntry(OS_InitOptions.lastPlayed);
            UI_DrawGameEntry(UI_WALL_WIDTH1, 0, &lastPlayedGame, UI_ENABLED, true);

            UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE + 1]), UI_HIGHLIGHTED, false);

            // List didn't scroll
            return false;
        }

        // Selected gameentry is the last displayed
        if (currGE == lastDisplayedGE)
        {
            // Draw next gameentry to last displayed position
            UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE + 1]), UI_HIGHLIGHTED, false);

            // Draw as many gameentries as fit the screen
            // (-2 because gameEntriesFullyFitting includes lastPlayed and last was already drawn)
            for (int i = 1; i < (UI_LIST_LENGTH - 1); i++)
            {
                gameEntryY -= UI_SHOWALL_GE_HEIGHT;
                UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE + 1 - i]), UI_ENABLED, false);
            }

            // List scrolled
            return true;
        }

        // Selected gameentry isn't the last displayed
        // Reset highlighting of currGE and highlight next gameentry
        UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE]), UI_ENABLED, false);
        gameEntryY += UI_SHOWALL_GE_HEIGHT;
        UI_DrawGameEntry(UI_WALL_WIDTH2, gameEntryY, &(OS_GameEntries[currGE + 1]), UI_HIGHLIGHTED, false);

        // List didn't scroll
        return false;
    }
}

// Compiler workaround functions

void UI_DrawMainPageMenuPoint(int id, UI_DrawOption_t option)
{
    UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[id]), option);
}

void UI_PerformMainPageAction(int id)
{
    OS_DoAction(UI_MainPage_MenuPoints[id].Action);
}
