/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  STMBoy Team
  * @version V1.0
  * @date    30-August-2017
  * @brief   Main program body
  ******************************************************************************
  */

#include "common.h"
#include "audio.h"
#include "led.h"
#include "lcd.h"
#include "input.h"
#include "sdc.h"
#include "cmod.h"
#include "gbc.h"
#include "gbc_mmu.h"
#include "ui.h"
#include "os.h"

//----------DEBUG----------
void ClockDebug_Initialize()
{
    RCC->CFGR |= RCC_CFGR_MCO2PRE_2;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitObject.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitObject);
}
//-------------------------

void HandleMainPage(void)
{
    // Initialize Fonts needed for this Page
    Fonts_InitializeSTMFonts();

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
            UI_DrawMainPageMenuPoint(0, UI_DISABLED);
            firstMenuPointID = 1;
        }
        else if (CMOD_Detect() && firstMenuPointID == 1)
        {
            UI_DrawMainPageMenuPoint(0, UI_ENABLED);
            firstMenuPointID = 0;
        }

        // If Fade-Top is pressed and we don't have the first valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point above
        if (Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_TOP_ID) && currMenuPointID > firstMenuPointID)
        {
            UI_DrawMainPageMenuPoint(currMenuPointID, UI_ENABLED);
            currMenuPointID--;
            UI_DrawMainPageMenuPoint(currMenuPointID, UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_TOP_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }

        // If Fade-Bot is pressed and we don't have the last valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point below
        if (Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_BOT_ID) && currMenuPointID < lastMenuPointID)
        {
            UI_DrawMainPageMenuPoint(currMenuPointID, UI_ENABLED);
            currMenuPointID++;
            UI_DrawMainPageMenuPoint(currMenuPointID, UI_HIGHLIGHTED);

            Input_Lock(INPUT_FADE_BOT_ID, OS_MAIN_PAGE_BUTTON_LOCK_TIME);
        }
        
        // If A-Button is pressed confirm the current selection and end the infinite loop
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID))
        {
            // Perform the action linked to the selected menupoint
            UI_PerformMainPageAction(currMenuPointID);

            // Lock all buttons until they are released so next page opens without anything pressed
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            break;
        }
    }
}

void HandleShowAllGamesPage(void)
{
    // Initialize Fonts needed for this Page
    Fonts_InitializeSTMFonts();

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
            OS_UpdateFavorite(&OS_GameEntries[currGameEntryIndex]);

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
            copyString(OS_CurrentGame.Name, OS_GameEntries[currGameEntryIndex].Name, OS_MAX_GAME_TITLE_LENGTH + 1);
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
    // Initialize Fonts needed for this Page
    Fonts_InitializeSTMFonts();

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
    int  pathLength = sizeof(OS_FAVORITE_DIRECTORY) + OS_MAX_GAME_TITLE_LENGTH + 1;
    char path[pathLength];

    OS_GetGamePath(&OS_CurrentGame, path, pathLength);

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

        while (!LCD_READY_FLAG);
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

        while (!LCD_READY_FLAG);
        LCD_DrawGBCFrameBufferScaled();
    }
}

int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */
    
    // Uncomment the following line if you want
    // to debug clock frequency settings using GPIO
    // ClockDebug_Initialize();

    // Use all bits of interrupt priority register for preempt priority
    NVIC_SetPriorityGrouping(0U);

    // Initialize drivers
    LED_Initialize();
    Audio_Initialize();
    Input_Initialize();
    LCD_Initialize();
    CMOD_Initialize();
    SDC_Initialize();
    
    // Load initial options
    OS_LoadOptions();

    // Turn on Display
    LCD_DimBacklight(0);


//----------DEBUG----------
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitObject);
    GPIO_ResetBits(GPIOA, GPIO_Pin_5);
//--------------------------

    /* Infinite loop */
    while (1)
    {
        // Switch current state
        // All Handle-Functions contain an infinite loop and will only exit when the page is left
        // or an error occurs
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
                if (HandleSDCIngame() == false) return 0;
                break;

            case OS_INGAME_FROM_CARTRIDGE:
                if (HandleCartridgeIngame() == false) return 0;
                break;

            default:
                return 0;
        }
    }
}
