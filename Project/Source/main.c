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

    int KaroOffset = 0;

    LED_Initialize();
    Audio_Initialize();
    Input_Initialize();
    LCD_Initialize();
    CMOD_Initialize();
    SDC_Initialize();
    
    if(CMOD_Detect())
    {
        CARTRIDGE_SPECS specs;
        CMOD_GetCSpecs(&specs);
        
        uint8_t data1[128];
        uint8_t data2[128];
        uint8_t data3[128];
        uint8_t romBank1 = 0x01;
        uint8_t romBank2 = 0x02;
        uint8_t romBank3 = 0x03;
        
        // Read the first 128 Bytes of Rom-Bank 1
        CMOD_WriteByte(0x2001, &romBank1);
        CMOD_ReadBytes(0x4000, 128, data1);
        
        // If a card with MBC3 is inserted enable its RAM, write to / read from it and disable it again
        if(specs.C_Mbc == C_MBC3)
        {
            uint8_t enableRAM = 0x0A;
            uint8_t disableRAM = 0x00;
            uint8_t ramBank1  = 0x01;
            uint8_t nintendoLogo[48];
            uint8_t ramData[48] = {0};
            
            CMOD_WriteByte(0x0001, &enableRAM);         // Enable RAM/RTC
            CMOD_WriteByte(0x4001, &ramBank1);          // Select RAM Bank 1    
            CMOD_ReadBytes(0x0134, 48, nintendoLogo);   // Read the nintendo Logo
            CMOD_WriteBytes(0xA000, 48, nintendoLogo);  // Write the Logo into Ram Bank1
            CMOD_ReadBytes(0xA000, 48, ramData);        // Read the first 48 Bytes from Ram Bank1
            CMOD_WriteByte(0x0001, &disableRAM);        // Disable RAM/RTC
        }
        
        // Read the first 128 Bytes of Rom-Bank 2
        CMOD_WriteByte(0x2001, &romBank2);
        CMOD_ReadBytes(0x4000, 128, data2);
        
        // If a card with MBC3 is inserted latch current time to RTC Register then read it
        if(specs.C_Mbc == C_MBC3)
        {
            uint8_t enableRTC = 0x0A;
            uint8_t disableRTC = 0x00;
            uint8_t rtcRegister = 0x08;
            uint8_t rtcData;
            uint8_t writeTime[2] = {0x00, 0x01};
            
            CMOD_WriteBytes(0x6001, 2, writeTime);  // Write current Time to RTC
            CMOD_WriteByte(0x0001, &enableRTC);     // Enable RAM/RTC
            CMOD_WriteByte(0x4001, &rtcRegister);   // Select RTC
            CMOD_ReadByte(0xA000, &rtcData);        // Read from RTC
            CMOD_WriteByte(0x0001, &disableRTC);    // Disable RTC/RAM
        }

        // Read the first 128 Bytes of Rom-Bank 3
        CMOD_WriteByte(0x2001, &romBank3);
        CMOD_ReadBytes(0x4000, 128, data3);
        while(CMOD_GetStatus() == CMOD_PROCESSING);
    }
    
    if (!GBC_MMU_LoadROM("red.gb"))
    {
        LED_EnableRed(true);
        return 0;
    }

    /* Infinite loop */
    while (1)
    {   
        if(LCD_READY_FLAG)
        {
            if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
            {
                LCD_PrintKaro(0, KaroOffset++);
                if (KaroOffset == 2400) KaroOffset = 0;
                /*for (long i = 0; i < 1000000; i++);
                if (KaroOffset == 0 || KaroOffset == 120)
                {
                    LCD_ClearColor(0x00FF);
                    KaroOffset = 0;
                }
                else if (KaroOffset == 60)
                {
                    LCD_ClearColor(0xFF00);
                }
                KaroOffset++;*/
            }
            LCD_RST_READY_FLAG;
        } 
    }
}
