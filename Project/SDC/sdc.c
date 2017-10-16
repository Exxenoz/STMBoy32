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
    
    long status = SD_Init();
    
    if (status != SD_OK) return;
    
    BYTE buffer[512] = {0};
    status = SD_ReadBlock(buffer, 0, 512);
    status = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    
    if (status != SD_OK) return;

    long result = f_mount(&g_FatFs, "", 1);
    // Mount drive
    if (result == FR_OK)
    {
        // Mounted OK, turn on RED LED
        LED_EnableRed(true);

        // Try to open file
        if (f_open(&g_File, "TEST.TXT", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
        {
            // File opened, turn off RED and turn on GREEN led
            LED_EnableGreen(true);

            f_mkdir("Hallo");
            
            // Close file, don't forget this!
            f_close(&g_File);
        }

        // Unmount drive, don't forget this!
        f_mount(0, "", 1);
    }

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
