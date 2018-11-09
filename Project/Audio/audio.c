#include "audio.h"

Audio_OutputMode_t Audio_OutputMode;
Audio_SampleAlignment_t Audio_SampleAlignment;
uint8_t* Audio_BufferLeft;
uint8_t* Audio_BufferRight;
uint32_t Audio_BufferSize;
uint32_t Audio_BufferSizeHalf;
bool Audio_Loop;
Audio_Playback_t Audio_Playback;
volatile bool Audio_IsPlayingOfBufferLeftFinished;
volatile bool Audio_IsPlayingOfBufferRightFinished;

TIM_HandleTypeDef       Audio_TimerHandle;

DMA_HandleTypeDef       Audio_DMAHandleL;
DMA_HandleTypeDef       Audio_DMAHandleR;

DAC_HandleTypeDef       Audio_DACHandle;
DAC_ChannelConfTypeDef  Audio_ChannelConfigL;
DAC_ChannelConfTypeDef  Audio_ChannelConfigR;



void Audio_InitializeBuffer(void)
{
    Audio_OutputMode = AUDIO_OUTPUTMODE_NONE;
    Audio_SampleAlignment = AUDIO_SAMPLE_ALIGNMENT_8B_R;
    Audio_BufferLeft = NULL;
    Audio_BufferRight = NULL;
    Audio_BufferSize = 0;
    Audio_BufferSizeHalf = 0;
    Audio_Loop = false;
    Audio_Playback = AUDIO_PLAYBACK_NONE;
    Audio_IsPlayingOfBufferLeftFinished = true;
    Audio_IsPlayingOfBufferRightFinished = true;
}

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

    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitObject.Pin   = AUDIO_NSD_PIN;
    HAL_GPIO_Init(AUDIO_NSD_PORT, &GPIO_InitObject);
    GPIO_InitObject.Pin   = AUDIO_NMUTE_PIN;
    HAL_GPIO_Init(AUDIO_NMUTE_PORT, &GPIO_InitObject);
    GPIO_InitObject.Pin   = AUDIO_MODE_PIN;
    HAL_GPIO_Init(AUDIO_MODE_PORT, &GPIO_InitObject);
}

void Audio_InitializeDAC(void)
{
    Audio_DACHandle.Instance = AUDIO_DAC_L;
    Audio_DACHandle.State    = HAL_DAC_STATE_RESET;
    HAL_DAC_Init(&Audio_DACHandle);

    Audio_ChannelConfigL.DAC_Trigger      = AUDIO_DAC_L_TRIGGER;
    Audio_ChannelConfigL.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;

    Audio_ChannelConfigR.DAC_Trigger      = AUDIO_DAC_R_TRIGGER;
    Audio_ChannelConfigR.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;

    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfigL, AUDIO_DAC_L_CHANNEL);
    HAL_DAC_ConfigChannel(&Audio_DACHandle, &Audio_ChannelConfigR, AUDIO_DAC_R_CHANNEL);

    HAL_DAC_Start(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL);
    HAL_DAC_Start(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL);
}

void Audio_InitializeTimer(Audio_OutputMode_t outputMode)
{
    Audio_TimerHandle.Instance = AUDIO_TIM;
    Audio_TimerHandle.State    = HAL_TIM_STATE_RESET;

    if (outputMode == AUDIO_OUTPUTMODE_GBC)
    {
        /**************************************
        // Duration of one GB frame:     0,0167427062988281s
        // Audio samples per frame:      1064
        // Duration of one audio sample: 0,0167427062988281s / 1064 = 1,5735626220703101503759398496241e-5
        // Timer frequency:              200MHz
        // Audio frequency:              1 / 1,5735626220703101503759398496241e-5 = 63550,06Hz ~ 63,550kHz
        // Timer period:                 200MHz / 63550,06Hz = 3147,1252
        **************************************/

        Audio_TimerHandle.Init.Period            = 3147;
        Audio_TimerHandle.Init.Prescaler         = 0;
        Audio_TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
        Audio_TimerHandle.Init.ClockDivision     = 0;
        Audio_TimerHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    }

    HAL_TIM_Base_Init(&Audio_TimerHandle);

    static TIM_MasterConfigTypeDef masterConfig;
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&Audio_TimerHandle, &masterConfig);

    HAL_TIM_Base_Start(&Audio_TimerHandle);
}

void Audio_InitializeDMA(Audio_OutputMode_t outputMode)
{
    if (outputMode == AUDIO_OUTPUTMODE_GBC)
    {
        Audio_DMAHandleL.Instance                 = AUDIO_DMA_L_STREAM;
        Audio_DMAHandleL.State                    = HAL_DMA_STATE_RESET;
        Audio_DMAHandleL.Init.Request             = DMA_REQUEST_DAC1;
        Audio_DMAHandleL.Init.Direction           = DMA_MEMORY_TO_PERIPH;
        Audio_DMAHandleL.Init.PeriphInc           = DMA_PINC_DISABLE;
        Audio_DMAHandleL.Init.MemInc              = DMA_MINC_ENABLE;
        Audio_DMAHandleL.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        Audio_DMAHandleL.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        Audio_DMAHandleL.Init.Mode                = DMA_NORMAL;
        Audio_DMAHandleL.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        Audio_DMAHandleL.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
        Audio_DMAHandleL.Init.MemBurst            = DMA_MBURST_INC8;
        Audio_DMAHandleL.Init.PeriphBurst         = DMA_PBURST_INC8;
        Audio_DMAHandleL.Init.Priority            = DMA_PRIORITY_HIGH;

        Audio_DMAHandleR.Instance                 = AUDIO_DMA_R_STREAM;
        Audio_DMAHandleR.State                    = HAL_DMA_STATE_RESET;
        Audio_DMAHandleR.Init.Request             = DMA_REQUEST_DAC2;
        Audio_DMAHandleR.Init.Direction           = DMA_MEMORY_TO_PERIPH;
        Audio_DMAHandleR.Init.PeriphInc           = DMA_PINC_DISABLE;
        Audio_DMAHandleR.Init.MemInc              = DMA_MINC_ENABLE;
        Audio_DMAHandleR.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        Audio_DMAHandleR.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        Audio_DMAHandleR.Init.Mode                = DMA_NORMAL;
        Audio_DMAHandleR.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        Audio_DMAHandleR.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
        Audio_DMAHandleR.Init.MemBurst            = DMA_MBURST_INC8;
        Audio_DMAHandleR.Init.PeriphBurst         = DMA_PBURST_INC8;
        Audio_DMAHandleR.Init.Priority            = DMA_PRIORITY_HIGH;
    }

    HAL_DMA_Init(&Audio_DMAHandleL);
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
    Audio_InitializeBuffer();
    Audio_InitializeGPIO();
    Audio_InitializeDAC();

    AUDIO_SET_MODE_D;
    
    Audio_EnablePower(true);
    Audio_Mute(false);
}

void Audio_EnablePower(bool enable)
{
    if (enable)
    {
        AUDIO_SET_NSD;
    }
    else
    {
        AUDIO_RST_NSD;
    }
}

void Audio_Mute(bool mute)
{
    if (mute)
    {
        AUDIO_RST_NMUTE;
    }
    else
    {
        AUDIO_SET_NMUTE;
    }
}

void Audio_SetOutputBuffer(Audio_OutputMode_t outputMode, Audio_SampleAlignment_t sampleAlignment,
    uint8_t* bufferLeft, uint8_t* bufferRight, uint32_t bufferSize)
{
    Audio_OutputMode = outputMode;
    Audio_SampleAlignment = sampleAlignment;
    Audio_BufferLeft = bufferLeft;
    Audio_BufferRight = bufferRight;
    Audio_BufferSize = bufferSize;
    Audio_BufferSizeHalf = bufferSize / 2;

    Audio_InitializeTimer(outputMode);
    Audio_InitializeDMA(outputMode);
}

void Audio_StartOutput(Audio_Playback_t playback, bool loop)
{
    if (Audio_OutputMode == AUDIO_OUTPUTMODE_NONE)
    {
        return;
    }

    if (Audio_Playback != AUDIO_PLAYBACK_NONE)
    {
        Audio_StopOutput();
    }

    Audio_IsPlayingOfBufferLeftFinished  = false;
    Audio_IsPlayingOfBufferRightFinished = false;

    switch (playback)
    {
        case AUDIO_PLAYBACK_NONE:
            break;
        case AUDIO_PLAYBACK_FULL:
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL, (uint32_t*)Audio_BufferLeft,  Audio_BufferSize, Audio_SampleAlignment);
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL, (uint32_t*)Audio_BufferRight, Audio_BufferSize, Audio_SampleAlignment);
            break;
        case AUDIO_PLAYBACK_HALF1:
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL, (uint32_t*)Audio_BufferLeft,  Audio_BufferSizeHalf, Audio_SampleAlignment);
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL, (uint32_t*)Audio_BufferRight, Audio_BufferSizeHalf, Audio_SampleAlignment);
            break;
        case AUDIO_PLAYBACK_HALF2:
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL, (uint32_t*)(&Audio_BufferLeft[Audio_BufferSizeHalf]),  Audio_BufferSizeHalf, Audio_SampleAlignment);
            HAL_DAC_Start_DMA(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL, (uint32_t*)(&Audio_BufferRight[Audio_BufferSizeHalf]), Audio_BufferSizeHalf, Audio_SampleAlignment);
            break;
        default:
            break;
    }

    Audio_Loop = loop;
    Audio_Playback = playback;
}

void Audio_StopOutput(void)
{
    HAL_DAC_Stop_DMA(&Audio_DACHandle, AUDIO_DAC_L_CHANNEL);
    HAL_DAC_Stop_DMA(&Audio_DACHandle, AUDIO_DAC_R_CHANNEL);

    Audio_Loop = false;
    Audio_Playback = AUDIO_PLAYBACK_NONE;

    Audio_IsPlayingOfBufferLeftFinished = true;
    Audio_IsPlayingOfBufferRightFinished = true;
}

void Audio_Join(void)
{
    while (Audio_Playback != AUDIO_PLAYBACK_NONE &&
        (Audio_IsPlayingOfBufferLeftFinished == false ||
        Audio_IsPlayingOfBufferRightFinished == false));
}

void Audio_Update(void)
{
    Audio_Join();

    if (Audio_Playback == AUDIO_PLAYBACK_NONE)
    {
        return;
    }

    if (!Audio_Loop)
    {
        Audio_StopOutput();
        return;
    }

    switch (Audio_Playback)
    {
        case AUDIO_PLAYBACK_NONE:
            break;
        case AUDIO_PLAYBACK_FULL:
            Audio_StartOutput(AUDIO_PLAYBACK_FULL, true);
            break;
        case AUDIO_PLAYBACK_HALF1:
            Audio_StartOutput(AUDIO_PLAYBACK_HALF2, true);
            break;
        case AUDIO_PLAYBACK_HALF2:
            Audio_StartOutput(AUDIO_PLAYBACK_HALF1, true);
            break;
        default:
            break;
    }
}

void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Audio_DACHandle.DMA_Handle1);
}

void DMA1_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Audio_DACHandle.DMA_Handle2);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    Audio_IsPlayingOfBufferLeftFinished = true;
}

void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
    Audio_IsPlayingOfBufferRightFinished = true;
}
