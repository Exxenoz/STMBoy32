#include "lcd.h"
#include "gbc_gpu.h"

bool LCD_READY_FLAG;

void LCD_InitializePins(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)        \
    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_OUT;     \
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;     \
    GPIO_InitObject.GPIO_Pin   = PIN;               \
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;  \
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz; \
    GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LCD_RESET_PORT, LCD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RS_PORT,    LCD_RS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_CS_PORT,    LCD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RD_PORT,    LCD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_WR_PORT,    LCD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_DATA_PORT,  GPIO_Pin_All);

    GPIO_InitObject.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitObject.GPIO_OType = GPIO_OType_PP;
    GPIO_InitObject.GPIO_Pin   = LCD_BACKLIT_PIN;
    GPIO_InitObject.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitObject.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(LCD_BACKLIT_PORT, &GPIO_InitObject);

    GPIO_PinAFConfig(LCD_BACKLIT_PORT, LCD_BACKLIT_PIN_SOURCE, GPIO_AF_TIM1);

    LCD_SET_RESET;
    LCD_SET_RS;
    LCD_SET_CS;
    LCD_SET_RD;
    LCD_SET_WR;
}

void LCD_InitializeTimer()
{
    TIM_TimeBaseInitTypeDef TIM_BaseObject;
    TIM_OCInitTypeDef       TIM_OCInitObject;


    TIM_BaseObject.TIM_Prescaler            = 16;
    TIM_BaseObject.TIM_CounterMode          = TIM_CounterMode_Up;
    TIM_BaseObject.TIM_Period               = 281;
    TIM_BaseObject.TIM_ClockDivision        = TIM_CKD_DIV1;
    TIM_BaseObject.TIM_RepetitionCounter    = 0;
    TIM_TimeBaseInit(LCD_TIM, &TIM_BaseObject);

    TIM_OCStructInit(&TIM_OCInitObject);
    TIM_OCInitObject.TIM_OCMode             = TIM_OCMode_PWM1;
    TIM_OCInitObject.TIM_OutputState        = TIM_OutputState_Enable;
    TIM_OCInitObject.TIM_OCPolarity         = TIM_OCPolarity_Low;
    TIM_OCInitObject.TIM_Pulse              = 281; 
    TIM_OC4Init(LCD_TIM, &TIM_OCInitObject);
    TIM_OC4PreloadConfig(LCD_TIM, TIM_OCPreload_Enable);

    TIM_ClearITPendingBit(LCD_TIM, TIM_IT_CC4);
    TIM_Cmd(LCD_TIM, ENABLE);

    TIM_CtrlPWMOutputs(LCD_TIM, ENABLE);  
}

void LCD_InitializeInterrupt(void)
{
    // SYSCFG APB clock must be enabled to get write access to SYSCFG_EXTICRx
    // registers using RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    EXTI_InitTypeDef EXTI_InitObject;
    NVIC_InitTypeDef NVIC_InitObject;


    SYSCFG_EXTILineConfig(INPUT_FRAME_EXTI_PORT, INPUT_FRAME_EXTI_PIN);
    
    EXTI_InitObject.EXTI_Line    = INPUT_FRAME_EXTI_LINE;                         
    EXTI_InitObject.EXTI_Mode    = EXTI_Mode_Interrupt;          
    EXTI_InitObject.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
    EXTI_InitObject.EXTI_LineCmd = ENABLE;                       
    EXTI_Init(&EXTI_InitObject);
    
    NVIC_InitObject.NVIC_IRQChannel                   = INPUT_FRAME_NVIC_CHANNEL; 
    NVIC_InitObject.NVIC_IRQChannelPreemptionPriority = 0x0F;    
    NVIC_InitObject.NVIC_IRQChannelSubPriority        = 0x0F;    
    NVIC_InitObject.NVIC_IRQChannelCmd                = ENABLE;  
    NVIC_Init(&NVIC_InitObject);                                 
    NVIC_EnableIRQ(INPUT_FRAME_NVIC_CHANNEL);
}

bool LCD_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(LCD_RESET_BUS,   ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_RS_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_CS_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_RD_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_WR_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_BACKLIT_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_DATA_BUS,    ENABLE);

    RCC_APB2PeriphClockCmd(LCD_TIM_BUS,     ENABLE);

    LCD_InitializePins();
    LCD_InitializeTimer();
    LCD_InitializeInterrupt();

    LCD_WriteCommand(LCD_REG_SOFTWARE_RESET);
    // Wait ~120ms (ILI9341 Datasheet p. 90)
    for (long i = 0; i < 2000000; i++);

    LCD_WriteCommand(LCD_REG_DISPLAY_OFF);

    LCD_Write(LCD_REG_POWER_CONTROL_1, 0x09);
    LCD_Write(LCD_REG_POWER_CONTROL_2, 0x13);

    uint16_t data[16];
    data[0] = 0x35; 
    data[1] = 0x3E;
    LCD_WriteBuffer(LCD_REG_VCOM_CONTROL_1, data, 2);
    LCD_Write(LCD_REG_VCOM_CONTROL_2, 0xBE);

    MemoryAccessControlData_t memoryAccessControlData = {0};
    memoryAccessControlData.RowAddressOrder           = 1;
    memoryAccessControlData.ColumnAddressOrder        = 1;
    memoryAccessControlData.RowColumnExchange         = 1;
    memoryAccessControlData.VerticalRefreshOrder      = 1;
    memoryAccessControlData.RGBBGROrder               = 1;
    memoryAccessControlData.HorizontalRefreshOrder    = 0;
    LCD_Write(LCD_REG_MEMORY_ACCESS_CONTROL, memoryAccessControlData.Data);

    LCD_Write(LCD_REG_PIXEL_FORMAT_SET, 0x55);

    LCD_Write(LCD_REG_GAMMA_SET, 0x01);

    data[0]  = 0x1f;
    data[1]  = 0x1a;
    data[2]  = 0x18;
    data[3]  = 0x0a;
    data[4]  = 0x0f;
    data[5]  = 0x06;
    data[6]  = 0x45;
    data[7]  = 0x87;
    data[8]  = 0x32;
    data[9]  = 0x0a;
    data[10] = 0x07;
    data[11] = 0x02;
    data[12] = 0x07;
    data[13] = 0x05;
    data[14] = 0x00;
    LCD_WriteBuffer(LCD_REG_POSITIVE_GAMMA_CORRECTION, data, 15);

    data[0]  = 0x00;
    data[1]  = 0x25;
    data[2]  = 0x27;
    data[3]  = 0x05;
    data[4]  = 0x10;
    data[5]  = 0x09;
    data[6]  = 0x3A;
    data[7]  = 0x78;
    data[8]  = 0x4D;
    data[9]  = 0x05;
    data[10] = 0x18;
    data[11] = 0x0D;
    data[12] = 0x38;
    data[13] = 0x3A;
    data[14] = 0x1F;
    LCD_WriteBuffer(LCD_REG_NEGATIVE_GAMMA_CORRECTION, data, 15);

    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_Y, LCD_DISPLAY_SIZE_X);

    LCD_Write(LCD_REG_TEARING_EFFECT_LINE_ON, 0);

    LCD_FrameRateControlData_t frameRateControlData = {0};
    frameRateControlData.DivisionRatio              = LCD_FRAME_RATE_CONTROL_DATA_DIVISION_RATIO_2;
    frameRateControlData.FrameRate                  = LCD_FRAME_RATE_CONTROL_DATA_FRAME_RATE_61HZ;
    LCD_WriteBuffer(LCD_REG_FRAME_RATE_CONTROL, frameRateControlData.Data, 2);

    LCD_Write(LCD_REG_ENTRY_MODE_SET, 0x07);

    data[0] = 0x0A;
    data[1] = 0x82;
    data[2] = 0x27;
    data[3] = 0x00;
    LCD_WriteBuffer(LCD_REG_DISPLAY_FUNCTION_CONTROL, data, 4);

    // Setting of porching is needed, because it increases the
    // available time to write from RAM to GRAM without tearing.
    /*data[0] = 0x5F; // Front Porching
    data[1] = 0x5F; // Back  Porching
    data[2] = 0x0A; // Default
    data[3] = 0x14; // Default
    LCD_WriteBuffer(LCD_REG_BLANKING_PORCH_CONTROL, data, 4);*/

    LCD_WriteCommand(LCD_REG_SLEEP_OUT);
    LCD_WriteCommand(LCD_REG_DISPLAY_ON);

    // ToDo: Move to somewhere else
    LCD_DimBacklight(0);

    return false; 
}

void LCD_DimBacklight(long percent)
{
    if (percent < 0)
    {
        percent = 0;
    }
    else if (percent > 100)
    {
        percent = 100;
    }

    TIM_SetCompare4(TIM1, 2.81f * percent);
}

void LCD_WriteAddr(uint16_t addr)
{
    LCD_DATA_PORT->ODR = addr;
    LCD_RST_RS; // Set to command
    LCD_RST_WR; // Set to write
    LCD_SET_WR;
    LCD_SET_RS; // Set to data
}

void LCD_WriteData(uint16_t data)
{
    LCD_DATA_PORT->ODR = data;
    LCD_RST_WR; // Set to write
    LCD_SET_WR;
}

void LCD_WriteCommand(uint16_t addr)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_SET_CS;
}

void LCD_Write(uint16_t addr, uint16_t data)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_WriteData(data);
    LCD_SET_CS;
}

void LCD_WriteBuffer(uint16_t addr, uint16_t buffer[], long length)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    for (long i = 0; i < length; i++)
    {
        LCD_WriteData(buffer[i]);
    }
    LCD_SET_CS;
}

uint16_t LCD_ReadData(void)
{
    LCD_DATA_PORT->MODER = GPIO_INPUT_MODE;
    LCD_RST_RD;
    for (uint16_t i = 0; i < 42; i++);
    uint16_t data = LCD_DATA_PORT->IDR;
    LCD_SET_RD;
    for (uint16_t i = 0; i < 10; i++);
    LCD_DATA_PORT->MODER = GPIO_OUTPUT_MODE;
    return data;
}

uint16_t LCD_Read(uint16_t addr)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    uint16_t data = LCD_ReadData();
    LCD_SET_CS;
    return data;
}

void LCD_ReadBuffer(uint16_t addr, uint16_t buffer[], long length)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    for (long i = 0; i < length; i++)
    {
        buffer[i] = LCD_ReadData();
    }
    LCD_SET_CS;
}

void LCD_SetDrawAreaHorizontal(uint16_t startColumn, uint16_t endColumn)
{
    ColumnAddressSetData_t columnAddressSetData = {0};
    columnAddressSetData.StartColumnHigh = startColumn >> 8;
    columnAddressSetData.StartColumnLow  = startColumn & 0xFF;
    columnAddressSetData.EndColumnHigh   = endColumn >> 8;
    columnAddressSetData.EndColumnLow    = endColumn & 0xFF;
    LCD_WriteBuffer(LCD_REG_COLUMN_ADDRESS_SET, columnAddressSetData.Data, 4);
}

void LCD_SetDrawAreaVertical(uint16_t startRow, uint16_t endRow)
{
    PageAddressSetData_t pageAddressSetData = {0};
    pageAddressSetData.StartPageHigh = startRow >> 8;
    pageAddressSetData.StartPageLow  = startRow & 0xFF;
    pageAddressSetData.EndPageHigh   = endRow >> 8;
    pageAddressSetData.EndPageLow    = endRow & 0xFF;
    LCD_WriteBuffer(LCD_REG_PAGE_ADDRESS_SET, pageAddressSetData.Data, 4);
}

void LCD_SetDrawArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    LCD_SetDrawAreaHorizontal(x, x + width - 1);
    LCD_SetDrawAreaVertical(y, y + height - 1);
}

void LCD_ClearColor(uint16_t color)
{
    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (long i = 0; i < LCD_DISPLAY_PIXELS; i++)
    {
        LCD_DATA_PORT->ODR = color;
        LCD_RST_WR; // Set to write
        LCD_SET_WR;
    }
    LCD_SET_CS;
}

void LCD_DrawFrameBuffer(void)
{
    LCD_SetDrawArea(80, 48, GBC_GPU_FRAME_SIZE_X, GBC_GPU_FRAME_SIZE_Y);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (long i = 0; i < GBC_GPU_FRAME_SIZE; i++)
    {
        LCD_DATA_PORT->ODR = GBC_GPU_FrameBuffer[i].Color;
        LCD_RST_WR;
        LCD_SET_WR;
    }
    LCD_SET_CS;
}

void LCD_DrawFrameBufferScaled(void)
{
    int linesDrawn = 0;
    
    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (int y = 1, i = 0; y <= 240; y++)
    {
        for (long x = 0; x < GBC_GPU_FRAME_SIZE_X; x++)
        {
            LCD_DATA_PORT->ODR = GBC_GPU_FrameBuffer[160 * i + x].Color; // ToDo: Use counter variable instead of (160 * i + x) -> Multiplications are slow
            LCD_RST_WR;
            LCD_SET_WR;
            LCD_DATA_PORT->ODR = GBC_GPU_FrameBuffer[160 * i + x].Color; // ToDo: This line is not needed, since ODR should not change ?
            LCD_RST_WR;
            LCD_SET_WR;
        }

        linesDrawn++;

        // ToDo: Do not use % operator, because it is slow. Use counter variable instead!
        if (linesDrawn == 2 || y % 5 == 0) { i++; linesDrawn = 0; }       // Draw all lines twice except every third
    }
    LCD_SET_CS;
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
    {
        if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
        {
            LCD_SET_READY_FLAG;
        }
        else
        {
            LCD_RST_READY_FLAG;
        }

        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
