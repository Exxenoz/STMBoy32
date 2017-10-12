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
    INITIALIZE_OUTPUT_PIN(CMOD_CLK_PORT, CMOD_CLK_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_ADDR_PORT, CMOD_ADDR_PINS);
    INITIALIZE_OUTPUT_PIN(CMOD_DATA_PORT, CMOD_DATA_PINS);
    
    CMOD_Initialize_Interrupt();
}

void CMOD_Initialize_Interrupt()
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
        GPIO_ToggleBits(GPIOB, GPIO_Pin_14);
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
}
