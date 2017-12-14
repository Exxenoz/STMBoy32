#include "audio.h"

uint16_t aSine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};

void Audio_GPIOConfig(void)
{
    RCC_AHB1PeriphClockCmd(AUDIO_BUS, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject = {0};
    GPIO_InitObject.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin = AUDIO_PIN;
    GPIO_InitObject.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_Init(AUDIO_PORT, &GPIO_InitObject);
}

void Audio_TimerConfig(void)
{
    RCC_APB1PeriphClockCmd(AUDIO_TIM_BUS, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseObject;
    TIM_TimeBaseStructInit(&TIM_TimeBaseObject);

    TIM_TimeBaseObject.TIM_Period = 4081;
    TIM_TimeBaseObject.TIM_Prescaler = 0;
    TIM_TimeBaseObject.TIM_ClockDivision = 0;
    TIM_TimeBaseObject.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(AUDIO_TIM, &TIM_TimeBaseObject);

    TIM_SelectOutputTrigger(AUDIO_TIM, TIM_TRGOSource_Update);

    TIM_Cmd(AUDIO_TIM, ENABLE);
}

void Audio_SinusConfig(void)
{
    RCC_APB1PeriphClockCmd(AUDIO_DAC_BUS,  ENABLE);
    RCC_AHB1PeriphClockCmd(AUDIO_DMA_BUS, ENABLE);

    DAC_InitTypeDef DAC_InitObject = {0};
    DAC_InitObject.DAC_Trigger = AUDIO_DAC_TRIGGER;
    DAC_InitObject.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitObject.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(AUDIO_DAC_CHANNEL, &DAC_InitObject);

    DMA_DeInit(AUDIO_DMA_STREAM);

    DMA_InitTypeDef DMA_InitObject = {0};
    DMA_InitObject.DMA_Channel = AUDIO_DMA_CHANNEL;  
    DMA_InitObject.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R1_ADDRESS;
    DMA_InitObject.DMA_Memory0BaseAddr = (uint32_t)&aSine12bit;
    DMA_InitObject.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitObject.DMA_BufferSize = 32;
    DMA_InitObject.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitObject.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitObject.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitObject.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitObject.DMA_Mode = DMA_Mode_Normal;
    DMA_InitObject.DMA_Priority = DMA_Priority_High;
    DMA_InitObject.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_InitObject.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitObject.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitObject.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_DMA_STREAM, &DMA_InitObject);

    DMA_Cmd(AUDIO_DMA_STREAM, ENABLE);

    DAC_Cmd(AUDIO_DAC_CHANNEL, ENABLE);

    DAC_DMACmd(AUDIO_DAC_CHANNEL, ENABLE);
}

void Audio_Initialize(void)
{
    Audio_GPIOConfig();
    Audio_TimerConfig();
    Audio_SinusConfig();
}
