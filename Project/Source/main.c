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

OS_State_t currState = OS_MAIN_PAGE;
OS_State_t lastState = OS_MAIN_PAGE;

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
    // If no cartridge is detected first valid menu point is SHOW ALL GAMES (ID 1) else BOOT CARTRIDGE (ID 0)
    int firstMenuPointID = CMOD_Detect() ? 0 : 1;
    int lastMenuPointID  = 3;
    int currMenuPointID  = firstMenuPointID;    

    // Draw MainPage and highlight first valid menu point
    UI_DrawMainPage(firstMenuPointID);
    UI_MainPage_HightlightMenuPoint(currMenuPointID);

    // MainPage loop
    while (1)
    {
        // Update the lockState of all buttons 
        // Locks are needed because otherwise a short press would trigger multiple scroll downs
        Input_UpdateLocks();

        // If no cartridge is detected and BOOT CARTRIDGE is enabled disable it and vice versa
        if (!CMOD_Detect() && firstMenuPointID == 0)
        {
            UI_MainPage_DrawDisabledMenuPoint(0);
            firstMenuPointID = 1;
        }
        else if (CMOD_Detect() && firstMenuPointID == 1)
        {
            UI_MainPage_DrawMenuPoint(0);
            firstMenuPointID = 0;
        }

        // If Fade-Top is pressed and we don't have the first valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point above
        if (Input_Interrupt_Flags.FadeTop && !Input_IsLocked(INPUT_FADE_TOP_ID) && currMenuPointID > firstMenuPointID)
        {
            UI_MainPage_DrawMenuPoint(currMenuPointID);
            currMenuPointID--;
            UI_MainPage_HightlightMenuPoint(currMenuPointID);

            Input_Lock(INPUT_FADE_TOP_ID, INPUT_LOCK_UNTIL_RELEASED);
        }

        // If Fade-Bot is pressed and we don't have the last valid menu point already selected,
        // reset highlighting of current menu point and highlight the menu point below
        if (Input_Interrupt_Flags.FadeBot && !Input_IsLocked(INPUT_FADE_BOT_ID) && currMenuPointID < lastMenuPointID)
        {
            UI_MainPage_DrawMenuPoint(currMenuPointID);
            currMenuPointID++;
            UI_MainPage_HightlightMenuPoint(currMenuPointID);

            Input_Lock(INPUT_FADE_BOT_ID, INPUT_LOCK_UNTIL_RELEASED);
        }
        
        // If A-Button is pressed confirm the current selection by changing states and ending the infinite loop
        if (Input_Interrupt_Flags.ButtonA && !Input_IsLocked(INPUT_A_ID))
        {
            lastState = currState;
            currState = UI_MainPage_MenuPoints[currMenuPointID].NewStateOnPress;

            // Lock all buttons until they are released so next page opens without anything pressed
            // If a button is not pressed at this thime UpdateLocks will immediately disable the lock again
            Input_LockAll(INPUT_LOCK_UNTIL_RELEASED);

            break;
        }
    }
}

void HandleShowAllGamesPage(void)
{
    // YTBI
    while (1)
    {
        
    }
}

void HandleShowFavoritesPage(void)
{
    // YTBI
    while (1)
    {
        
    }
}

void HandleOptionPage(void)
{
    // YTBI
    while (1)
    {
        
    }
}

bool HandleSDCIngame(void)
{
    if(GBC_LoadFromCartridge() != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return false;
    }

    while (1)
    {
        GBC_Update();

        while (!LCD_READY_FLAG);
        LCD_DrawGBCFrameBuffer();
    }
}

bool HandleCartridgeIngame(void)
{
    if(GBC_LoadFromSDC("tetris.gb") != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return false;
    }

    while (1)
    {
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

    // Turn on Display
    LCD_DimBacklight(0);

    // Initialize UI
    UI_Initialize();

    currState = OS_MAIN_PAGE;


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
        switch (currState)
        {
            case OS_MAIN_PAGE:
                HandleMainPage();
                break;
            case OS_SHOW_ALL:
                HandleShowAllGamesPage();
                break;
            case OS_SHOW_FAV:
                HandleShowFavoritesPage();
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
