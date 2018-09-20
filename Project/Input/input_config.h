#ifndef INPUT_CONFIG_H
#define INPUT_CONFIG_H


/******************************************************************************/
/*                 Input Pin, Port, Bus and Timer definitions                 */
/******************************************************************************/

#define INPUT_LOCK_TIM                      TIM7
#define INPUT_LOCK_TIM_FREQ                 200000000   // Do not change!
#define INPUT_LOCK_TIM_PRESCALER            49999

#define INPUT_POLLING_TIM                   TIM2
#define INPUT_POLLING_TIM_NVIC_CHANNEL      TIM2_IRQn
#define INPUT_POLLING_TIM_FREQ              200000000   // Do not change!
#define INPUT_POLLING_TIM_PRESCALER         49999

#define INPUT_FRAME_PORT                    GPIOA
#define INPUT_FRAME_PIN                     GPIO_PIN_0

#define INPUT_A_PORT                        GPIOA
#define INPUT_A_PIN                         GPIO_PIN_1

#define INPUT_B_PORT                        GPIOA
#define INPUT_B_PIN                         GPIO_PIN_2

#define INPUT_SELECT_PORT                   GPIOD
#define INPUT_SELECT_PIN                    GPIO_PIN_3

#define INPUT_START_PORT                    GPIOD
#define INPUT_START_PIN                     GPIO_PIN_4

#define INPUT_FADE_RIGHT_PORT               GPIOD
#define INPUT_FADE_RIGHT_PIN                GPIO_PIN_5

#define INPUT_FADE_LEFT_PORT                GPIOD
#define INPUT_FADE_LEFT_PIN                 GPIO_PIN_6

#define INPUT_FADE_TOP_PORT                 GPIOD
#define INPUT_FADE_TOP_PIN                  GPIO_PIN_7

#define INPUT_FADE_BOTTOM_PORT              GPIOF
#define INPUT_FADE_BOTTOM_PIN               GPIO_PIN_8


#endif //INPUT_CONFIG_H
