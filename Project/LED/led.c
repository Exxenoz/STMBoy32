#include "led.h"

void LED_Initialize(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)       \
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_OD;    \
    GPIO_InitObject.Pin   = PIN;                     \
    GPIO_InitObject.Pull  = GPIO_NOPULL;              \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LED_RED_PORT,   LED_RED_PIN);
    INITIALIZE_OUTPUT_PIN(LED_GREEN_PORT, LED_GREEN_PIN);
    INITIALIZE_OUTPUT_PIN(LED_BLUE_PORT,  LED_BLUE_PIN);

    LED_EnableRed(false);
    LED_EnableGreen(false);
    LED_EnableBlue(true);
}

void LED_EnableRed(bool enable)
{
    if (enable)
    {
        LED_RED_RESET;
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
        LED_GREEN_RESET;
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
        LED_BLUE_RESET;
    }
    else
    {
        LED_BLUE_SET;
    }
}
