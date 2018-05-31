#include "lcd.h"
#include "lcd_regdef.h"
#include "gbc_gpu.h"

TIM_HandleTypeDef  LCD_TIM_Handle_Backlight;
TIM_HandleTypeDef  LCD_TIM_Handle_PixelTransferTiming;

TIM_OC_InitTypeDef OC_Config_Data;
TIM_OC_InitTypeDef OC_Config_WR_Rst;
TIM_OC_InitTypeDef OC_Config_WR_Set;

DMA_HandleTypeDef LCD_DMA_Handle_Data;
DMA_HandleTypeDef LCD_DMA_Handle_WR_Set;
DMA_HandleTypeDef LCD_DMA_Handle_WR_Rst;

LCD_Pixel_t LCD_FrameBuffer[LCD_DISPLAY_PIXELS];

bool LCD_READY_FLAG;

void LCD_InitializePins(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)       \
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;    \
    GPIO_InitObject.Pin   = PIN;                     \
    GPIO_InitObject.Pull  = GPIO_NOPULL;              \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LCD_RESET_PORT, LCD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RS_PORT,    LCD_RS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_CS_PORT,    LCD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RD_PORT,    LCD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_WR_PORT,    LCD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_DATA_PORT,  GPIO_PIN_All);

    LCD_SET_RESET;
    LCD_SET_RS;
    LCD_SET_CS;
    LCD_SET_RD;
    LCD_SET_WR;
}

void LCD_InitializeBacklight()
{
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = LCD_PIN_BACKLIGHT;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = LCD_ALT_BACKLIGHT;
    HAL_GPIO_Init(LCD_PORT_BACKLIGHT, &GPIO_InitObject);

    LCD_TIM_Handle_Backlight.Instance               = LCD_TIM_BACKLIGHT;
    LCD_TIM_Handle_Backlight.Init.Prescaler         = 37;
    LCD_TIM_Handle_Backlight.Init.CounterMode       = TIM_COUNTERMODE_UP;
    LCD_TIM_Handle_Backlight.Init.Period            = 626;
    LCD_TIM_Handle_Backlight.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    LCD_TIM_Handle_Backlight.Init.RepetitionCounter = 0;

    static TIM_OC_InitTypeDef LCD_TIM_OC_Init_Backlight;
    LCD_TIM_OC_Init_Backlight.Pulse        = 281; 
    LCD_TIM_OC_Init_Backlight.OCMode       = TIM_OCMODE_PWM1;
    LCD_TIM_OC_Init_Backlight.OCFastMode   = TIM_OCFAST_DISABLE;
    LCD_TIM_OC_Init_Backlight.OCPolarity   = TIM_OCPOLARITY_LOW;
    LCD_TIM_OC_Init_Backlight.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    LCD_TIM_OC_Init_Backlight.OCIdleState  = TIM_OCIDLESTATE_RESET;
    LCD_TIM_OC_Init_Backlight.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_Init(&LCD_TIM_Handle_Backlight);
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_Backlight, &LCD_TIM_OC_Init_Backlight, LCD_TIM_BACKLIGHT_CHANNEL);
    HAL_TIM_PWM_Start(&LCD_TIM_Handle_Backlight, LCD_TIM_BACKLIGHT_CHANNEL);
}

void LCD_InitializeDMATimer(void)
{
    // DEBUG: Configure alternate function for WR pin
    //GPIO_InitTypeDef GPIO_InitObject;
    //GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    //GPIO_InitObject.Pin       = GPIO_PIN_6;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = GPIO_AF3_TIM8;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitObject);

    // DEBUG: Configure data transfer timer signal output pin
    //GPIO_InitTypeDef GPIO_InitObject;
    //GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    //GPIO_InitObject.Pin       = GPIO_PIN_7;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = GPIO_AF3_TIM8;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitObject);

    // Configure pixel transfer timing
    LCD_TIM_Handle_PixelTransferTiming.Instance                = LCD_DATA_WR_TIM;
    LCD_TIM_Handle_PixelTransferTiming.Init.Period             = 40; // 200 MHz / 40 = 5 MHz, fast enough to send 76800 pixels in about 15.36ms
    LCD_TIM_Handle_PixelTransferTiming.Init.Prescaler          = 0;
    LCD_TIM_Handle_PixelTransferTiming.Init.CounterMode        = TIM_COUNTERMODE_UP;
    LCD_TIM_Handle_PixelTransferTiming.Init.ClockDivision      = TIM_CLOCKDIVISION_DIV1;
    LCD_TIM_Handle_PixelTransferTiming.Init.RepetitionCounter  = 0;
    LCD_TIM_Handle_PixelTransferTiming.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&LCD_TIM_Handle_PixelTransferTiming);

    // Configure data pulse width modulation
    OC_Config_Data.Pulse        = 1;
    OC_Config_Data.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_Data.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_Data.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_Data.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_Data.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_Data.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_Data, LCD_DATA_TIM_CHANNEL);

    // Configure WR reset pulse width modulation
    OC_Config_WR_Rst.Pulse        = 1;
    OC_Config_WR_Rst.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_WR_Rst.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_WR_Rst.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_WR_Rst.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_WR_Rst.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_WR_Rst.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_WR_Rst, LCD_WR_RST_TIM_CHANNEL);

    // Configure WR set pulse width modulation
    OC_Config_WR_Set.Pulse        = 30;
    OC_Config_WR_Set.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_WR_Set.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_WR_Set.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_WR_Set.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_WR_Set.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_WR_Set.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_WR_Set, LCD_WR_SET_TIM_CHANNEL);
}

void LCD_InitializeDMA(void)
{
    // Configure DMA for data
    LCD_DMA_Handle_Data.Instance                 = DMA1_Stream0;
    LCD_DMA_Handle_Data.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_Data.Init.Request             = DMA_REQUEST_TIM8_CH2;
    LCD_DMA_Handle_Data.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_Data.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_Data.Init.MemInc              = DMA_MINC_ENABLE;
    LCD_DMA_Handle_Data.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_Data.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_Data.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_Data.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    LCD_DMA_Handle_Data.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    LCD_DMA_Handle_Data.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_Data.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_Data.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_Data);

    // Configure DMA for WR set
    LCD_DMA_Handle_WR_Set.Instance                 = DMA1_Stream1;
    LCD_DMA_Handle_WR_Set.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_WR_Set.Init.Request             = DMA_REQUEST_TIM8_CH1;
    LCD_DMA_Handle_WR_Set.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_WR_Set.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_WR_Set.Init.MemInc              = DMA_MINC_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Set.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Set.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_WR_Set.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_WR_Set);

    // Configure DMA for WR rst
    LCD_DMA_Handle_WR_Rst.Instance                 = DMA1_Stream2;
    LCD_DMA_Handle_WR_Rst.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_WR_Rst.Init.Request             = DMA_REQUEST_TIM8_CH3;
    LCD_DMA_Handle_WR_Rst.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_WR_Rst.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_WR_Rst.Init.MemInc              = DMA_MINC_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Rst.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Rst.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_WR_Rst.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_WR_Rst);

    // Link DMA with pixel transfer timing
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC2], LCD_DMA_Handle_Data);
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC1], LCD_DMA_Handle_WR_Set);
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC3], LCD_DMA_Handle_WR_Rst);

    // Enable interrupt handler
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);

    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

void LCD_InitializeInterrupt(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin  = INPUT_FRAME_PIN;
    HAL_GPIO_Init(INPUT_FRAME_PORT, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
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

    __HAL_TIM_SET_COMPARE(&LCD_TIM_Handle_Backlight, LCD_TIM_BACKLIGHT_CHANNEL, 2.81f * percent);
}

// Write register address
void LCD_WriteAddr(uint16_t addr)
{
    LCD_DATA_PORT->ODR = addr;
    LCD_RST_RS;
    LCD_RST_WR;
    LCD_SET_WR;
    LCD_SET_RS;
}

// Write data
void LCD_WriteData(uint16_t data)
{
    LCD_DATA_PORT->ODR = data;
    LCD_RST_WR;
    LCD_SET_WR;
}

// Write Command without data
void LCD_WriteCommand(uint16_t addr)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_SET_CS;
}

// Write Command with single byte data
void LCD_WriteCommandWithData(uint16_t addr, uint16_t data)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    LCD_WriteData(data);
    LCD_SET_CS;
}

// Write Command with data consisting of multiply parameters
void LCD_WriteCommandWithParameters(uint16_t addr, uint16_t parameters[], long length)
{
    LCD_RST_CS;
    LCD_WriteAddr(addr);
    for (long i = 0; i < length; i++)
    {
        LCD_WriteData(parameters[i]);
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
    LCD_WriteCommandWithParameters(LCD_REG_COLUMN_ADDRESS_SET, columnAddressSetData.Data, 4);
}

void LCD_SetDrawAreaVertical(uint16_t startRow, uint16_t endRow)
{
    PageAddressSetData_t pageAddressSetData = {0};
    pageAddressSetData.StartPageHigh = startRow >> 8;
    pageAddressSetData.StartPageLow  = startRow & 0xFF;
    pageAddressSetData.EndPageHigh   = endRow >> 8;
    pageAddressSetData.EndPageLow    = endRow & 0xFF;
    LCD_WriteCommandWithParameters(LCD_REG_PAGE_ADDRESS_SET, pageAddressSetData.Data, 4);
}

void LCD_SetDrawArea(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
    if (width == 0 || height == 0) return;

    LCD_SetDrawAreaHorizontal(x0, x0 + width - 1);
    LCD_SetDrawAreaVertical(y0, y0 + height - 1);
}

// Remove this function along with LCD_FrameRateControlData_t if frame rate doesn't need to be changed during runtime
void LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO_t divRatio, LCD_FRAME_RATE_t frameRate)
{
    LCD_FrameRateControlData_t frameRateControlData = {0};
    frameRateControlData.DivisionRatio              = divRatio;
    frameRateControlData.FrameRate                  = frameRate;
    LCD_WriteCommandWithParameters(LCD_REG_FRAME_RATE_CONTROL, frameRateControlData.Data, 2);
}

void LCD_SetDrawBehaviour(bool writeBotToTop, bool writeRightToLeft, bool rowColExchange, bool refreshBotToTop, bool refreshRightToLeft, bool bgr)
{
    MemoryAccessControlData_t memoryAccessControlData = {0};
    memoryAccessControlData.RowAddressOrder           = writeBotToTop;
    memoryAccessControlData.ColumnAddressOrder        = writeRightToLeft;
    memoryAccessControlData.RowColumnExchange         = rowColExchange;
    memoryAccessControlData.VerticalRefreshOrder      = refreshBotToTop;
    memoryAccessControlData.HorizontalRefreshOrder    = refreshRightToLeft;
    memoryAccessControlData.RGBBGROrder               = bgr;

    LCD_WriteCommandWithData(LCD_REG_MEMORY_ACCESS_CONTROL, memoryAccessControlData.Data);
}

void LCD_Initialize(void)
{
// Reminder: Porching increases available time to write from RAM to GRAM
    LCD_InitializePins();
    LCD_InitializeBacklight();
    LCD_InitializeInterrupt();
    LCD_InitializeDMATimer();
    LCD_InitializeDMA();
    

    LCD_WriteCommand(LCD_REG_SOFTWARE_RESET);
    HAL_Delay(120); // Wait ~120ms (ILI9341 Datasheet p. 90)
    LCD_WriteCommand(LCD_REG_DISPLAY_OFF);


    LCD_WriteCommandWithData(LCD_REG_POWER_CONTROL_1, LCD_REG_POWER_CONTROL1_DATA);
    LCD_WriteCommandWithData(LCD_REG_POWER_CONTROL_2, LCD_REG_POWER_CONTROL2_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_VCOM_CONTROL_1, LCD_REG_VCOM_CONTROL_DATA, 2);
    LCD_WriteCommandWithData(LCD_REG_VCOM_CONTROL_2, LCD_REG_VCOM_CONTROL2_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_DISPLAY_FUNCTION_CONTROL, LCD_REG_DISPLAY_FUNCTION_CONTROL_DATA, 4);
    LCD_WriteCommandWithData(LCD_REG_PIXEL_FORMAT_SET, LCD_REG_PIXEL_FORMAT_DATA);
    LCD_WriteCommandWithData(LCD_REG_ENTRY_MODE_SET, LCD_REG_ENTRY_MODE_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_POSITIVE_GAMMA_CORRECTION, LCD_REG_POSITIVE_GAMMA_CORRECTION_DATA, 15);
    LCD_WriteCommandWithParameters(LCD_REG_NEGATIVE_GAMMA_CORRECTION, LCD_REG_NEGATIVE_GAMMA_CORRECTION_DATA, 15);

    LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO1, LCD_FRAME_RATE_61HZ);
    LCD_SetDrawBehaviour(true, true, true, true, false, true);


    //LCD_WriteCommandWithData(LCD_REG_GAMMA_SET, 0x01);             // Default value, why even bother?
    LCD_WriteCommandWithData(LCD_REG_TEARING_EFFECT_LINE_ON, 0);   // Default value, why even bother?
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y); // necessary?

    LCD_WriteCommand(LCD_REG_SLEEP_OUT);
    LCD_WriteCommand(LCD_REG_DISPLAY_ON);
}

// Print whole Display in given color
void LCD_ClearColor(uint16_t color)
{
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (long i = 0; i < LCD_DISPLAY_PIXELS; i++)
    {
        LCD_DATA_PORT->ODR = color;
        LCD_RST_WR;
        LCD_SET_WR;
    }
    LCD_SET_CS;
}

// Not very time efficient when called thousands of times but totally fine for drawing symbols etc.
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetDrawArea(x, y, 1, 1);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);

    LCD_DATA_PORT->ODR = color;
    LCD_RST_WR;
    LCD_SET_WR;
    LCD_SET_CS;

    // Set drawarea back to full size
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);
}

// This function is used to increase readability
// However if speed is important rather use DrawFilledBox
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t width, uint16_t color, LCD_Orientation_t o)
{
    if (o == LCD_VERTICAL) LCD_DrawFilledBox(x0, y0, width, length, color);
    else                   LCD_DrawFilledBox(x0, y0, length, width, color);
}

// Again his function is used to increase readability, for more speed use DrawFilledBox directly or use a buffer
void LCD_DrawEmptyBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t width, uint16_t color)
{   
    // Draw horizontal lines of the box
    LCD_DrawLine(x0, y0, length, width, color, LCD_HORIZONTAL);
    LCD_DrawLine(x0, y0 + height - width, length, width, color, LCD_HORIZONTAL);
    // Draw vertical lines of the box
    LCD_DrawLine(x0, y0 + width, height - 2 * width, width, color, LCD_VERTICAL);
    LCD_DrawLine(x0 + length - width, y0 + width, height - 2 * width, width, color, LCD_VERTICAL);
}

// This function doesn't use DrawPixel in order to avoid unneccessary function calls
void LCD_DrawFilledBox(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, uint16_t color)
{
    LCD_SetDrawArea(x0, y0, length, height);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (long i = 0; i < length * height; i++)
    {
        LCD_DATA_PORT->ODR = color;
        LCD_RST_WR;
        LCD_SET_WR;
    }
    LCD_SET_CS;

    // Set drawarea back to full size
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);
}

// Draws a line of bricks, use height to truncate the line
void LCD_DrawBrickline(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool offset, LCD_Brick_t *p_brick)
{
    uint16_t x;

    // Draw either a full or an half brick at the beginning of every brick line, depending on offset
    LCD_DrawFilledBox(x0, y0, (p_brick->Length / (offset + 1)), height, p_brick->Color);

    // Draw as many bricks as fully fit into the line
    for (x = (x0 + p_brick->Length / (offset + 1)); x < (x0 + length - p_brick->Length - 2 * p_brick->Border.Width);)
    {
        LCD_DrawLine(x, y0, height, p_brick->Border.Width, p_brick->Border.Color, LCD_VERTICAL);
        x += p_brick->Border.Width;
        LCD_DrawFilledBox(x, y0, p_brick->Length, height, p_brick->Color);
        x += p_brick->Length;
    }

    // Draw as much of last brick as possible
    LCD_DrawLine(x, y0, height, p_brick->Border.Width, p_brick->Border.Color, LCD_VERTICAL);
    x += p_brick->Border.Width;
    LCD_DrawFilledBox(x, y0, x0 + length - x, height, p_brick->Color);

    // Draw the bottom horizontal brick border if line is not truncated
    if (height >= p_brick->Height)
    {
        LCD_DrawLine(x0, y0 + height, length, p_brick->Border.Width, p_brick->Border.Color, LCD_HORIZONTAL);
    }
}

void LCD_DrawWall(uint16_t x0, uint16_t y0, uint16_t length, uint16_t height, bool initOffset, LCD_Brick_t *p_brick)
{
    uint16_t y = y0;
    bool offset = initOffset;

    // Draw as many brick lines as fully fit into wall height
    for (; y < (y0 + height - p_brick->Height - p_brick->Border.Width); y += (p_brick->Height + p_brick->Border.Width))
    {
        LCD_DrawBrickline(x0, y, length, p_brick->Height, offset, p_brick);

        // Shift every second line to get the wall pattern
        if (offset) offset = false;
        else        offset = true;
    }

    // Draw as much of the last line as possible
    LCD_DrawBrickline(x0, y, length, y0 + height - y, offset, p_brick);
}

// Draws a text in a colored box, using a certain font, starting at x0/y0 (upper left corner of the box) 
void LCD_DrawText(uint16_t x0, uint16_t y0, uint16_t bgColor, LCD_TextDef_t *text, Fonts_FontDef_16_t *p_font)
{
    // Calculate number of characters to draw and replace unknown chars with '?'
    int chars = 0;
    for (int i = 0; text->Characters[i] != '\0'; i++)
    {
        if (text->Characters[i] < 32 || text->Characters[i] > 126) text->Characters[i] = '?';
        chars ++;
    }

    // Calculate needed size
    int length = text->Padding.Left + chars * (p_font->FontWidth + text->Spacing) + text->Padding.Right;
    int height = text->Padding.Upper + p_font->FontHeight + text->Padding.Lower;
    int size   = length * height;

    // Initialize FrameBuffer with background color
    for (int i = 0; i < size; i++)
    {
        GBC_GPU_FrameBuffer[i].Color = bgColor;
    }

    // Draw first line of all chars, then second one, ...
    for (int yChar = 0; yChar < p_font->FontHeight; yChar++)
    {
        for (int c = 0; c < chars; c++)
        {
            for (int xChar = 0; xChar < p_font->FontWidth; xChar++)
            {
                // Calculate the current buffer coordinates
                int xBuffer = text->Padding.Left + c * (p_font->FontWidth + text->Spacing) + xChar;
                int yBuffer = text->Padding.Upper + yChar;

                // Each byte of fontData represents a line of a char where a set bit means a pixel must be drawn
                // All lines of a char are stored sequentially in fontData, same goes for all chars
                // The first char is space (ASCII 32), afterwards all chars are stored in ASCII order
                uint16_t currentCharLine = p_font->FontData[(text->Characters[c] - 32) * p_font->FontHeight + yChar];

                // If FontData is mirrored check lines from right to left
                if ((p_font->LettersMirrored && ((currentCharLine >> xChar) & 0x0001)) ||
                   (!p_font->LettersMirrored && ((currentCharLine << xChar) & 0x8000)))
                {
                    // If a bit is set write textColor to the current Buffer position
                    // Padding Right and Bot is accomplished because buffer is actually
                    // Left Padding + c(MAX) * (FontWidth + textSpacing) + xFont(MAX) !+ Right Padding! long
                    // and Upper Padding + yFont(MAX) !+Lower Padding! high
                    GBC_GPU_FrameBuffer[length * yBuffer + xBuffer].Color = text->Color;
                }
            }
        }
    }

    // Draw the text border (if border width is 0 no border will be drawn)
    uint16_t borderWidth = text->Border.Width;
    LCD_DrawEmptyBox(x0, y0, length + 2 * borderWidth, height + 2 * borderWidth, borderWidth, text->Border.Color);

    // Draw the Text and its background from the now initialized buffer
    LCD_SetDrawArea(x0 + text->Border.Width, y0 + text->Border.Width, length, height);
    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (long i = 0; i < size; i++)
    {
        LCD_DATA_PORT->ODR = GBC_GPU_FrameBuffer[i].Color;
        LCD_RST_WR;
        LCD_SET_WR;
    }
    LCD_SET_CS;

    // Set drawarea back to full size
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);
}

void LCD_DrawSymbol(uint16_t x0, uint16_t y0, uint16_t color, Fonts_SymbolDef_32_t *p_symbol)
{
    for (int y = 0; y < p_symbol->SymbolHeight; y++)
    {
        for (int x = 0; x < p_symbol->SymbolWidth; x++)
        {
            uint32_t currentSymbolLine = p_symbol->SymbolData[y];
            if ((currentSymbolLine << x) & 0x80000000)
            {
                LCD_DrawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}

void LCD_DrawFrameBuffer(uint16_t* frameBuffer, uint32_t frameBufferLength, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
    // Disable the TIM update DMA request
    __HAL_TIM_DISABLE_DMA(&LCD_TIM_Handle_PixelTransferTiming, TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3);

    DMA_Stream_TypeDef* h1 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_Data.Instance);
    DMA_Stream_TypeDef* h2 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_WR_Set.Instance);
    DMA_Stream_TypeDef* h3 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_WR_Rst.Instance);

    // Abort DMA transfer and reset DMA handles
    //HAL_DMA_Abort(&LCD_DMA_Handle_Data);
    //HAL_DMA_Abort(&LCD_DMA_Handle_WR_Set);
    //HAL_DMA_Abort(&LCD_DMA_Handle_WR_Rst);

    // ToDo: Move to menu so it's only called on change
    LCD_SetDrawArea(x0, y0, width, height);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);

    // Enable transfer complete IT
    h1->CR |= DMA_IT_TC;
    h2->CR |= DMA_IT_TC;
    h3->CR |= DMA_IT_TC;

    // Reset TIM counter
    LCD_DATA_WR_TIM->CNT = 0;

    // Enable capture/compare
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_DATA_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_WR_SET_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_WR_RST_TIM_CHANNEL);

    // Start DMA transfer
    if (frameBufferLength > 65535)
    {
        // ToDo
    }
    else
    {
        static const uint8_t WR_SetByte = 0xFF;
        static const uint8_t WR_RstByte = 0x00;

        // Configure DMA transfer
        /*HAL_DMA_Config(&LCD_DMA_Handle_Data,   (uint32_t)frameBuffer, (uint32_t)(&LCD_DATA_PORT->ODR), frameBufferLength);
        HAL_DMA_Config(&LCD_DMA_Handle_WR_Set, (uint32_t)&WR_SetByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength + 1);
        HAL_DMA_Config(&LCD_DMA_Handle_WR_Rst, (uint32_t)&WR_RstByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength);

        // Enable DMA transfer
        h1->CR |= DMA_SxCR_EN;
        h2->CR |= DMA_SxCR_EN;
        h3->CR |= DMA_SxCR_EN;*/

        HAL_DMA_Start(&LCD_DMA_Handle_Data,   (uint32_t)frameBuffer, (uint32_t)(&LCD_DATA_PORT->ODR), frameBufferLength);
        HAL_DMA_Start(&LCD_DMA_Handle_WR_Set, (uint32_t)&WR_SetByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength + 1);
        HAL_DMA_Start(&LCD_DMA_Handle_WR_Rst, (uint32_t)&WR_RstByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength);
    }

    // Enable the TIM update DMA request
    __HAL_TIM_ENABLE_DMA(&LCD_TIM_Handle_PixelTransferTiming, TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3);

    // Enable main output
    __HAL_TIM_MOE_ENABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Enable timer
    __HAL_TIM_ENABLE(&LCD_TIM_Handle_PixelTransferTiming);
}

void LCD_DrawGBCFrameBuffer(void)
{
    LCD_DrawFrameBuffer((uint16_t*)&GBC_GPU_FrameBuffer, GBC_GPU_FRAME_SIZE,
        80, 48, GBC_GPU_FRAME_SIZE_X, GBC_GPU_FRAME_SIZE_Y); // Set Draw Area to the middle 160x144px for non scaled display
}

void LCD_DrawGBCFrameBufferScaled(void)
{
    for (int y = 0, i = 0, line = 0, timesLineDrawn = 0, linesDrawn = 0; y < LCD_DISPLAY_SIZE_Y; y++)
    {
        for (int x = 0; x < GBC_GPU_FRAME_SIZE_X; x++)
        {
            LCD_FrameBuffer[i++].Color = GBC_GPU_FrameBuffer[line + x].Color;
            LCD_FrameBuffer[i++].Color = GBC_GPU_FrameBuffer[line + x].Color; // Draw every pixel in a line twice to achieve a width of 320 pixel
        }
        timesLineDrawn++;
        linesDrawn++;

        // If a line has been drawn twice or if it's the fifth line draw the next line (twice)
        if (timesLineDrawn == 2 || linesDrawn == 5) { line += 160; timesLineDrawn = 0; }
        if (linesDrawn == 5) linesDrawn = 0;
    }

    for (int i = 0; i < 5; i++)
    {
        LCD_FrameBuffer[i].Color = 0;
        LCD_FrameBuffer[i].Red = 31;
    }
    for (int i = 5; i < 10; i++)
    {
        LCD_FrameBuffer[i].Color = 0;
        LCD_FrameBuffer[i].Blue = 31;
    }

    LCD_DrawFrameBuffer((uint16_t*)&LCD_FrameBuffer, 10,
        0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);
}

void DMA1_Stream0_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_Data);

    // Disable timer
    __HAL_TIM_DISABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Disable capture/compare
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_DATA_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_WR_SET_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_WR_RST_TIM_CHANNEL);

    // Disable main output
    __HAL_TIM_MOE_DISABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Reset timer state
    LCD_TIM_Handle_PixelTransferTiming.State = HAL_TIM_STATE_READY;

    LCD_SET_WR;
    LCD_SET_CS;
}

void DMA1_Stream1_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_WR_Set);
}

void DMA1_Stream2_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_WR_Rst);
}

// ToDo: Disable when not needed
void EXTI0_IRQHandler(void)
{ 
    if (__HAL_GPIO_EXTI_GET_IT(INPUT_FRAME_PIN) != RESET) 
    {
        if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
        {
            LCD_SET_READY_FLAG;
        }
        else
        {
            LCD_RST_READY_FLAG;
        }

        __HAL_GPIO_EXTI_CLEAR_IT(INPUT_FRAME_PIN);        
    }
}
