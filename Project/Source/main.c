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

OS_STATE_t currState = OS_MAIN_PAGE;
OS_STATE_t lastState = OS_MAIN_PAGE;

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

//----------DEBUG----------
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitObject);
    GPIO_ResetBits(GPIOA, GPIO_Pin_5);

    CMOD_SaveResult_t result;
    GBC_LoadResult_t loaded = GBC_LoadFromCartridge();
    if(loaded == GBC_LOAD_RESULT_OK)
    {
        /*uint8_t bank1 = 0x01;
        memset(test, 0xFF, 1024);
        memset(testResult, 0x00, 1024);
        CMOD_WriteByte(0x4001, &bank1);
        CMOD_WriteBytes(0xA000, 1024, test);
        CMOD_ReadBytes(0xA000, 1024, testResult);
        while (CMOD_GetStatus() == CMOD_PROCESSING);
        NOP;*/
       /*result = CMOD_SaveCartridge(true);
       if(result == CMOD_SUCCESS)
       {
            LED_EnableBlue(true);
       }
       else
       {
            LED_EnableRed(true);
       }*/
    }
    else if (GBC_LoadFromSDC("tetris.gb") != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return 0;
    }

    Fonts_InitializeSTMFonts();
    currState = OS_MAIN_PAGE;
//--------------------------

    /* Infinite loop */
    while (1)
    {
        switch (currState)
        {
            case OS_MAIN_PAGE:

                UI_DrawMainPage();

                while (1)
                {
                    
                }
                break;
            
            case OS_SHOW_ALL:
                while (1)
                {
                    
                }
                break;
            
            case OS_SHOW_FAV:
                while (1)
                {
                    
                }
                break;
            
            case OS_OPTIONS:
                while (1)
                {
                    
                }
                break;
            
            case OS_INGAME:
                while (1)
                {
                    //GPIO_ToggleBits(GPIOA, GPIO_Pin_5);
                    GBC_Update();
                    //GPIO_ToggleBits(GPIOA, GPIO_Pin_5);

                    while (!LCD_READY_FLAG);
                    LCD_DrawFrameBufferScaled();
                }
                break;
                
            default:
                return 0;
        }
    }
}
