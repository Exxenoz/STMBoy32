#include "sdc.h"
#include "led.h"
#include "ff.h"
#include "stm324x9i_eval_sdio_sd.h"

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

void SDC_Initialize()
{
    SDC_InitializeInterrupts();

    uint8_t buffer[512] = {0};
    SD_Error status = SD_Init();
    
    if (status != 0) return;

    SD_CardInfo info;
    status = SD_GetCardInfo(&info);
    
    if (status != 0) return;

    while (1)
    {
        status = SD_ReadBlock(buffer, 0, 512);
        status = SD_WaitReadOperation();
        while(SD_GetStatus() != SD_TRANSFER_OK);

        if (buffer[510] != 0x55 || buffer[511] != 0xAA || status != SD_OK)
        {
            return;
        }
    }
    
    if (status != 0) return;

    /*SD_CardStatus s;
    status = SD_GetCardStatus(&s);

    if (status != 0) return;*/

    LED_EnableGreen(true);
}

void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();  
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    SD_ProcessDMAIRQ();  
}   
