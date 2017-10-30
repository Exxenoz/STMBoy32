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

    int KaroOffset = 0;

    LED_Initialize();
    Audio_Initialize();
    Input_Initialize();
    LCD_Initialize();
    CMOD_Initialize();
    SDC_Initialize();
    
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
    else if (GBC_LoadFromSDC("cpu.gb") != GBC_LOAD_RESULT_OK)
    {
        LED_EnableRed(true);
        return 0;
    }

    /* Infinite loop */
    while (1)
    {
        //GPIO_ToggleBits(GPIOA, GPIO_Pin_5);
        GBC_Update();
        //GPIO_ToggleBits(GPIOA, GPIO_Pin_5);

        while (!LCD_READY_FLAG);

        if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
        {
            //LCD_PrintKaro(0, KaroOffset++);
            //if (KaroOffset == 2400) KaroOffset = 0;
            LCD_DrawFrameBuffer();
        }
        LCD_RST_READY_FLAG;
    }
}
