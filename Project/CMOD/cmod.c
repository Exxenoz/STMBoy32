#include "cmod.h"

void CMOD_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_RESET_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CS_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_RD_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_WR_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DETECT_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CLK_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_ADDR_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DATA_BUS, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)        \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;     \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;     \
    GPIO_InitObject.GPIO_Pin   = PIN;               \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;  \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; \
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(CMOD_RESET_PORT, CMOD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_CS_PORT, CMOD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_RD_PORT, CMOD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_WR_PORT, CMOD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_ADDR_PORT, CMOD_ADDR_PINS);
    INITIALIZE_OUTPUT_PIN(CMOD_DATA_PORT, CMOD_DATA_PINS);
    
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;    
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;    
    GPIO_InitObject.GPIO_Pin   = CMOD_CLK_PIN;              
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL; 
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(CMOD_CLK_PORT, &GPIO_InitObject);             
    
    GPIO_PinAFConfig(CMOD_CLK_PORT, CMOD_CLK_PINSOURCE, CMOD_CLK_AF);
    
    CMOD_Initialize_Timer();
    CMOD_Initialize_PWM();
    CMOD_Initialize_CLK_Interrupt();
    CMOD_Initialize_Insertion_Interrupt();
}

void CMOD_Initialize_Timer(void) 
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_BaseObject;

    TIM_BaseObject.TIM_Prescaler         = 0;
    TIM_BaseObject.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period            = 89;                  // TIM_Period = ((timer_tick_frequency / PWM_frequency)) - 1
    TIM_BaseObject.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_BaseObject);

    TIM_Cmd(TIM4, ENABLE);
}

void CMOD_Initialize_PWM(void) 
{
	TIM_OCInitTypeDef TIM_OCObject;

	TIM_OCObject.TIM_OCMode      = TIM_OCMode_PWM2;
	TIM_OCObject.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCObject.TIM_OCPolarity  = TIM_OCPolarity_Low;
	TIM_OCObject.TIM_Pulse       = 44;                          // TIM_Pulse = (((TIM_Period + 1) * DutyCycle) / 100) - 1
	TIM_OC4Init(TIM4, &TIM_OCObject);
    
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
}

void CMOD_Initialize_CLK_Interrupt(void)
{
    // TODO
}

void CMOD_Initialize_Insertion_Interrupt()
{
    RCC_AHB1PeriphClockCmd(CMOD_DETECT_BUS, ENABLE);

    // SYSCFG APB clock must be enabled to get write access to SYSCFG_EXTICRx
    // registers using RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    EXTI_InitTypeDef EXTI_InitObject;
    NVIC_InitTypeDef NVIC_InitObject;
    
    SYSCFG_EXTILineConfig(CMOD_DETECT_EXTI_PORT, CMOD_DETECT_EXTI_PIN);

    EXTI_InitObject.EXTI_Line    = CMOD_DETECT_EXTI_LINE;                         
    EXTI_InitObject.EXTI_Mode    = EXTI_Mode_Interrupt;          
    EXTI_InitObject.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
    EXTI_InitObject.EXTI_LineCmd = ENABLE;                       
    EXTI_Init(&EXTI_InitObject);                                 

    NVIC_InitObject.NVIC_IRQChannel                   = CMOD_DETECT_NVIC_CHANNEL; 
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0x0F;    
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0x0F;    
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;  
    NVIC_Init(&NVIC_InitObject);                                 
    NVIC_EnableIRQ(CMOD_DETECT_NVIC_CHANNEL);                                     
}

void EXTI15_10_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line11) != RESET) 
    {
        LED_EnableRed(false);
        LED_EnableGreen(true);
        LED_EnableBlue(true);
    }
}
