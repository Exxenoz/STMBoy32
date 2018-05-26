#include "lcd.h"
#include "lcd_regdef.h"
#include "gbc_gpu.h"

TIM_HandleTypeDef  LCD_TIM_Handle_Backlight;
TIM_HandleTypeDef  LCD_TIM_Handle_WR;
TIM_HandleTypeDef  LCD_TIM_Handle_DataTransfer;

DMA_HandleTypeDef LCD_DMA_Handle_DataTransfer;
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
    INITIALIZE_OUTPUT_PIN(LCD_PORT_WR,    LCD_PIN_WR);
    INITIALIZE_OUTPUT_PIN(LCD_PORT_DATA,  GPIO_PIN_All);

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
    //GPIO_InitObject.Pin       = LCD_PIN_WR;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = LCD_ALT_WR;
    //HAL_GPIO_Init(LCD_PORT_WR, &GPIO_InitObject);

    // DEBUG: Configure data transfer timer signal output pin
    //GPIO_InitTypeDef GPIO_InitObject;
    //GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    //GPIO_InitObject.Pin       = GPIO_PIN_7;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = GPIO_AF3_TIM8;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitObject);

    // Configure data transfer timer
    LCD_TIM_Handle_DataTransfer.Instance                = LCD_TIM_DATA;
    LCD_TIM_Handle_DataTransfer.Init.Period             = 40; // 200 MHz / 40 = 5 MHz, fast enough to send 76800 pixels in about 15.36ms
    LCD_TIM_Handle_DataTransfer.Init.Prescaler          = 0;
    LCD_TIM_Handle_DataTransfer.Init.CounterMode        = TIM_COUNTERMODE_UP;
    LCD_TIM_Handle_DataTransfer.Init.ClockDivision      = TIM_CLOCKDIVISION_DIV1;
    LCD_TIM_Handle_DataTransfer.Init.RepetitionCounter  = 0;
    LCD_TIM_Handle_DataTransfer.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&LCD_TIM_Handle_DataTransfer);

    // Configure data transfer pulse width modulation
    static TIM_OC_InitTypeDef OC_Config_DataTransfer;
    OC_Config_DataTransfer.Pulse        = 20;
    OC_Config_DataTransfer.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_DataTransfer.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_DataTransfer.OCPolarity   = TIM_OCPOLARITY_HIGH;
    OC_Config_DataTransfer.OCNPolarity  = TIM_OCNPOLARITY_LOW;
    OC_Config_DataTransfer.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_DataTransfer.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_DataTransfer, &OC_Config_DataTransfer, LCD_TIM_CHANNEL_DATA);

    // Configure WR pulse width modulation
    static TIM_OC_InitTypeDef OC_Config_WR;
    OC_Config_WR.Pulse        = 10;
    OC_Config_WR.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_WR.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_WR.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_WR.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_WR.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_WR.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_DataTransfer, &OC_Config_WR, LCD_TIM_CHANNEL_WR);
}

void LCD_InitializeDMA(void)
{
    // Configure DMA for data transfer
    LCD_DMA_Handle_DataTransfer.Instance                 = DMA1_Stream0;
    LCD_DMA_Handle_DataTransfer.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_DataTransfer.Init.Request             = DMA_REQUEST_TIM8_CH2;
    LCD_DMA_Handle_DataTransfer.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_DataTransfer.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_DataTransfer.Init.MemInc              = DMA_MINC_ENABLE;
    LCD_DMA_Handle_DataTransfer.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_DataTransfer.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_DataTransfer.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_DataTransfer.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    LCD_DMA_Handle_DataTransfer.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    LCD_DMA_Handle_DataTransfer.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_DataTransfer.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_DataTransfer.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_DataTransfer);

    // Link DMA with data transfer timer
    __HAL_LINKDMA(&LCD_TIM_Handle_DataTransfer, hdma[TIM_DMA_ID_CC2], LCD_DMA_Handle_DataTransfer);

    // Enable interrupt handler
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
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
    LCD_PORT_DATA->ODR = addr;
    LCD_RST_RS;
    LCD_RST_WR;
    LCD_SET_WR;
    LCD_SET_RS;
}

// Write data
void LCD_WriteData(uint16_t data)
{
    LCD_PORT_DATA->ODR = data;
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
    LCD_PORT_DATA->MODER = GPIO_INPUT_MODE;
    LCD_RST_RD;
    for (uint16_t i = 0; i < 42; i++);
    uint16_t data = LCD_PORT_DATA->IDR;
    LCD_SET_RD;
    for (uint16_t i = 0; i < 10; i++);
    LCD_PORT_DATA->MODER = GPIO_OUTPUT_MODE;
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
        LCD_PORT_DATA->ODR = color;
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

    LCD_PORT_DATA->ODR = color;
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
        LCD_PORT_DATA->ODR = color;
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
        LCD_PORT_DATA->ODR = GBC_GPU_FrameBuffer[i].Color;
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

void LCD_DrawGBCFrameBuffer(void)
{
    // ToDo: Move to menu so it's only called on change
    // Set Draw Area to the middle 160x144px for non scaled display
    LCD_SetDrawArea(80, 48, GBC_GPU_FRAME_SIZE_X, GBC_GPU_FRAME_SIZE_Y);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);

    // Activate alternate function for WR pin
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = LCD_PIN_WR;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = LCD_ALT_WR;
    HAL_GPIO_Init(LCD_PORT_WR, &GPIO_InitObject);

    __HAL_TIM_DISABLE_DMA(&LCD_TIM_Handle_DataTransfer, TIM_DMA_CC2);

    // Enable transfer complete IT
    ((DMA_Stream_TypeDef*)LCD_DMA_Handle_DataTransfer.Instance)->CR |= DMA_IT_TC;

    // Start DMA transfer
    HAL_DMA_Start(&LCD_DMA_Handle_DataTransfer, (uint32_t)&GBC_GPU_FrameBuffer, (uint32_t)&LCD_PORT_DATA->ODR, GBC_GPU_FRAME_SIZE);

    // Enable the TIM update DMA request
    __HAL_TIM_ENABLE_DMA(&LCD_TIM_Handle_DataTransfer, TIM_DMA_CC2);

    // Reset counter and start at 3/4 period, so the first WR toggle is skipped
    LCD_TIM_DATA->CNT = 30;

    // Start timer
    LCD_TIM_DATA->CCER &= ~((TIM_CCER_CC1E << LCD_TIM_CHANNEL_DATA) | (TIM_CCER_CC1E << LCD_TIM_CHANNEL_WR));
    LCD_TIM_DATA->CCER |= (uint32_t)((TIM_CCx_ENABLE << LCD_TIM_CHANNEL_DATA) | (TIM_CCx_ENABLE << LCD_TIM_CHANNEL_WR));

    if (IS_TIM_BREAK_INSTANCE(LCD_TIM_Handle_DataTransfer.Instance) != RESET)  
    {
        // Enable the main output
        __HAL_TIM_MOE_ENABLE(&LCD_TIM_Handle_DataTransfer);
    }

    // Enable timer
    __HAL_TIM_ENABLE(&LCD_TIM_Handle_DataTransfer);
}

void LCD_DrawGBCFrameBufferScaled(void)
{
    // ToDo: Move to menu so it's only called on change
    // Make sure Draw Area is correct in case of switching to scaled after using the not-scaled method
    // LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);
    for (int y = 0, line = 0, timesLineDrawn = 0, linesDrawn = 0; y < LCD_DISPLAY_SIZE_Y; y++)
    {
        for (int x = 0; x < GBC_GPU_FRAME_SIZE_X; x++)
        {
            LCD_PORT_DATA->ODR = GBC_GPU_FrameBuffer[line + x].Color;
            LCD_RST_WR;
            LCD_SET_WR;

            // Draw every pixel in a line twice to achieve a width of 320 pixel
            LCD_RST_WR;
            LCD_SET_WR;
        }
        timesLineDrawn++;
        linesDrawn++;

        // If a line has been drawn twice or if it's the fifth line draw the next line (twice)
        if (timesLineDrawn == 2 || linesDrawn == 5) { line += 160; timesLineDrawn = 0; }
        if (linesDrawn == 5) linesDrawn = 0;
    }
    LCD_SET_CS;
}

void LCD_DrawGBCFrameBufferScaledDMA(void)
{
    // Configure WR pin
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = LCD_PIN_WR;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = LCD_ALT_WR;
    HAL_GPIO_Init(LCD_PORT_WR, &GPIO_InitObject);

    
}

// Copy from stm32h7xx_hal_dma.c
typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

void DMA1_Stream0_IRQHandler(void)
{
    LCD_SET_CS;

    // Reset counter to avoid another WR toggle during stopping
    LCD_TIM_DATA->CNT = 0;

    // Disable timer
    LCD_TIM_Handle_DataTransfer.Instance->CR1 &= ~(TIM_CR1_CEN);

    // Stop timer
    LCD_TIM_DATA->CCER &= ~((TIM_CCER_CC1E << LCD_TIM_CHANNEL_DATA) | (TIM_CCER_CC1E << LCD_TIM_CHANNEL_WR));
    LCD_TIM_DATA->CCER |= (uint32_t)((TIM_CCx_DISABLE << LCD_TIM_CHANNEL_DATA) | (TIM_CCx_DISABLE << LCD_TIM_CHANNEL_WR));

    if (IS_TIM_BREAK_INSTANCE(LCD_TIM_Handle_DataTransfer.Instance) != RESET)  
    {
        // Disable the main output
        __HAL_TIM_MOE_DISABLE(&LCD_TIM_Handle_DataTransfer);
    }

    // Disable alternate function for WR pin
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitObject.Pin       = LCD_PIN_WR;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = 0;
    HAL_GPIO_Init(LCD_PORT_WR, &GPIO_InitObject);

    HAL_DMA_IRQHandler(&LCD_DMA_Handle_DataTransfer);
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
