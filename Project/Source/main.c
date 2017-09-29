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
    SDC_Initialize();
    
    uint32_t volatile data = 0;

    SDIO_CmdInitTypeDef SDIO_CmdInitObject;
    
    SDIO_CmdInitObject.SDIO_Argument = 0x0;
    SDIO_CmdInitObject.SDIO_CmdIndex = 0x0;
    SDIO_CmdInitObject.SDIO_Response = SDIO_Response_No;
    SDIO_CmdInitObject.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitObject.SDIO_CPSM = SDIO_CPSM_Disable;
    SDIO_SendCommand(&SDIO_CmdInitObject);
    
    SDIO_CmdInitObject.SDIO_Argument = 0x0;
    SDIO_CmdInitObject.SDIO_CmdIndex = 2;
    SDIO_CmdInitObject.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitObject.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitObject.SDIO_CPSM = SDIO_CPSM_Disable;
    SDIO_SendCommand(&SDIO_CmdInitObject);
    
    data = 1;
    data = SDIO_GetResponse(SDIO_RESP1);
    data = SDIO_GetResponse(SDIO_RESP2);
    data = SDIO_GetResponse(SDIO_RESP3);
    data = SDIO_GetResponse(SDIO_RESP4);
    data = SDIO_GetCommandResponse();
    
    data = data + 1;

    /* Infinite loop */
    while (1)
    {
    }
}
