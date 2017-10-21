#include "sdc.h"
#include "led.h"
#include "stm324x9i_eval_sdio_sd.h"

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
}

void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();  
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    SD_ProcessDMAIRQ();  
}   
