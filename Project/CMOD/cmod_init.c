#include "cmod.h"
#include "cmod_init.h"
#include "cmod_config.h"


TIM_OC_InitTypeDef CMOD_TIM_OCInitObject;
TIM_HandleTypeDef  CMOD_TimerHandle =
{ 
    .Instance = CMOD_TIM,
    .Channel  = HAL_TIM_ACTIVE_CHANNEL_1
};

bool CMOD_Initialized = false;



void CMOD_Initialize_GPIOS(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)       \
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;    \
    GPIO_InitObject.Pin   = PIN;                     \
    GPIO_InitObject.Pull  = GPIO_PULLDOWN;            \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(CMOD_LLC_PORT,   CMOD_LLC_DD_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_RD_PORT,    CMOD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_ADDR_PORT,  CMOD_ADDR_PINS);
    INITIALIZE_OUTPUT_PIN(CMOD_DATA_PORT,  CMOD_DATA_PINS);

    GPIO_InitObject.Pull = GPIO_PULLUP;
    GPIO_InitObject.Pin  = CMOD_LLC_OE_PIN;
    HAL_GPIO_Init(CMOD_LLC_PORT, &GPIO_InitObject);
    
    GPIO_InitObject.Pin  = CMOD_RESET_PIN;
    HAL_GPIO_Init(CMOD_RESET_PORT, &GPIO_InitObject);
    
    GPIO_InitObject.Pin  = CMOD_CS_PIN;
    HAL_GPIO_Init(CMOD_CS_PORT, &GPIO_InitObject);
    
    GPIO_InitObject.Pin  = CMOD_WR_PIN;
    HAL_GPIO_Init(CMOD_WR_PORT, &GPIO_InitObject);
}

void CMOD_Initialize_CLK(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = CMOD_CLK_PIN;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(CMOD_CLK_PORT, &GPIO_InitObject);

    CMOD_TimerHandle.Init.Prescaler         = 1;                      // Tim4 runs with ~190Mhz -> scale to ~95Mhz.
    CMOD_TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    CMOD_TimerHandle.Init.Period            = 90;                     // Count 'til 91(-1) -> 1,05Mhz PWM.
    CMOD_TimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    CMOD_TimerHandle.Init.RepetitionCounter = 0;

    CMOD_TIM_OCInitObject.Pulse        = 47;
    CMOD_TIM_OCInitObject.OCMode       = TIM_OCMODE_PWM1;
    CMOD_TIM_OCInitObject.OCFastMode   = TIM_OCFAST_DISABLE;
    CMOD_TIM_OCInitObject.OCPolarity   = TIM_OCPOLARITY_LOW;
    CMOD_TIM_OCInitObject.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    CMOD_TIM_OCInitObject.OCIdleState  = TIM_OCIDLESTATE_RESET;
    CMOD_TIM_OCInitObject.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_Init(&CMOD_TimerHandle);

    if (HAL_TIM_PWM_ConfigChannel(&CMOD_TimerHandle, &CMOD_TIM_OCInitObject, CMOD_TIM_CHANNEL) != HAL_OK)
    {
        //Error_Handler();
    }

    HAL_NVIC_SetPriority(CMOD_TIM_NVIC_CHANNEL, INTERRUPT_PRIORITY_2, INTERRUPT_PRIORITY_2);
}

void CMOD_Initialize(void)
{
    CMOD_Initialize_GPIOS();
    CMOD_Initialize_CLK();

    CMOD_SET_RESET;
    CMOD_SET_CS;
    CMOD_SET_WR;
    CMOD_RST_RD;

    CMOD_Initialized = true;
}
