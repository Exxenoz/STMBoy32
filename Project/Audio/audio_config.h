#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#define AUDIO_SD_PORT          GPIOA
#define AUDIO_SD_PIN           GPIO_PIN_12

#define AUDIO_L_PORT           GPIOA
#define AUDIO_L_PIN            GPIO_PIN_4

#define AUDIO_R_PORT           GPIOA
#define AUDIO_R_PIN            GPIO_PIN_5

#define AUDIO_TIM              TIM6
#define AUDIO_TIM_BUS          RCC_APB2Periph_TIM6

#define AUDIO_DAC_L            DAC1
#define AUDIO_DAC_L_BUS        RCC_APB1Periph_DAC
#define AUDIO_DAC_L_CHANNEL    DAC_CHANNEL_1
#define AUDIO_DAC_L_TRIGGER    DAC_TRIGGER_T6_TRGO

#define AUDIO_DMA_L            DMA1
#define AUDIO_DMA_L_BUS        RCC_AHB1Periph_DMA1
#define AUDIO_DMA_L_STREAM     DMA1_Stream5
#define AUDIO_DMA_L_IRQn       DMA1_Stream5_IRQn
#define AUDIO_DMA_L_IRQHandler DMA1_Stream5_IRQHandler

#define AUDIO_DAC_R            DAC1
#define AUDIO_DAC_R_BUS        RCC_APB1Periph_DAC
#define AUDIO_DAC_R_CHANNEL    DAC_CHANNEL_2
#define AUDIO_DAC_R_TRIGGER    DAC_TRIGGER_T6_TRGO

#define AUDIO_DMA_R            DMA1
#define AUDIO_DMA_R_BUS        RCC_AHB1Periph_DMA1
#define AUDIO_DMA_R_STREAM     DMA1_Stream6
#define AUDIO_DMA_R_IRQn       DMA1_Stream6_IRQn
#define AUDIO_DMA_R_IRQHandler DMA1_Stream6_IRQHandler

#endif
