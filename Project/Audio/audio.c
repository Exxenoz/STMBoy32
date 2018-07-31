#include "audio.h"


TIM_HandleTypeDef      Audio_TimerHandle;

DAC_HandleTypeDef      Audio_DACHandle;
DAC_ChannelConfTypeDef Audio_ChannelConfigL;
DAC_ChannelConfTypeDef Audio_ChannelConfigR;

DMA_HandleTypeDef      Audio_DMAHandleL;
DMA_HandleTypeDef      Audio_DMAHandleR;

volatile bool Audio_IsPlayingOfBufferFinished = false;
uint32_t      Audio_BufferPlayedCounter       = 0;



void Audio_InitializeGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitObject = {0};
    GPIO_InitObject.Mode  = GPIO_MODE_ANALOG;
    GPIO_InitObject.Pin   = AUDIO_L_PIN;
    GPIO_InitObject.Pull  = GPIO_NOPULL;
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 
    HAL_GPIO_Init(AUDIO_L_PORT, &GPIO_InitObject);

    GPIO_InitObject.Pin   = AUDIO_R_PIN;
    HAL_GPIO_Init(AUDIO_R_PORT, &GPIO_InitObject);
}

void Audio_InitializeTimer(void)
{
    Audio_TimerHandle.Instance = AUDIO_TIM;
    Audio_TimerHandle.State    = HAL_TIM_STATE_RESET;

    /**************************************
    // Duration of one GB frame: 0,0167427062988281s
    // Audio samples per frame: 532 * 4 = 2128
    // Duration of one audio sample: 0,0167427062988281s / 2128 = 7,8678131103515507518796992481203e-6
    // Timer frequency: 100MHz
    // Audio frequency: 1 / 7,8678131103515507518796992481203e-6s = 127100,12121212Hz ~ 127,1KHz
    // Timer period: 100MHz / 127100,12121212Hz = 786,781311
    **************************************/

    Audio_TimerHandle.Init.Period        = 786;
    Audio_TimerHandle.Init.Prescaler     = 0;
    Audio_TimerHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    Audio_TimerHandle.Init.ClockDivision = 0;
    Audio_TimerHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&Audio_TimerHandle);

    static TIM_MasterConfigTypeDef masterConfig;
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&Audio_TimerHandle, &masterConfig);

    HAL_TIM_Base_Start(&Audio_TimerHandle);
}

void Audio_InitializeDAC(void)
{
    Audio_DACHandle.Instance = AUDIO_DAC_L;
    Audio_DACHandle.State    = HAL_DAC_STATE_RESET;
    HAL_DAC_Init(&Audio_DACHandle);

    Audio_ChannelConfigL.DAC_Trigger      = AUDIO_DAC_L_TRIGGER;
    Audio_ChannelConfigL.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    Audio_ChannelConfigR.DAC_Trigger      = AUDIO_DAC_R_TRIGGER;
    Audio_ChannelConfigR.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfigL, AUDIO_DAC_L_CHANNEL);
    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfigR, AUDIO_DAC_R_CHANNEL);

    HAL_DAC_Start(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL);
    HAL_DAC_Start(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL);
}

void Audio_InitializeDMA(void)
{
    Audio_DMAHandleL.Instance                 = AUDIO_DMA_L_STREAM;
    Audio_DMAHandleL.State                    = HAL_DMA_STATE_RESET;
    Audio_DMAHandleL.Init.Request             = DMA_REQUEST_DAC1;
    Audio_DMAHandleL.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    Audio_DMAHandleL.Init.PeriphInc           = DMA_PINC_DISABLE;
    Audio_DMAHandleL.Init.MemInc              = DMA_MINC_ENABLE;
    Audio_DMAHandleL.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    Audio_DMAHandleL.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    Audio_DMAHandleL.Init.Mode                = DMA_CIRCULAR;
    Audio_DMAHandleL.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    Audio_DMAHandleL.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
    Audio_DMAHandleL.Init.MemBurst            = DMA_MBURST_SINGLE;
    Audio_DMAHandleL.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    Audio_DMAHandleL.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&Audio_DMAHandleL);

    Audio_DMAHandleR.Instance                 = AUDIO_DMA_R_STREAM;
    Audio_DMAHandleR.State                    = HAL_DMA_STATE_RESET;
    Audio_DMAHandleR.Init.Request             = DMA_REQUEST_DAC2;
    Audio_DMAHandleR.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    Audio_DMAHandleR.Init.PeriphInc           = DMA_PINC_DISABLE;
    Audio_DMAHandleR.Init.MemInc              = DMA_MINC_ENABLE;
    Audio_DMAHandleR.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    Audio_DMAHandleR.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    Audio_DMAHandleR.Init.Mode                = DMA_CIRCULAR;
    Audio_DMAHandleR.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    Audio_DMAHandleR.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
    Audio_DMAHandleR.Init.MemBurst            = DMA_MBURST_SINGLE;
    Audio_DMAHandleR.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    Audio_DMAHandleR.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&Audio_DMAHandleR);

    __HAL_LINKDMA(&Audio_DACHandle, DMA_Handle1, Audio_DMAHandleL);
    __HAL_LINKDMA(&Audio_DACHandle, DMA_Handle2, Audio_DMAHandleR);

    HAL_NVIC_SetPriority(AUDIO_DMA_L_IRQn, 2, 0);
    HAL_NVIC_SetPriority(AUDIO_DMA_R_IRQn, 2, 0);

    HAL_NVIC_EnableIRQ(AUDIO_DMA_L_IRQn);
    HAL_NVIC_EnableIRQ(AUDIO_DMA_R_IRQn);
}

void Audio_Initialize(void)
{
    Audio_InitializeGPIO();
    Audio_InitializeTimer();
    Audio_InitializeDAC();
    Audio_InitializeDMA();
}

void Audio_SetAudioBuffer(uint16_t* audioBufferL, uint16_t* audioBufferR, uint32_t audioBufferSize)
{
    Audio_BufferPlayedCounter = 0;

    HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL, (uint32_t *)audioBufferL, audioBufferSize, DAC_ALIGN_12B_R);
    HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL, (uint32_t *)audioBufferR, audioBufferSize, DAC_ALIGN_12B_R);
}

void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Audio_DACHandle.DMA_Handle1);
}

void DMA1_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Audio_DACHandle.DMA_Handle2);
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    Audio_IsPlayingOfBufferFinished = true;
}

void HAL_DAC_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
}
