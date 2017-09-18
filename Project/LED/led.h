#ifndef LED_H
#define LED_H

#include "common.h"
#include "led_config.h"

#define LED_RED_SET     LED_RED_PORT->BSRRL |= LED_RED_PIN
#define LED_RED_RESET   LED_RED_PORT->BSRRH |= LED_RED_PIN

#define LED_GREEN_SET   LED_GREEN_PORT->BSRRL |= LED_GREEN_PIN
#define LED_GREEN_RESET LED_GREEN_PORT->BSRRH |= LED_GREEN_PIN

#define LED_BLUE_SET    LED_BLUE_PORT->BSRRL |= LED_BLUE_PIN
#define LED_BLUE_RESET  LED_BLUE_PORT->BSRRH |= LED_BLUE_PIN

void LED_Initialize(void);

void LED_EnableRed(bool enable);
void LED_EnableGreen(bool enable);
void LED_EnableBlue(bool enable);

#endif //LED_H
