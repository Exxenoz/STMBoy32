/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  STMBoy Team
  * @version V1.0
  * @date    30-August-2017
  * @brief   Main program body
  ******************************************************************************
  */

#include "os.h"
#include "os_state_handlers.h"

#include "gbc.h"
#include "gbc_mmu.h"

#include "led.h"
#include "sdc.h"
#include "cmod.h"
#include "audio.h"
#include "input_init.h"
#include "common.h"
#include "system.h"
#include "lcd_init.h"
#include "sdram.h"


int main(void)
{
    // Initialize library.
    HAL_Init();

    // Initialize system.
    System_Initialize();

    // Initialize drivers.
    SDRAM_Initialize();
    OS_Initialize();
    SDC_Initialize();
    LED_Initialize();
    LCD_Initialize();
    //CMOD_Initialize();
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
