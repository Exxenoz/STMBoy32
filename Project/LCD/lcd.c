#include "lcd.h"
#include "lcd_regdef.h"
#include "gbc_gpu.h"

bool LCD_READY_FLAG;

void LCD_InitializePins(void)
{
    RCC_AHB1PeriphClockCmd(LCD_RESET_BUS,   ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_RS_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_CS_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_RD_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_WR_BUS,      ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_DATA_BUS,    ENABLE);
    RCC_AHB1PeriphClockCmd(LCD_BACKLIT_BUS, ENABLE);

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
    RCC_APB2PeriphClockCmd(LCD_TIM_BUS, ENABLE);

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
    // SYSCFG APB clock must be enabled to get write access to SYSCFG_EXTICRx registers
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

    TIM_SetCompare4(LCD_TIM, 2.81f * percent);
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
    LCD_InitializeTimer();
    LCD_InitializeInterrupt();
    

    LCD_WriteCommand(LCD_REG_SOFTWARE_RESET);
    for (long i = 0; i < 2000000; i++);         // Wait ~120ms (ILI9341 Datasheet p. 90)
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

void LCD_DrawGBCFrameBuffer(void)
{
    // ToDo: Move to menu so it's only called on change
    // Set Draw Area to the middle 160x144px for non scaled display
    // LCD_SetDrawArea(80, 48, GBC_GPU_FRAME_SIZE_X, GBC_GPU_FRAME_SIZE_Y);

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
            LCD_DATA_PORT->ODR = GBC_GPU_FrameBuffer[line + x].Color;
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

// ToDo: Disable when not needed
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
