#include "sdc.h"

void SDC_GPIOConfig()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitObject;
    
    #define INITIALIZE_SDIO_PIN(PORT, PIN)                       \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;                   \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;                  \
    GPIO_InitObject.GPIO_Pin   = PIN;                            \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_UP;                   \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;              \
    GPIO_Init(PORT, &GPIO_InitObject);                           \
  
    INITIALIZE_SDIO_PIN(SDC_DAT0_PORT, SDC_DAT0_PIN);
    INITIALIZE_SDIO_PIN(SDC_DAT1_PORT, SDC_DAT1_PIN);
    INITIALIZE_SDIO_PIN(SDC_DAT2_PORT, SDC_DAT2_PIN);
    INITIALIZE_SDIO_PIN(SDC_DAT3_PORT, SDC_DAT3_PIN);
    INITIALIZE_SDIO_PIN(SDC_CLK_PORT,  SDC_CLK_PIN);
    INITIALIZE_SDIO_PIN(SDC_CMD_PORT,  SDC_CMD_PIN);

    GPIO_PinAFConfig(SDC_DAT0_PORT, SDC_DAT0_PIN_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SDC_DAT1_PORT, SDC_DAT1_PIN_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SDC_DAT2_PORT, SDC_DAT2_PIN_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SDC_DAT3_PORT, SDC_DAT3_PIN_SOURCE, GPIO_AF_SDIO);
    GPIO_PinAFConfig(SDC_CLK_PORT,  SDC_CLK_PIN_SOURCE,  GPIO_AF_SDIO);
    GPIO_PinAFConfig(SDC_CMD_PORT,  SDC_CMD_PIN_SOURCE,  GPIO_AF_SDIO);
}

void SDC_SDIOConfig()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);
    
    SDIO_InitTypeDef SDIO_InitObject;

    SDIO_InitObject.SDIO_ClockDiv = 150;
    SDIO_InitObject.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
    SDIO_InitObject.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
    SDIO_InitObject.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
    SDIO_InitObject.SDIO_BusWide = SDIO_BusWide_1b;
    SDIO_InitObject.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
    SDIO_Init(&SDIO_InitObject);
    
    SDIO_SetPowerState(SDIO_PowerState_ON);
    SDIO_ClockCmd(ENABLE);
}

void SDC_InterruptConfig()
{
    NVIC_InitTypeDef NVIC_InitObject;
    
    #define INITIALIZE_NVIC_CHANNEL(CHANNEL)                     \
    NVIC_InitObject.NVIC_IRQChannel                   = CHANNEL; \
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0x0F;    \
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0x0F;    \
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;  \
    NVIC_Init(&NVIC_InitObject);                                 \
    NVIC_EnableIRQ(CHANNEL);                                     \
    
    SDIO_ITConfig(SDIO_IT_SDIOIT, ENABLE);
}

void SDC_DMAConfig(void)
{
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1, ENABLE);

    DMA_InitTypeDef  DMA_InitObject;
    
    DMA_InitObject.DMA_Channel = 0;
    DMA_InitObject.DMA_PeripheralBaseAddr = 0;
    DMA_InitObject.DMA_Memory0BaseAddr = 0;
    DMA_InitObject.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitObject.DMA_BufferSize = 0;
    DMA_InitObject.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitObject.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitObject.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitObject.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitObject.DMA_Mode = DMA_Mode_Normal;
    DMA_InitObject.DMA_Priority = DMA_Priority_Low;
    DMA_InitObject.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitObject.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitObject.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitObject.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream1, &DMA_InitObject);

    SDIO_DMACmd(ENABLE);
    DMA_Cmd(DMA1_Stream1, ENABLE); 
}

void SDC_Initialize()
{
   SDC_GPIOConfig();
   SDC_SDIOConfig();
  // SDC_DMAConfig();

  // SDC_InterruptConfig();
}
