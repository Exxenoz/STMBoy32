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
#include "input.h"

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
    //ClockDebug_Initialize();

    Input_Initialize();

    LCD_Initialize();

    int i;
    uint16_t red = 0xF000;
    uint16_t green = 0x0F00;
    uint16_t blue = 0x000F;
    /* Infinite loop */
    while (1)
    {
        LCD_ClearColor(red);
        for(i=0; i< 10000000; i++);
        LCD_ClearColor(green);
        for(i=0; i< 10000000; i++);
        LCD_ClearColor(blue);
        for(i=0; i< 10000000; i++);
    }
}
