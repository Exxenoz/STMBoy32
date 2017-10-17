#include "sdc.h"
#include "led.h"
#include "ff.h"
#include "stm324x9i_eval_sdio_sd.h"
#include "stdlib.h"

/* Fatfs object */
FATFS g_FatFs;
/* File object */
FIL g_File;

void SDC_InitializeInterrupts(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // SDIO Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // DMA2 STREAMx Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

BYTE memoryBlock[16384];

void SDC_Initialize()
{
    SDC_InitializeInterrupts();
    
    GPIO_InitTypeDef GPIO_InitObject;
    #define INITIALIZE_GPIO_PIN(PORT, PIN)                       \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;                  \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;                  \
    GPIO_InitObject.GPIO_Pin   = PIN;                            \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;               \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;              \
    GPIO_Init(PORT, &GPIO_InitObject);                           \
    INITIALIZE_GPIO_PIN(GPIOA, GPIO_Pin_3);

    if (f_mount(&g_FatFs, "", 1) == FR_OK)
    {
        if (f_open(&g_File, "red.gb", FA_OPEN_ALWAYS | FA_READ) == FR_OK)
        {
            UINT bytesRead = 0;
            GPIOA->ODR ^= GPIO_Pin_3;
            if (f_read(&g_File, memoryBlock, 16384, &bytesRead) == FR_OK)
            {
                GPIOA->ODR ^= GPIO_Pin_3;
                LED_EnableGreen(true);
            }

            f_close(&g_File);
        }
        
        f_mount(NULL, "", 1);
    }
}

void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();  
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    SD_ProcessDMAIRQ();  
}   
