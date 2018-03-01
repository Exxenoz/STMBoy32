#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#define AUDIO_PORT         GPIOA
#define AUDIO_PIN          GPIO_PIN_4
#define AUDIO_BUS          RCC_AHB1Periph_GPIOA

#define AUDIO_TIM          TIM8
#define AUDIO_TIM_BUS      RCC_APB2Periph_TIM8

#define AUDIO_DAC_BUS      RCC_APB1Periph_DAC
#define AUDIO_DAC_CHANNEL  DAC_Channel_1
#define AUDIO_DAC_TRIGGER  DAC_Trigger_T8_TRGO

#define AUDIO_DMA          DMA1
#define AUDIO_DMA_BUS      RCC_AHB1Periph_DMA1
#define AUDIO_DMA_STREAM   DMA1_Stream5
#define AUDIO_DMA_CHANNEL  DMA_Channel_7

#endif
