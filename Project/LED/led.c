#include "led.h"

void LED_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)        \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;     \
    GPIO_InitObject.GPIO_OType = GPIO_OType_OD;     \
    GPIO_InitObject.GPIO_Pin   = PIN;               \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;  \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; \
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LED_RED_PORT, LED_RED_PIN);
    INITIALIZE_OUTPUT_PIN(LED_GREEN_PORT, LED_GREEN_PIN);
    INITIALIZE_OUTPUT_PIN(LED_BLUE_PORT, LED_BLUE_PIN);

    LED_EnableRed(false);
    LED_EnableGreen(false);
    LED_EnableBlue(true);
}

void LED_EnableRed(bool enable)
{
    if (enable)
    {
        LED_RED_RESET;
        LED_GREEN_SET;
        LED_BLUE_SET;
    }
    else
    {
        LED_RED_SET;
    }
}

void LED_EnableGreen(bool enable)
{
    if (enable)
    {
        LED_RED_SET;
        LED_GREEN_RESET;
        LED_BLUE_SET;
    }
    else
    {
        LED_GREEN_SET;
    }
}

void LED_EnableBlue(bool enable)
{
    if (enable)
    {
        LED_RED_SET;
        LED_GREEN_SET;
        LED_BLUE_RESET;
    }
    else
    {
        LED_BLUE_SET;
    }
}
