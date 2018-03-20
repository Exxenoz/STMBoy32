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

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
    while(1)
    {
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE BYPASS)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 4
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;

    /*!< Supply configuration update enable */
    MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

    /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if(ret != HAL_OK)
    {
        Error_Handler();
    }

    /* Select PLL as system clock source and configure  bus clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
    if(ret != HAL_OK)
    {
        Error_Handler();
    }
}

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
    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    /* STM32H7xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
    HAL_Init();

    /* Configure the system clock to 400 MHz */
    SystemClock_Config();

    // Enable _all_ busses
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

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
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitObject.Pin   = GPIO_PIN_5;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitObject);
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
