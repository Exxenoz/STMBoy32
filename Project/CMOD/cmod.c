#include "cmod.h"

CMOD_ACTION  cmod_action;
CMOD_STATUS  cmod_status      = WAITING;
uint16_t     cmod_address     = 0x0000;
uint8_t      cmod_data_in     = 0x00;
uint8_t      *cmod_data_out   = NULL;
int          cmod_bytesToRead = 1;

CMOD_STATUS CMOD_GetStatus(void) 
{ 
    return cmod_status; 
}

void CMOD_Read_Byte(uint16_t address, uint8_t *data)
{
    cmod_status      = PROCESSING;
    cmod_address     = address;
    cmod_data_out    = data;
    cmod_action      = CMOD_READ;
    cmod_bytesToRead = 1;
    
    CMOD_Enable_Interrupt();
}

void CMOD_Read_Bytes(uint16_t starting_address, int bytes, uint8_t *data)
{
    cmod_status      = PROCESSING;
    cmod_address     = starting_address;
    cmod_data_out    = data;
    cmod_action      = CMOD_READ;
    cmod_bytesToRead = bytes;
    
    CMOD_Enable_Interrupt();
}

void CMOD_Write_Byte(uint16_t address, uint8_t data)
{
    cmod_status  = PROCESSING;
    cmod_address = address;
    cmod_data_in = data;
    cmod_action  = CMOD_READ;
    
    CMOD_Enable_Interrupt();
}

void CMOD_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_RESET_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CS_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_RD_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_WR_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DETECT_BUS, ENABLE);
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
 
    GPIO_SetBits(CMOD_WR_PORT, CMOD_WR_PIN);
    CMOD_Initialize_CLK();
    CMOD_Initialize_Insertion_Interrupt();
}

void CMOD_Initialize_CLK(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_CLK_BUS, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;
    TIM_TimeBaseInitTypeDef TIM_BaseObject;
    TIM_OCInitTypeDef       TIM_OCInitObject;
    NVIC_InitTypeDef        NVIC_InitObject;
    
    
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;    
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;    
    GPIO_InitObject.GPIO_Pin   = CMOD_CLK_PIN;              
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL; 
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(CMOD_CLK_PORT, &GPIO_InitObject);              
    GPIO_PinAFConfig(CMOD_CLK_PORT, CMOD_CLK_PINSOURCE, CMOD_CLK_AF);
      
    TIM_BaseObject.TIM_Prescaler         = 0;
    TIM_BaseObject.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period            = 85;                  
    TIM_BaseObject.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_BaseObject);
    
    TIM_OCInitObject.TIM_OCMode       = TIM_OCMode_PWM2;
	TIM_OCInitObject.TIM_OutputState  = TIM_OutputState_Enable;
	TIM_OCInitObject.TIM_OCPolarity   = TIM_OCPolarity_Low;
	TIM_OCInitObject.TIM_Pulse        = 42;                      
	TIM_OC4Init(TIM4, &TIM_OCInitObject);  
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
    // TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC4 | TIM_IT_Trigger | TIM_IT_CC3, DISABLE);
    // TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
    TIM_Cmd(TIM4, ENABLE);

    NVIC_InitObject.NVIC_IRQChannel                   = TIM4_IRQn;
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitObject);                                     
}

void CMOD_Initialize_Insertion_Interrupt()
{
    RCC_AHB1PeriphClockCmd(CMOD_DETECT_BUS, ENABLE);
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

void CMOD_Enable_Interrupt(void)
{
    NVIC_EnableIRQ(TIM4_IRQn);
}

void CMOD_Disable_Interrupt(void)
{
    NVIC_DisableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus (TIM4, TIM_IT_Update) != RESET) 
  {
    // WIP
    GPIO_ToggleBits(CMOD_WR_PORT, CMOD_WR_PIN);
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
  }
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
