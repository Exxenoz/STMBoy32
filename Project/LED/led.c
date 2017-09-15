#include "led.h"

void LED_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitObject;
    TIM_OCInitTypeDef TIM_OCInitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)        \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;      \
    GPIO_InitObject.GPIO_Pin   = PIN;               \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; \
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LED_RED_PORT, LED_RED_PIN);
    INITIALIZE_OUTPUT_PIN(LED_GREEN_PORT, LED_GREEN_PIN);
    INITIALIZE_OUTPUT_PIN(LED_BLUE_PORT, LED_BLUE_PIN);

    GPIO_PinAFConfig(LED_RED_PORT,   LED_RED_PIN_SOURCE,   GPIO_AF_TIM1);
    GPIO_PinAFConfig(LED_GREEN_PORT, LED_GREEN_PIN_SOURCE, GPIO_AF_TIM1);
    GPIO_PinAFConfig(LED_BLUE_PORT,  LED_BLUE_PIN_SOURCE,  GPIO_AF_TIM1);

    uint32_t frequency = 1000000; // 1kHz
    uint32_t period    = frequency / 1000;

    TIM_TimeBaseInitObject.TIM_Prescaler = (uint16_t)((SystemCoreClock / frequency) - 1);
    TIM_TimeBaseInitObject.TIM_Period = period;
    TIM_TimeBaseInitObject.TIM_ClockDivision = 0;
    TIM_TimeBaseInitObject.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitObject.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitObject);

    TIM_OCInitObject.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitObject.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCInitObject.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitObject.TIM_Pulse = period;

    TIM_OC1Init(TIM1, &TIM_OCInitObject);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM1, &TIM_OCInitObject);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM1, &TIM_OCInitObject);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC4Init(TIM1, &TIM_OCInitObject);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM1, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}
