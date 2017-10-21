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
#include "gbc_mmu.h"

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

    LED_Initialize();
    Audio_Initialize();
    Input_Initialize();
    LCD_Initialize();
    CMOD_Initialize();
    SDC_Initialize();

    if (!GBC_MMU_LoadROM("red.gb"))
    {
        LED_EnableRed(true);
        return 0;
    }

    int g_KaroOffset = 0;

    /* Infinite loop */
    while (1)
    {
        if (CMOD_RISING_CLK_FLAG)
        {
            GPIO_ToggleBits(CMOD_WR_PORT, CMOD_WR_PIN);
            CMOD_RST_RISING_CLK_FLAG;
        }
        
        if(LCD_READY_FLAG)
        {
            if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
            {
                LCD_PrintKaro(0, g_KaroOffset++);
                
                /*for (long i = 0; i < 1000000; i++);
                if (g_KaroOffset == 0 || g_KaroOffset == 120)
                {
                    LCD_ClearColor(0x00FF);
                    g_KaroOffset = 0;
                }
                else if (g_KaroOffset == 60)
                {
                    LCD_ClearColor(0xFF00);
                }
                g_KaroOffset++;*/
            }
            LCD_RST_READY_FLAG;
        }
    }
}
