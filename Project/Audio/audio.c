#include "audio.h"

void Audio_InitializeGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitObject = {0};
    GPIO_InitObject.Mode = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin = AUDIO_PIN;
    GPIO_InitObject.Pull = GPIO_NOPULL;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 
    HAL_GPIO_Init(AUDIO_PORT, &GPIO_InitObject);
}

void Audio_InitializeTimer(void)
{
    RCC_APB2PeriphClockCmd(AUDIO_TIM_BUS, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseObject;
    TIM_TimeBaseStructInit(&TIM_TimeBaseObject);

    TIM_TimeBaseObject.TIM_Period = 5626; // 32 kHz
    TIM_TimeBaseObject.TIM_Prescaler = 0;
    TIM_TimeBaseObject.TIM_ClockDivision = 0;
    TIM_TimeBaseObject.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(AUDIO_TIM, &TIM_TimeBaseObject);

    TIM_SelectOutputTrigger(AUDIO_TIM, TIM_TRGOSource_Update);

    TIM_Cmd(AUDIO_TIM, ENABLE);
}

void Audio_InitializeDAC(void)
{
    __HAL_RCC_DAC12_CLK_ENABLE();

    DAC_InitTypeDef DAC_InitObject = {0};
    DAC_InitObject.DAC_Trigger = AUDIO_DAC_TRIGGER;
    DAC_InitObject.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitObject.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(AUDIO_DAC_CHANNEL, &DAC_InitObject);

    DAC_Cmd(AUDIO_DAC_CHANNEL, ENABLE);

    DAC_DMACmd(AUDIO_DAC_CHANNEL, ENABLE);
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

    DMA_DeInit(AUDIO_DMA_STREAM);

    DMA_InitTypeDef DMA_InitObject = {0};
    DMA_InitObject.DMA_Channel = AUDIO_DMA_CHANNEL;  
    DMA_InitObject.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R1_ADDRESS;
    DMA_InitObject.DMA_Memory0BaseAddr = (uint32_t)audioBuffer;
    DMA_InitObject.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitObject.DMA_BufferSize = audioBufferSize;
    DMA_InitObject.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitObject.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitObject.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitObject.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitObject.DMA_Mode = DMA_Mode_Circular;
    DMA_InitObject.DMA_Priority = DMA_Priority_High;
    DMA_InitObject.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitObject.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitObject.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitObject.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_DMA_STREAM, &DMA_InitObject);

    DMA_Cmd(AUDIO_DMA_STREAM, ENABLE);
}
