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

int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */
    
    LCD_Initialize();
    
    LCD_RST_CS;
    LCD_WriteAddr(0x2C);
    for (long i = 0; i < 320*240; i++)
    {
        if (i < 320*120)
        {
            LCD_WriteData(0x00FD);
        }
        else 
        {
            LCD_WriteData(0x0000);
        }
    }
    LCD_SET_CS;
    
    uint16_t madctl[2];
    LCD_ReadBuffer(0x0B, madctl, 2);
    
    /* Infinite loop */
    while (1)
    {
    }
}
