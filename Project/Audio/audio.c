#include "audio.h"

TIM_HandleTypeDef Audio_TimerHandle = { .Instance = AUDIO_TIM };
DAC_ChannelConfTypeDef Audio_ChannelConfig;
DAC_HandleTypeDef Audio_DACHandle = { .Instance = AUDIO_DAC };

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
    Audio_TimerHandle.Init.Period        = 3124; // 32 kHz
    Audio_TimerHandle.Init.Prescaler     = 0;
    Audio_TimerHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    Audio_TimerHandle.Init.ClockDivision = 0;
    HAL_TIM_Base_Init(&Audio_TimerHandle);

    TIM_MasterConfigTypeDef masterConfig;
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&Audio_TimerHandle, &masterConfig);

    HAL_TIM_Base_Start(&Audio_TimerHandle);
}

void Audio_Initialize(void)
{
    __HAL_RCC_DAC12_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    Audio_InitializeGPIO();
    Audio_InitializeTimer();
}

void Audio_SetAudioBuffer(uint16_t* audioBuffer, uint32_t audioBufferSize)
{
    HAL_DAC_DeInit(&Audio_DACHandle);
    HAL_DAC_Init(&Audio_DACHandle);

    Audio_ChannelConfig.DAC_Trigger = AUDIO_DAC_TRIGGER;
    Audio_ChannelConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfig, AUDIO_DAC_CHANNEL);
    HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_CHANNEL, (uint32_t *)audioBuffer, audioBufferSize, DAC_ALIGN_8B_R);
}
