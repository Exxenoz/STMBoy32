#include "sdc.h"
#include "led.h"

//FATFS SDC_FatFS;                    // Global FatFS object
bool  SDC_Mounted = false;          // Global mounted state

void SDC_InitializeInterrupts(void)
{
    /*NVIC_InitTypeDef NVIC_InitStructure;

    // SDIO Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel = SDC_SDIO_NVIC_CHANNEL;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // DMA2 STREAMx Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel = SDC_DMA_NVIC_CHANNEL;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);*/
}

void SDC_Initialize(void)
{
    SDC_InitializeInterrupts();
}

bool SDC_Mount(void)
{
    /*if (!SD_Detect())
    {
        return false;
    }*/

    if (SDC_Mounted)
    {
        SDC_Unmount();
    }

    //if (f_mount(&SDC_FatFS, "", 1) != FR_OK)
    {
        return false;
    }

    return SDC_Mounted = true;
}

bool SDC_IsMounted(void)
{
    return SDC_Mounted;
}

void SDC_Unmount(void)
{
    //f_mount(NULL, "", 1);
    SDC_Mounted = false;
}

/*void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();  
}

void SD_SDIO_DMA_IRQHANDLER(void)
{
    SD_ProcessDMAIRQ();  
}*/
