#include "os_state_handlers.h"



void HandleMainPage(void)
{
    // If no cartridge is detected first valid menupoint is SHOW ALL GAMES (ID 1) else BOOT CARTRIDGE (ID 0).
    int firstMenuPointID = CMOD_CheckForCartridge() ? 0 : 1;
    int lastMenuPointID  = 2;
    int currMenuPointID  = firstMenuPointID;

    // Draw MainPage
    UI_DrawMainPage(currMenuPointID);

    // MainPage loop
    while (1)
    {
        // Update the lockState of all buttons.
        // Locks are needed because otherwise a short press would trigger multiple scroll downs.
        Input_UpdateLocks();

        // If no cartridge is detected and BOOT CARTRIDGE is enabled disable it and vice versa.
        if (!CMOD_CheckForCartridge() && firstMenuPointID == 0)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_DISABLED);
            firstMenuPointID = 1;
        }
        else if (CMOD_CheckForCartridge() && firstMenuPointID == 1)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_ENABLED);
            firstMenuPointID = 0;
        }

        // If Fade-Top is pressed and we don't have the first valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point above.
        if (Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_TOP_ID) && currMenuPointID > firstMenuPointID)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_ENABLED);

            currMenuPointID--;

            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_TOP_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }

        // If Fade-Bot is pressed and we don't have the last valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point below.
        if (Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_BOT_ID) && currMenuPointID < lastMenuPointID)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_ENABLED);

            currMenuPointID++;

            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_BOT_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }
        
        // If A-Button is pressed confirm the current selection and end the infinite loop.
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID))
        {
            // Perform the action linked to the selected menupoint
            OS_DoAction(UI_MainPage_MenuPoints[currMenuPointID].Action);

            // Lock all buttons until they are released so next page opens without anything pressed.
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again.
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            break;
        }
    }
}

void HandleShowAllGamesPage(void)
{
    UI_ShowAllTabs_t tabs[UI_NUMBER_OF_SHOWALL_MPS] = { UI_ALLGAMES, UI_FAVORITES, UI_LASTPLAYED };
    int currDesignID = 0;

    // Draw the page.
    UI_DrawShowAllPage(tabs[currDesignID]);

    // Initialize IDs.
    int lastGameEntryID     = OS_TotalGamesCounter - 1;
    int currGameEntryID     = 0;
    int currGameEntryIndex  = 0;
    int currGameEntryListID = 0;


    while (1)
    {
        // Update the lockState of all buttons.
        // Locks are needed because otherwise a short press would trigger multiple scroll downs.
        Input_UpdateLocks();

        // If Fade-Right is pressed switch design accordingly.
        if (Input_Interrupt_Flags.FadeRight && !Input_IsLocked(INPUT_FADE_RIGHT_ID))
        {
            if (currDesignID == UI_NUMBER_OF_SHOWALL_MPS - 1) currDesignID = 0;
            else                                              currDesignID++;

            UI_DrawShowAllPage(tabs[currDesignID]);

            // Re-Initialize IDs
            lastGameEntryID     = OS_TotalGamesCounter - 1;
            currGameEntryID     = 0;
            currGameEntryIndex  = 0;
            currGameEntryListID = 0;

            Input_Lock(INPUT_FADE_RIGHT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If Fade-Left is pressed switch design accordingly.
        if (Input_Interrupt_Flags.FadeLeft && !Input_IsLocked(INPUT_FADE_LEFT_ID))
        {
            if (currDesignID == 0) currDesignID = UI_NUMBER_OF_SHOWALL_MPS - 1;
            else                   currDesignID--;

            UI_DrawShowAllPage(tabs[currDesignID]);

            // Re-Initialize IDs
            lastGameEntryID     = OS_TotalGamesCounter - 1;
            currGameEntryID     = 0;
            currGameEntryIndex  = 0;
            currGameEntryListID = 0;

            Input_Lock(INPUT_FADE_LEFT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If Fade-Top is pressed & Fade-Bot not & we don't have the first valid entry already selected, scroll up.
        if (Input_Interrupt_Flags.FadeTop && !Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_TOP_ID)
            && currGameEntryID > 0)
        {
            UI_ScrollGames(&currGameEntryIndex, &currGameEntryListID, UI_SCROLLUP);
            currGameEntryID--;
            UI_DrawScrollBar(currGameEntryID);

            Input_LockDynamically(INPUT_FADE_TOP_ID);
        }

        // If Fade-Bot is pressed & Fade-Top not & we don't have the last valid entry already selected, scroll down.
        if (Input_Interrupt_Flags.FadeBot && !Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_BOT_ID)
            && currGameEntryID < lastGameEntryID)
        {
            UI_ScrollGames(&currGameEntryIndex, &currGameEntryListID, UI_SCROLLDOWN);
            currGameEntryID++;
            UI_DrawScrollBar(currGameEntryID);

            Input_LockDynamically(INPUT_FADE_BOT_ID);
        }

        // If Select-Button is pressed invert the favorite-status of the currently selected game (if valid).
        if (Input_Interrupt_Flags.ButtonSelect && !Input_IsLocked(INPUT_SELECT_ID) && currGameEntryID != -1)
        {
            OS_InvertFavoriteStatus(&OS_GameEntries[currGameEntryIndex]);

            // If only favorites are displayed this means deleting the currently selected game from the list.
            if (tabs[currDesignID] == UI_FAVORITES)
            {
                OS_RemoveGameEntry(currGameEntryIndex);

                // If the last GE was selected previously now the previous GE is selected, otherwise the next.
                if (currGameEntryIndex == OS_TotalGamesCounter)
                {
                    currGameEntryID--;
                    currGameEntryIndex--;
                    currGameEntryListID--;
                }
                lastGameEntryID--;

                UI_ReDrawGEList(currGameEntryIndex, currGameEntryListID);
                UI_DrawScrollBar(currGameEntryID);
            }
            // If all games or last played are displayed only the favorite-indication (star) changes.
            else
            {
                UI_ReDrawCurrGE(currGameEntryIndex, currGameEntryListID);
            }

            Input_Lock(INPUT_SELECT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If A-Button is pressed confirm the current selection (if valid) and end the infinite loop.
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID) && currGameEntryID != -1)
        {
            // Set the game which is to be started.
            CopyString(OS_CurrentGame.Name, OS_GameEntries[currGameEntryIndex].Name, sizeof(OS_CurrentGame.Name));
            OS_CurrentGame.IsFavorite = OS_GameEntries[currGameEntryIndex].IsFavorite;

            OS_DoAction(OS_SWITCH_TO_STATE_INGAME_FSD);
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            break;
        }

        // If B-Button is pressed switch to previous state and end the infinite loop.
        if (Input_Interrupt_Flags.ButtonB && !Input_IsLocked(INPUT_B_ID))
        {
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            OS_DoAction(OS_SWITCH_TO_PREVIOUS_STATE);
            break;
        }
    }
}

void HandleOptionPage(void)
{
    int  firstMenuPointID = 0;
    int  lastMenuPointID  = 5;
    int  currMenuPointID  = firstMenuPointID;
    bool accessedFromGame = (bool)(OS_LastState == OS_INGAME_FROM_SDC || OS_LastState == OS_INGAME_FROM_CARTRIDGE);

    // Handle whether the page is accessed from the mainpage or from within a game.
    if (accessedFromGame)
    {
        UI_Options_MenuPoints[5] = UI_Options_alternate_MP_6;
    }
    else
    {
        UI_Options_MenuPoints[5] = UI_Options_regular_MP_6;

        if (!CMOD_CheckForCartridge())
        {
            lastMenuPointID = 4;
        }
    }

    // Draw options page.
    UI_DrawOptionsPage(currMenuPointID);

    // Options loop.
    while (1)
    {
        // Update the lockState of all buttons.
        // Locks are needed because otherwise a short press would trigger multiple scroll downs.
        Input_UpdateLocks();

        // If no cartridge is detected and SAVE CARTRIDGE is enabled disable it and vice versa.
        if (!accessedFromGame)
        {
            if (!CMOD_CheckForCartridge() && lastMenuPointID == 5)
            {
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[5]), UI_DISABLED);
                lastMenuPointID = 4;
            }
            else if (CMOD_CheckForCartridge() && lastMenuPointID == 4)
            {
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[5]), UI_ENABLED);
                lastMenuPointID = 5;
            }
        }
        
        // If Fade-Right is pressed switch design accordingly.
        if (Input_Interrupt_Flags.FadeRight && !Input_IsLocked(INPUT_FADE_RIGHT_ID))
        {
            if (currMenuPointID == 0 || currMenuPointID == 1)
            {/*
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_ENABLED);
                
                if   (currMenuPointID == 0)  currMenuPointID = 1;
                else                         currMenuPointID = 0;
                
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);
                
                Input_Lock(INPUT_FADE_RIGHT_ID, INPUT_LOCK_UNTIL_RELEASED);*/
            }
            
            if (OS_EditBrightnessMode && OS_Options.Brightness < 100)
            {
                OS_Options.Brightness++;
                OS_UpdateOptions();

                LCD_DimBacklight(OS_Options.Brightness);
                UI_ShowBrightness();
                
                Input_LockDynamically(INPUT_FADE_RIGHT_ID);
            }
            
            if (OS_EditLanguageMode && (int)OS_Options.Language < OS_NUMBER_OF_SUPPORTED_LANGUAGES)
            {
                if (OS_Options.Language == (OS_Language_t)(UI_SUPPORTED_LANGUAGES_NUM - 1))
                {
                    OS_Options.Language = (OS_Language_t)0;
                }
                else
                {
                    OS_Options.Language++;
                }
                OS_UpdateOptions();

                UI_ShowLanguage();

                Input_Lock(INPUT_FADE_RIGHT_ID, INPUT_LOCK_UNTIL_RELEASED);
            }
        }

        // If Fade-Left is pressed switch design accordingly.
        if (Input_Interrupt_Flags.FadeLeft && !Input_IsLocked(INPUT_FADE_LEFT_ID))
        {
            if (currMenuPointID == 0 || currMenuPointID == 1)
            {/*
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_ENABLED);
                
                if   (currMenuPointID == 0)  currMenuPointID = 1;
                else                         currMenuPointID = 0;
                
                UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);

                Input_Lock(INPUT_FADE_LEFT_ID, INPUT_LOCK_UNTIL_RELEASED);*/
            }
            
            if (OS_EditBrightnessMode && OS_Options.Brightness > OS_MIN_BRIGHTNESS)
            {
                OS_Options.Brightness--;
                OS_UpdateOptions();

                LCD_DimBacklight(OS_Options.Brightness);
                UI_ShowBrightness();
                
                Input_LockDynamically(INPUT_FADE_LEFT_ID);
            }
            
            if (OS_EditLanguageMode && (int)OS_Options.Language > 0)
            {
                if (OS_Options.Language == (OS_Language_t)0)
                {
                    OS_Options.Language = (OS_Language_t)(UI_SUPPORTED_LANGUAGES_NUM - 1);
                }
                else
                {
                    OS_Options.Language--;
                }
                OS_UpdateOptions();

                UI_ShowLanguage();

                Input_Lock(INPUT_FADE_LEFT_ID, INPUT_LOCK_UNTIL_RELEASED);
            }
        }

        // If Fade-Top is pressed and scrolling is currently possible,
        // reset highlighting of current menu point and highlight the menu point above.
        if (Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_TOP_ID) && currMenuPointID > firstMenuPointID && !OS_EditBrightnessMode && !OS_EditLanguageMode)
        {
            // Reset highlighting of the selected menu point and the correspondig checkmark (if there is one).
            UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_ENABLED);
            if (currMenuPointID == 3 && OS_Options.DrawScaled)  LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
            if (currMenuPointID == 4 && OS_Options.AutoBoot)    LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);

            // Menu point 1 is besides menu point 0
            if   (currMenuPointID == 2) currMenuPointID = 0;
            else                        currMenuPointID--;

            // Highlight selected menu point and the correspondig checkmark (if there is one).
            UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);
            if (currMenuPointID == 3 && OS_Options.DrawScaled)  LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
            if (currMenuPointID == 4 && OS_Options.AutoBoot)    LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);

            Input_Lock(INPUT_FADE_TOP_ID, OS_OPTIONS_PAGE_BUTTON_LOCK_TIME);
        }

        // If Fade-Top is pressed and scrolling is currently possible,
        // reset highlighting of current menu point and highlight the menu point below.
        if (Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_BOT_ID) && currMenuPointID < lastMenuPointID && !OS_EditBrightnessMode && !OS_EditLanguageMode)
        {
            // Reset highlighting of the selected menu point and the correspondig checkmark (if there is one).
            UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_ENABLED);
            if (currMenuPointID == 3 && OS_Options.DrawScaled)  LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
            if (currMenuPointID == 4 && OS_Options.AutoBoot)    LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);

            // Menu point 1 is besides menu point 0
            if   (currMenuPointID == 0) currMenuPointID = 2;
            else                        currMenuPointID++;

            // Highlight selected menu point and the correspondig checkmark (if there is one).
            UI_DrawMenuPoint(&(UI_Options_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);
            if (currMenuPointID == 3 && OS_Options.DrawScaled)  LCD_DrawSymbol(UI_MP_4_CHECKMARK_X, UI_MP_4_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);
            if (currMenuPointID == 4 && OS_Options.AutoBoot)    LCD_DrawSymbol(UI_MP_5_CHECKMARK_X, UI_MP_5_CHECKMARK_Y, UI_HIGHLIGHTED_MP_CHECKMARK_COLOR, &Fonts_Checkmark_28x20, false);

            Input_Lock(INPUT_FADE_BOT_ID, OS_OPTIONS_PAGE_BUTTON_LOCK_TIME);
        }
        
        // If A-Button is pressed confirm the current selection and end the infinite loop.
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID))
        {
            // Perform the action linked to the selected menupoint
            OS_DoAction(UI_Options_MenuPoints[currMenuPointID].Action);
            
            // Lock all buttons until they are released so next page opens without anything pressed.
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again.
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            
            // If END GAME or DESIGNS was pressed leave this page, if not re-draw it.
            if   (currMenuPointID == 1 || (accessedFromGame && currMenuPointID == 5))  break;
            else                                                                       UI_DrawOptionsPage(currMenuPointID);
        }
        
        // If B-Button is pressed switch to previous state and end the infinite loop.
        if (Input_Interrupt_Flags.ButtonB && !Input_IsLocked(INPUT_B_ID))
        {
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            OS_DoAction(OS_SWITCH_TO_PREVIOUS_STATE);
            break;
        }
    }
}

void HandleSDCIngame(void)
{
    // If the state switches directly from options to ingame the game was only paused and doesn't need to be loaded again.
    if (OS_LastState != OS_OPTIONS)
    {
        // Define path array with maximal needed size and get the game path.
        char path[OS_MAX_PATH_LENGTH];
        OS_GetGamePath(&OS_CurrentGame, path, OS_MAX_PATH_LENGTH);

        // Load the game.
        if(GBC_LoadFromSDC(path) != GBC_LOAD_RESULT_OK)
        {
            LED_EnableRed(true);

            OS_DoAction(OS_SWITCH_TO_STATE_MAINPAGE);

            return;
        }
    }

    // If the game is not to be scaled draw the frame.
    if (!OS_Options.DrawScaled)
    {
        LCD_Brick_t brick;
        brick.Color        = UI_BRICK_COLOR;
        brick.Height       = UI_BRICK_HEIGHT;
        brick.Length       = UI_BRICK_LENGTH;
        brick.Border.Color = UI_BRICK_BORDER_COLOR;
        brick.Border.Width = UI_BRICK_BORDER_WIDTH;

        LCD_DrawWall(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y, false, &brick);
    }


    while (1)
    {
        // If select and start are pressed simultaneously in game pause it and go to options page.
        if (Input_Interrupt_Flags.ButtonSelect && Input_Interrupt_Flags.ButtonStart)
        {
            // Quickfix until drawing methods use dma.
            while (LCD_DMA_TransferCompleteFlags != LCD_DMA_TRANSFER_COMPLETE);

            // Lock all buttons until they are released so options opens without anything pressed.
            // If a button is not pressed at this time UpdateLocks will immediately disable the lock again.
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            OS_DoAction(OS_SWITCH_TO_STATE_OPTIONS);

            return;
        }

        GBC_Update();

        while (Audio_IsPlayingOfBufferFinished == false);
        Audio_IsPlayingOfBufferFinished = false;

        if (OS_Options.DrawScaled)  LCD_DrawGBCFrameBufferScaled();
        else                        LCD_DrawGBCFrameBuffer();
    }
}

void HandleCartridgeIngame(void)
{
    // If the state switches directly from options to ingame the game was only paused and doesn't need to be loaded again.
    if (OS_LastState != OS_OPTIONS)
    {
        // Load the game directly from the cartridge.
        if (GBC_LoadFromCartridge() != GBC_LOAD_RESULT_OK)
        {
            LED_EnableRed(true);

            OS_DoAction(OS_SWITCH_TO_STATE_MAINPAGE);

            return;
        }
    }

    // If the game is not to be scaled draw the frame.
    if (!OS_Options.DrawScaled)
    {
        LCD_Brick_t brick;
        brick.Color        = UI_BRICK_COLOR;
        brick.Height       = UI_BRICK_HEIGHT;
        brick.Length       = UI_BRICK_LENGTH;
        brick.Border.Color = UI_BRICK_BORDER_COLOR;
        brick.Border.Width = UI_BRICK_BORDER_WIDTH;

        LCD_DrawWall(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y, false, &brick);
    }


    while (1)
    {
        // If select and start are pressed simultaneously in game pause it and go to options page.
        if (Input_Interrupt_Flags.ButtonSelect && Input_Interrupt_Flags.ButtonStart)
        {
            // Quickfix until drawing methods use dma.
            while (LCD_DMA_TransferCompleteFlags != LCD_DMA_TRANSFER_COMPLETE);

            // Lock all buttons until they are released so next page opens without anything pressed.
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again.
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            OS_DoAction(OS_SWITCH_TO_STATE_OPTIONS);

            return;
        }

        GBC_Update();

        while (Audio_IsPlayingOfBufferFinished == false);
        Audio_IsPlayingOfBufferFinished = false;

        if (OS_Options.DrawScaled)  LCD_DrawGBCFrameBufferScaled();
        else                        LCD_DrawGBCFrameBuffer();
    }
}

void os_state_handler(void)
{
	// All Handle-Functions contain an infinite loop and will only exit when the page is left or an error occurs.
	switch (OS_CurrState)
	{
        case OS_MAINPAGE:
            HandleMainPage();
            break;

        case OS_SHOW_ALL:
            HandleShowAllGamesPage();
            break;

        case OS_OPTIONS:
            HandleOptionPage();
            break;

        case OS_INGAME_FROM_SDC:
            HandleSDCIngame();
            break;

        case OS_INGAME_FROM_CARTRIDGE:
            HandleCartridgeIngame();
            break;

        default:
            return;
	}
}
