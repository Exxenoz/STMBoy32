#include "input.h"

void Input_Initialize() 
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitObject;
    EXTI_InitTypeDef EXTI_InitObject;
    NVIC_InitTypeDef NVIC_InitObject;

    #define INITIALIZE_GPIO_PIN(PORT, PIN)                       \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_IN;                   \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;                  \
    GPIO_InitObject.GPIO_Pin   = PIN;                            \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_DOWN;                 \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;              \
    GPIO_Init(PORT, &GPIO_InitObject);                           \
                                                                 
    #define INITIALIZE_EXTI_LINE(LINE)                           \
    EXTI_InitObject.EXTI_Line    = LINE;                         \
    EXTI_InitObject.EXTI_Mode    = EXTI_Mode_Interrupt;          \
    EXTI_InitObject.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  \
    EXTI_InitObject.EXTI_LineCmd = ENABLE;                       \
    EXTI_Init(&EXTI_InitObject);                                 \
                                                                 
    #define INITIALIZE_NVIC_CHANNEL(CHANNEL)                     \
    NVIC_InitObject.NVIC_IRQChannel                   = CHANNEL; \
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0x0F;    \
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0x0F;    \
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;  \
    NVIC_Init(&NVIC_InitObject);                                 \
    
    INITIALIZE_GPIO_PIN(INPUT_A_PORT,           INPUT_A_PIN);
    INITIALIZE_GPIO_PIN(INPUT_B_PORT,           INPUT_B_PIN);         
    INITIALIZE_GPIO_PIN(INPUT_START_PORT,       INPUT_START_PIN);
    INITIALIZE_GPIO_PIN(INPUT_SELECT_PORT,      INPUT_SELECT_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_TOP_PORT,    INPUT_FADE_TOP_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_RIGHT_PORT,  INPUT_FADE_RIGHT_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_BOTTOM_PORT, INPUT_FADE_BOTTOM_PIN);
    INITIALIZE_GPIO_PIN(INPUT_FADE_LEFT_PORT,   INPUT_FADE_LEFT_PIN);
    
    SYSCFG_EXTILineConfig(INPUT_A_EXTI_PORT,           INPUT_A_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_B_EXTI_PORT,           INPUT_B_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_START_EXTI_PORT,       INPUT_START_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_SELECT_EXTI_PORT,      INPUT_SELECT_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_FADE_TOP_EXTI_PORT,    INPUT_FADE_TOP_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_FADE_RIGHT_EXTI_PORT,  INPUT_FADE_RIGHT_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_FADE_BOTTOM_EXTI_PORT, INPUT_FADE_BOTTOM_EXTI_PIN);
    SYSCFG_EXTILineConfig(INPUT_FADE_LEFT_EXTI_PORT,   INPUT_FADE_LEFT_EXTI_PIN);
    
    INITIALIZE_EXTI_LINE(INPUT_A_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_B_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_START_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_SELECT_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_FADE_TOP_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_FADE_RIGHT_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_FADE_BOTTOM_EXTI_LINE);
    INITIALIZE_EXTI_LINE(INPUT_FADE_LEFT_EXTI_LINE);
    
    INITIALIZE_NVIC_CHANNEL(INPUT_A_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_B_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_START_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_SELECT_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_FADE_TOP_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_FADE_RIGHT_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_FADE_BOTTOM_NVIC_CHANNEL);
    INITIALIZE_NVIC_CHANNEL(INPUT_FADE_LEFT_NVIC_CHANNEL);
    
    //-----------DEBUG LED----------
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitObject);
    //------------------------------
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
    }    
}

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) 
    {
        /*if(GPIOB->ODR & GPIO_Pin_0)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_0);
        }
        else
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        }*/
        GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
    }  
    

    EXTI_ClearITPendingBit(EXTI_Line4);
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI9_5_IRQHandler(void) 
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    
    if (EXTI_GetITStatus(EXTI_Line6) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    
    if (EXTI_GetITStatus(EXTI_Line7) != RESET) 
    { 
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    
    if (EXTI_GetITStatus(EXTI_Line8) != RESET) 
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
}
