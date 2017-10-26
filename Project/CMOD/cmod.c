#include "cmod.h"

CMOD_ACTION  CMOD_Action       = CMOD_NOACTION;
CMOD_STATUS  CMOD_Status       = CMOD_WAITING;
uint16_t     CMOD_Address      = 0x0000;
uint8_t      *CMOD_DataOut     = NULL;
uint8_t      *CMOD_DataIn      = NULL;
int          CMOD_BytesToRead  = 0;
int          CMOD_BytesRead    = 0;
int          CMOD_BytesToWrite = 0;
int          CMOD_BytesWritten = 0;

CMOD_STATUS CMOD_GetStatus(void) 
{ 
    return CMOD_Status; 
}

// Check for a Cartridge by trying to read the first byte of the Nintendo Logo
bool CMOD_Detect(void)
{
    uint8_t data = 0x00;
    
    CMOD_ReadByte(0x0104, &data);
    while (CMOD_Status == CMOD_PROCESSING);
    
    if (data == 0xCE) return true;
    else              return false;
}

void CMOD_SaveCartridge(void)
{
    
}

// Manually switch the currently active Memory Bank of the Cartridge (to be able to save it)
void CMOD_SwitchMB(uint8_t bank)
{
    GBC_MMU_MemoryBankController_t GBC_MMU_MemoryBankController = GBC_MMU_GetMemoryBankController();
    
    if (GBC_MMU_MemoryBankController == GBC_MMU_MBC_NONE)
    {
        
    }
    else if (GBC_MMU_MemoryBankController == GBC_MMU_MBC1)
    {
        
    }
    else if (GBC_MMU_MemoryBankController == GBC_MMU_MBC2 || GBC_MMU_MemoryBankController == GBC_MMU_MBC3)
    {
        
    }
    else if (GBC_MMU_MemoryBankController == GBC_MMU_MBC5)
    {
        
    }
    else
    {
        
    }
}

void CMOD_EnableInterrupt(void)
{
    TIM_ITConfig(CMOD_TIM, TIM_IT_Update, ENABLE);
}

void CMOD_DisableInterrupt(void)
{
    TIM_ITConfig(CMOD_TIM, TIM_IT_Update, DISABLE);
}

void CMOD_ReadByte(uint16_t address, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);
    
    CMOD_Action      = CMOD_READ;
    CMOD_Address     = address;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = 1;
    CMOD_BytesRead   = 0;
    
    CMOD_Status      = CMOD_PROCESSING;
    CMOD_EnableInterrupt();
}

void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);
    
    CMOD_Action      = CMOD_READ;
    CMOD_Address     = startingAddress;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = bytes;
    CMOD_BytesRead   = 0;
    
    CMOD_Status      = CMOD_PROCESSING;
    CMOD_EnableInterrupt();
}

void CMOD_WriteByte(uint16_t address, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);
    
    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = address;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = 1;
    CMOD_BytesWritten = 0;
    
    CMOD_Status  = CMOD_PROCESSING;
    CMOD_EnableInterrupt();
}

void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    while (CMOD_Status == CMOD_PROCESSING);
    
    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = startingAddress;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = bytes;
    CMOD_BytesWritten = 0;
    
    CMOD_Status       = CMOD_PROCESSING;
    CMOD_EnableInterrupt();
}

void CMOD_Initialize_CLK(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_CLK_BUS, ENABLE);
    RCC_APB1PeriphClockCmd(CMOD_TIM_BUS, ENABLE);

    GPIO_InitTypeDef        GPIO_InitObject;
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
    TIM_TimeBaseInit(CMOD_TIM, &TIM_BaseObject);
    
    TIM_OCInitObject.TIM_OCMode          = TIM_OCMode_PWM1;
	TIM_OCInitObject.TIM_OutputState     = TIM_OutputState_Enable;
	TIM_OCInitObject.TIM_OCPolarity      = TIM_OCPolarity_Low;
	TIM_OCInitObject.TIM_Pulse           = 42;                      
	TIM_OC4Init(CMOD_TIM, &TIM_OCInitObject);  
	TIM_OC4PreloadConfig(CMOD_TIM, TIM_OCPreload_Enable);
    
    TIM_ITConfig(CMOD_TIM, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4 | TIM_IT_Trigger | TIM_IT_Update, DISABLE);
    TIM_ClearITPendingBit(CMOD_TIM, TIM_IT_CC4);
    TIM_Cmd(CMOD_TIM, ENABLE);

    NVIC_InitObject.NVIC_IRQChannel                   = CMOD_TIM_NVIC_CHANNEL;
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitObject);                          
    NVIC_EnableIRQ(CMOD_TIM_NVIC_CHANNEL);    
}

void CMOD_Initialize_InsertionInterrupt()
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

void CMOD_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_RESET_BUS,  ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CS_BUS,     ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_RD_BUS,     ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_WR_BUS,     ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DETECT_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_ADDR_BUS,   ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DATA_BUS,   ENABLE);

    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)        \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;     \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;     \
    GPIO_InitObject.GPIO_Pin   = PIN;               \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;  \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; \
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(CMOD_RESET_PORT, CMOD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_CS_PORT,    CMOD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_RD_PORT,    CMOD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_WR_PORT,    CMOD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(CMOD_ADDR_PORT,  CMOD_ADDR_PINS);
    INITIALIZE_OUTPUT_PIN(CMOD_DATA_PORT,  CMOD_DATA_PINS);
 
    CMOD_SET_CS;
    CMOD_SET_RD;
    CMOD_SET_WR;
    CMOD_SET_RESET;
    
    CMOD_Initialize_CLK();
    CMOD_Initialize_InsertionInterrupt();
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)             
  { // Set WR after a Write, if last operation was a read it is already set, GB does that 20ns before the CLK rises                                                          
      CMOD_SET_WR; 
      CMOD_SET_CS;                                               // CS rises at 0ns (after CLK Flank) 
                                                               
      if (CMOD_Action == CMOD_READ && CMOD_BytesRead != 0)       // If a Byte was read, store it
      {                                                          // Gameboy stops driving Data Bus at 0ns -> Data ready
          CMOD_DataIn[CMOD_BytesRead -1] = CMOD_GET_DATA();      // Store the nt byte at CMOD_DataIn[n-1]
      }
      
      if (CMOD_Action == CMOD_READ && CMOD_BytesRead == CMOD_BytesToRead)          // Read all Data?
      {                                                                          
          CMOD_Status = CMOD_DATA_READY;                                           // -> Data Ready
          CMOD_Action = CMOD_NOACTION;                                             // All actions finished
          CMOD_DisableInterrupt();                                                 // Disable Interrupt until needed again
          TIM_ClearITPendingBit(TIM4, TIM_IT_Update);                              // Leave Interrupt Handler 
          return;                                                // Needed?
      }
      else if (CMOD_Action == CMOD_WRITE && CMOD_BytesWritten == CMOD_BytesToWrite)// All Bytes written?
      {
          CMOD_Status = CMOD_WRITE_COMPLETE;                                       // -> Write complete
          CMOD_Action = CMOD_NOACTION;                                             // All actions finished
          CMOD_DisableInterrupt();                                                 // Disable Interrupt until needed again
          TIM_ClearITPendingBit(TIM4, TIM_IT_Update);                              // Leave Interrupt Handler
          return;                                                // Needed?
      }
      
      if (CMOD_Action == CMOD_READ)                              // Do we want to read?
      {
          CMOD_RST_RD;                                           // RD goes low for a read at 30ns (in GB)
          CMOD_SET_ADDR(CMOD_Address + CMOD_BytesRead);          // Address changes at 140ns (in GB)
          CMOD_RST_CS;                                           // CS goes low at 240ns (in GB)
          CMOD_BytesRead++;                                      // Increase BytesRead in good faith
          CMOD_DATA_MODE_IN;                                     // Set the DataPins to Input mode for a read
      }
      else if (CMOD_Action == CMOD_WRITE)
      {
          CMOD_SET_RD;                                           // RD rises before a write at 140ns (in GB)
          CMOD_SET_ADDR(CMOD_Address + CMOD_BytesWritten);       // Address changes at 140ns (in GB)
          CMOD_RST_CS;                                           // CS goes low at 240ns (in GB)
          CMOD_BytesWritten++;                                   // Increase BytesWritten in good faith
          CMOD_DATA_MODE_OUT;                                    // Set the DataPins to Output mode for a write
          CMOD_SET_DATA(CMOD_DataOut[CMOD_BytesWritten - 1]);    // Data Bus is driven at 480ns (falling CLK) (in GB)
          CMOD_RST_WR;                                           // WR goes low for a write at 480ns (falling CLK) (in GB)
      }   
    
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
