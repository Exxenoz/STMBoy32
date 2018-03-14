#include "audio.h"

TIM_HandleTypeDef Audio_TimerHandle = { .Instance = AUDIO_TIM };
DMA_HandleTypeDef DMA_Handle;

void Audio_InitializeGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitObject = {0};
    GPIO_InitObject.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin   = AUDIO_PIN;
    GPIO_InitObject.Pull  = GPIO_NOPULL;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 
    HAL_GPIO_Init(AUDIO_PORT, &GPIO_InitObject);
}

void Audio_InitializeTimer(void)
{
    Audio_TimerHandle.Init.Period        = 5626; // 32 kHz
    Audio_TimerHandle.Init.Prescaler     = 0;
    Audio_TimerHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    Audio_TimerHandle.Init.ClockDivision = 0;

    //TIM_SelectOutputTrigger(AUDIO_TIM, TIM_TRGOSource_Update);

    HAL_TIM_Base_Init(&Audio_TimerHandle);
    HAL_TIM_Base_Start(&Audio_TimerHandle);
}

void Audio_InitializeDAC(void)
{
    __HAL_RCC_DAC12_CLK_ENABLE();

    //DAC_InitTypeDef DAC_InitObject = {0};
    //DAC_InitObject.DAC_Trigger = AUDIO_DAC_TRIGGER;
    //DAC_InitObject.DAC_WaveGeneration = DAC_WaveGeneration_None;
    //DAC_InitObject.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    //DAC_Init(AUDIO_DAC_CHANNEL, &DAC_InitObject);
    //
    //DAC_Cmd(AUDIO_DAC_CHANNEL, ENABLE);
    //
    //DAC_DMACmd(AUDIO_DAC_CHANNEL, ENABLE);
}

void Audio_Initialize(void)
{
    Audio_InitializeGPIO();
    Audio_InitializeTimer();
    Audio_InitializeDAC();
}

void Audio_SetAudioBuffer(uint16_t* audioBuffer, uint32_t audioBufferSize)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    //DMA_DeInit(AUDIO_DMA_STREAM);
    //
    //DMA_InitTypeDef DMA_InitObject = {0};
    //DMA_InitObject.DMA_Channel = AUDIO_DMA_CHANNEL;  
    //DMA_InitObject.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R1_ADDRESS;
    //DMA_InitObject.DMA_Memory0BaseAddr = (uint32_t)audioBuffer;
    //DMA_InitObject.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    //DMA_InitObject.DMA_BufferSize = audioBufferSize;
    //DMA_InitObject.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //DMA_InitObject.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //DMA_InitObject.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    //DMA_InitObject.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    //DMA_InitObject.DMA_Mode = DMA_Mode_Circular;
    //DMA_InitObject.DMA_Priority = DMA_Priority_High;
    //DMA_InitObject.DMA_FIFOMode = DMA_FIFOMode_Disable;
    //DMA_InitObject.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    //DMA_InitObject.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    //DMA_InitObject.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    //DMA_Init(AUDIO_DMA_STREAM, &DMA_InitObject);
    //
    //DMA_Cmd(AUDIO_DMA_STREAM, ENABLE);
}
