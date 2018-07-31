#include "os_state_handlers.h"



void HandleMainPage(void)
{
    // If no cartridge is detected first valid menupoint is SHOW ALL GAMES (ID 1) else BOOT CARTRIDGE (ID 0)
    int firstMenuPointID = CMOD_Detect() ? 0 : 1;
    int lastMenuPointID  = 2;
    int currMenuPointID  = firstMenuPointID;

    // Draw MainPage
    UI_DrawMainPage(firstMenuPointID);

    // MainPage loop
    while (1)
    {
        // Update the lockState of all buttons 
        // Locks are needed because otherwise a short press would trigger multiple scroll downs
        Input_UpdateLocks();

        // If no cartridge is detected and BOOT CARTRIDGE is enabled disable it and vice versa
        if (!CMOD_Detect() && firstMenuPointID == 0)
        {
					  UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_DISABLED);
            firstMenuPointID = 1;
        }
        else if (CMOD_Detect() && firstMenuPointID == 1)
        {
						UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]), UI_ENABLED);
            firstMenuPointID = 0;
        }

        // If Fade-Top is pressed and we don't have the first valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point above
        if (Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_TOP_ID) && currMenuPointID > firstMenuPointID)
        {
						UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_ENABLED);

            currMenuPointID--;

						UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_TOP_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }

        // If Fade-Bot is pressed and we don't have the last valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point below
        if (Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_BOT_ID) && currMenuPointID < lastMenuPointID)
        {
						UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_ENABLED);

            currMenuPointID++;

						UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currMenuPointID]), UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_BOT_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }
        
        // If A-Button is pressed confirm the current selection and end the infinite loop
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID))
        {
            // Perform the action linked to the selected menupoint
						OS_DoAction(UI_MainPage_MenuPoints[currMenuPointID].Action);

            // Lock all buttons until they are released so next page opens without anything pressed
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            break;
        }
    }
}

void HandleShowAllGamesPage(void)
{
    // ?
    UI_ShowAllDesign_t designs[UI_NUMBER_OF_SHOWALL_MPS] = { UI_ALLGAMES, UI_FAVORITES, UI_LASTPLAYED };
    int currDesignID = 0;

    // Draw the page
    UI_DrawShowAllPage(designs[currDesignID]);

    // Initialize IDs
    int lastGameEntryID     = OS_TotalGamesCounter - 1;
    int currGameEntryID     = 0;
    int currGameEntryIndex  = 0;
    int currGameEntryListID = 0;

    while (1)
    {
        // Update the lockState of all buttons 
        // Locks are needed because otherwise a short press would trigger multiple scroll downs
        Input_UpdateLocks();

        // If Fade-Right is pressed switch design accordingly
        if (Input_Interrupt_Flags.FadeRight && !Input_IsLocked(INPUT_FADE_RIGHT_ID))
        {
            if (currDesignID == UI_NUMBER_OF_SHOWALL_MPS - 1) currDesignID = 0;
            else                                              currDesignID++;

            UI_DrawShowAllPage(designs[currDesignID]);

            // Re-Initialize IDs
            lastGameEntryID     = OS_TotalGamesCounter - 1;
            currGameEntryID     = 0;
            currGameEntryIndex  = 0;
            currGameEntryListID = 0;

            Input_Lock(INPUT_FADE_RIGHT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If Fade-Left is pressed switch design accordingly
        if (Input_Interrupt_Flags.FadeLeft && !Input_IsLocked(INPUT_FADE_LEFT_ID))
        {
            if (currDesignID == 0) currDesignID = UI_NUMBER_OF_SHOWALL_MPS - 1;
            else                   currDesignID--;

            UI_DrawShowAllPage(designs[currDesignID]);

            // Re-Initialize IDs
            lastGameEntryID     = OS_TotalGamesCounter - 1;
            currGameEntryID     = 0;
            currGameEntryIndex  = 0;
            currGameEntryListID = 0;

            Input_Lock(INPUT_FADE_LEFT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If Fade-Top is pressed & Fade-Bot not & we don't have the first valid entry already selected, scroll up
        if (Input_Interrupt_Flags.FadeTop && !Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_TOP_ID)
            && currGameEntryID > 0)
        {
            UI_ScrollGames(&currGameEntryIndex, &currGameEntryListID, UI_SCROLLUP);
            currGameEntryID--;
            UI_DrawScrollBar(currGameEntryID);

            Input_LockDynamically(INPUT_FADE_TOP_ID);
        }

        // If Fade-Bot is pressed & Fade-Top not & we don't have the last valid entry already selected, scroll down
        if (Input_Interrupt_Flags.FadeBot && !Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_BOT_ID)
            && currGameEntryID < lastGameEntryID)
        {
            UI_ScrollGames(&currGameEntryIndex, &currGameEntryListID, UI_SCROLLDOWN);
            currGameEntryID++;
            UI_DrawScrollBar(currGameEntryID);

            Input_LockDynamically(INPUT_FADE_BOT_ID);
        }

        // If Select-Button is pressed invert the favorite-status of the currently selected game (if valid)
        if (Input_Interrupt_Flags.ButtonSelect && !Input_IsLocked(INPUT_SELECT_ID) && currGameEntryID != -1)
        {
            OS_InvertFavoriteStatus(&OS_GameEntries[currGameEntryIndex]);

            // If only favorites are displayed this means deleting the currently selected game from the list
            if (designs[currDesignID] == UI_FAVORITES)
            {
                OS_RemoveGameEntry(currGameEntryIndex);

                // If the last GE was selected previously now the previous GE is selected, otherwise the next
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
            // If all games or last played are displayed only the favorite-indication (star) changes
            else
            {
                UI_ReDrawCurrGE(currGameEntryIndex, currGameEntryListID);
            }

            Input_Lock(INPUT_SELECT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If A-Button is pressed confirm the current selection (if valid) and end the infinite loop
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID) && currGameEntryID != -1)
        {
            // Set the game which is to be started
            CopyString(OS_CurrentGame.Name, OS_GameEntries[currGameEntryIndex].Name, OS_MAX_GAME_TITLE_LENGTH + 1);
            OS_CurrentGame.IsFavorite = OS_GameEntries[currGameEntryIndex].IsFavorite;

            // Update the last played games accordingly
            OS_UpdateLastPlayed();

            // Switch state
            OS_DoAction(OS_SWITCH_TO_STATE_INGAME_FSD);
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            break;
        }

        // If B-Button is pressed switch to previous state and end the infinite loop
        if (Input_Interrupt_Flags.ButtonB && !Input_IsLocked(INPUT_B_ID))
        {
            OS_DoAction(OS_SWITCH_TO_PREVIOUS_STATE);
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);
            break;
        }
    }
}

void HandleOptionPage(void)
{
    // YTBI
    while (1)
    {
        OS_CurrState = OS_MAIN_PAGE;
        break;
    }
}

bool HandleSDCIngame(void)
{
    // Define path array with maximal needed size and get the game path
    char path[OS_MAX_PATH_LENGTH];

    OS_GetGamePath(&OS_CurrentGame, path, OS_MAX_PATH_LENGTH);

    // Load the game
    if(GBC_LoadFromSDC(path) != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return false;
    }

    while (1)
    {
        // If select and start are pressed simultaneously in game pause it and go to options page
        if (Input_Interrupt_Flags.ButtonSelect && Input_Interrupt_Flags.ButtonStart)
        {
            //ToDo: Pause?

            OS_DoAction(OS_SWITCH_TO_STATE_OPTIONS);

            // Lock all buttons until they are released so options opens without anything pressed
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            return true;
        }

        GBC_Update();

        while (Audio_IsPlayingOfBufferFinished == false);
        Audio_IsPlayingOfBufferFinished = false;

        LCD_DrawGBCFrameBufferScaled();
    }
}

bool HandleCartridgeIngame(void)
{
    if (GBC_LoadFromCartridge() != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return false;
    }

    while (1)
    {
        // If select and start are pressed simultaneously in game pause it and go to options page
        if (Input_Interrupt_Flags.ButtonSelect && Input_Interrupt_Flags.ButtonStart)
        {
            //ToDo: Pause?

            OS_DoAction(OS_SWITCH_TO_STATE_OPTIONS);

            // Lock all buttons until they are released so next page opens without anything pressed
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            return true;
        }

        GBC_Update();

        while (Audio_IsPlayingOfBufferFinished == false);
        Audio_IsPlayingOfBufferFinished = false;

        LCD_DrawGBCFrameBufferScaled();
    }
}

void os_state_handler(void)
{
	// All Handle-Functions contain an infinite loop and will only exit when the page is left or an error occurs
	switch (OS_CurrState)
	{
			case OS_MAIN_PAGE:
					HandleMainPage();
					break;

			case OS_SHOW_ALL:
					HandleShowAllGamesPage();
					break;

			case OS_OPTIONS:
					HandleOptionPage();
					break;

			case OS_INGAME_FROM_SDC:
					if (HandleSDCIngame() == false) return;
					break;

			case OS_INGAME_FROM_CARTRIDGE:
					if (HandleCartridgeIngame() == false) return;
					break;

			default:
					return;
	}
}
