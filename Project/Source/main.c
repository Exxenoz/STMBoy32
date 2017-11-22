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
#include "string.h"
uint8_t test[1024];
uint8_t testResult[1024];

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
    // If no cartridge is detected first valid menu point is SHOW ALL GAMES
    int firstMenuPointID   = CMOD_Detect() ? 0 : 1;
    int currentMenuPointID = firstMenuPointID;

    // Draw MainPage and highlight first valid menu point
    UI_DrawMainPage();
    UI_HightlightMenuPoint(&(UI_MainPage_MenuPoints[currentMenuPointID]));

    // MainPage loop
    while (1)
    {
        // If no cartridge is detected and BOOT CARTRIDGE is enabled disable it and vice versa
        if (!CMOD_Detect() && firstMenuPointID == 0)
        {
            UI_DrawDisabledMenuPoint(&(UI_MainPage_MenuPoints[0]));
            firstMenuPointID = 1;
        }
        else if (CMOD_Detect() && firstMenuPointID == 1)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[0]));
            firstMenuPointID = 0;
        }

        // If Fade-Top is pressed and we don't have the first valid menu point already selected
        // highlight the menu point above and reset highlighting of current menu point
        if (Input_Interrupt_Flags.FadeTop == INPUT_PRESSED && currentMenuPointID > firstMenuPointID)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currentMenuPointID]));
            currentMenuPointID--;
            UI_HightlightMenuPoint(&(UI_MainPage_MenuPoints[currentMenuPointID]));
        }

        // If Fade-Bot is pressed and we don't have the last valid menu point already selected
        // highlight the menu point below and reset highlighting of current menu point
        if (Input_Interrupt_Flags.FadeBot == INPUT_PRESSED && currentMenuPointID < 4)
        {
            UI_DrawMenuPoint(&(UI_MainPage_MenuPoints[currentMenuPointID]));
            currentMenuPointID++;
            UI_HightlightMenuPoint(&(UI_MainPage_MenuPoints[currentMenuPointID]));
        }
        
        // If A-Button is pressed change the current and last states accordingly and leave main menu
        if (Input_Interrupt_Flags.ButtonA == INPUT_PRESSED)
        {
            lastState = currState;
            currState = UI_MainPage_MenuPoints[currentMenuPointID].newStateOnPress;
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
        LCD_DrawFrameBufferScaled();
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
        LCD_DrawFrameBufferScaled();
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
    Fonts_InitializeSTMFonts();

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
