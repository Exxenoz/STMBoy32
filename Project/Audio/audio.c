#include "audio.h"

DAC_ChannelConfTypeDef Audio_ChannelConfig;
DAC_HandleTypeDef Audio_DACHandle;
TIM_HandleTypeDef Audio_TimerHandle;
DMA_HandleTypeDef Audio_DMAHandle;

void Audio_InitializeGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitObject = {0};
    GPIO_InitObject.Mode  = GPIO_MODE_ANALOG;
    GPIO_InitObject.Pin   = AUDIO_PIN;
    GPIO_InitObject.Pull  = GPIO_NOPULL;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 
    HAL_GPIO_Init(AUDIO_PORT, &GPIO_InitObject);
}

void Audio_InitializeTimer(void)
{
    __HAL_RCC_TIM8_CLK_ENABLE();

    Audio_TimerHandle.Instance = AUDIO_TIM;
    Audio_TimerHandle.State    = HAL_TIM_STATE_RESET;

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

void Audio_InitializeDAC(void)
{
    __HAL_RCC_DAC12_CLK_ENABLE();

    Audio_DACHandle.Instance = AUDIO_DAC;
    Audio_DACHandle.State    = HAL_DAC_STATE_RESET;

    HAL_DAC_Init(&Audio_DACHandle);

    Audio_ChannelConfig.DAC_Trigger      = AUDIO_DAC_TRIGGER;
    Audio_ChannelConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfig, AUDIO_DAC_CHANNEL);
}

void Audio_InitializeDMA(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    Audio_DMAHandle.Instance = AUDIO_DMA_STREAM;
    Audio_DMAHandle.State    = HAL_DMA_STATE_RESET;

    Audio_DMAHandle.Init.Request             = DMA_REQUEST_DAC1;
    Audio_DMAHandle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    Audio_DMAHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
    Audio_DMAHandle.Init.MemInc              = DMA_MINC_ENABLE;
    Audio_DMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    Audio_DMAHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    Audio_DMAHandle.Init.Mode                = DMA_CIRCULAR;
    Audio_DMAHandle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    Audio_DMAHandle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
    Audio_DMAHandle.Init.MemBurst            = DMA_MBURST_SINGLE;
    Audio_DMAHandle.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    Audio_DMAHandle.Init.Priority            = DMA_PRIORITY_HIGH;

    HAL_DMA_Init(&Audio_DMAHandle);

    __HAL_LINKDMA(&Audio_DACHandle, DMA_Handle1, Audio_DMAHandle);

    HAL_NVIC_SetPriority(AUDIO_DMA_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DMA_IRQn);
}

void Audio_Initialize(void)
{
    Audio_InitializeGPIO();
    Audio_InitializeTimer();
    Audio_InitializeDAC();
    Audio_InitializeDMA();
}

void Audio_SetAudioBuffer(uint16_t* audioBuffer, uint32_t audioBufferSize)
{
    HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_CHANNEL, (uint32_t *)audioBuffer, audioBufferSize, DAC_ALIGN_12B_R);
}

void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Audio_DACHandle.DMA_Handle1);
}
