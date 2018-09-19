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

#include "lcd.h"
#include "lcd_init.h"

#include "os.h"
#include "os_state_handlers.h"

#include "cmod_init.h"
#include "input_init.h"

#include "gbc.h"
#include "led.h"
#include "sdc.h"
#include "audio.h"
#include "sdram.h"
#include "system.h"



int main(void)
{
    // Initialize library.
    HAL_Init();

    // Initialize system.
    System_Initialize();

    // Initialize drivers.
    SDRAM_Initialize();
    SDC_Initialize();
    OS_Initialize();
    LED_Initialize();
    LCD_Initialize();
    CMOD_Initialize();
    Audio_Initialize();
    Input_Initialize();

    // Turn on display.
    LCD_SetBrightness(OS_Options.Brightness);


    /* Main loop */
    while (1)
    {
        os_state_handler();
    }
}
