#include "lcd.h"


LCD_Pixel_t LCD_FrameBuffer[LCD_DISPLAY_PIXELS];

uint16_t*   LCD_DMA_TransferFrameBuffer;
uint32_t    LCD_DMA_TransferFrameBufferSize;
uint32_t    LCD_DMA_TransferFrameBufferOffset;

volatile LCD_DMA_TransferCompleteFlags_t LCD_DMA_TransferCompleteFlags = LCD_DMA_TRANSFER_COMPLETE;



/******************************************************************************/
/*                          LCD Low-Level Functions                           */
/******************************************************************************/

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

	for (uint16_t i = 0; i < 42; i++); // ToDo: use Hal_delay

    uint16_t data = LCD_DATA_PORT->IDR;
    LCD_SET_RD;

    for (uint16_t i = 0; i < 10; i++); // ToDo: use Hal_delay

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


/******************************************************************************/
/*                        LCD Configuration Functions                         */
/******************************************************************************/

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

// Remove this function along with FrameRateControlData_t if frame rate doesn't need to be changed during runtime
void LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO_t divRatio, LCD_FRAME_RATE_t frameRate)
{
    FrameRateControlData_t frameRateControlData = {0};
    frameRateControlData.DivisionRatio = divRatio;
    frameRateControlData.FrameRate     = frameRate;
    LCD_WriteCommandWithParameters(LCD_REG_FRAME_RATE_CONTROL, frameRateControlData.Data, 2);
}

void LCD_SetDrawBehaviour(bool writeBotToTop, bool writeRightToLeft, bool rowColExchange, bool refreshBotToTop, bool refreshRightToLeft, bool bgr)
{
    MemoryAccessControlData_t memoryAccessControlData = {0};
    memoryAccessControlData.RowAddressOrder        = writeBotToTop;
    memoryAccessControlData.ColumnAddressOrder     = writeRightToLeft;
    memoryAccessControlData.RowColumnExchange      = rowColExchange;
    memoryAccessControlData.VerticalRefreshOrder   = refreshBotToTop;
    memoryAccessControlData.HorizontalRefreshOrder = refreshRightToLeft;
    memoryAccessControlData.RGBBGROrder            = bgr;

    LCD_WriteCommandWithData(LCD_REG_MEMORY_ACCESS_CONTROL, memoryAccessControlData.Data);
}


/******************************************************************************/
/*                          LCD Framebuffer Functions                         */
/******************************************************************************/

void LCD_StartFrameBufferTransfer(uint16_t* frameBuffer, uint32_t frameBufferOffset, uint16_t frameBufferLength)
{
    // Disable the TIM update DMA request
    __HAL_TIM_DISABLE_DMA(&LCD_TIM_Handle_PixelTransferTiming, TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3);

    DMA_Stream_TypeDef* h1 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_Data.Instance);
    DMA_Stream_TypeDef* h2 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_WR_Set.Instance);
    DMA_Stream_TypeDef* h3 = ((DMA_Stream_TypeDef *)LCD_DMA_Handle_WR_Rst.Instance);

    // Enable transfer complete IT
    h1->CR |= DMA_IT_TC;
    h2->CR |= DMA_IT_TC;
    h3->CR |= DMA_IT_TC;

    // Enable capture/compare
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_DATA_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_WR_SET_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER |= (uint32_t)(TIM_CCx_ENABLE << LCD_WR_RST_TIM_CHANNEL);

    // Reset TIM counter
    LCD_DATA_WR_TIM->CNT = 0;

    static const uint8_t WR_SetByte = 0xFF;
    static const uint8_t WR_RstByte = 0x00;

    // Start DMA transfer
    HAL_DMA_Start(&LCD_DMA_Handle_Data,   (uint32_t)&frameBuffer[frameBufferOffset], (uint32_t)(&LCD_DATA_PORT->ODR), frameBufferLength);
    HAL_DMA_Start(&LCD_DMA_Handle_WR_Set, (uint32_t)&WR_SetByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength + 1);
    HAL_DMA_Start(&LCD_DMA_Handle_WR_Rst, (uint32_t)&WR_RstByte, (uint32_t)(&LCD_WR_PORT->ODR) + LCD_WR_PORT_ODR_BYTE, frameBufferLength);

    // Enable the TIM update DMA request
    __HAL_TIM_ENABLE_DMA(&LCD_TIM_Handle_PixelTransferTiming, TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3);

    // Enable main output
    __HAL_TIM_MOE_ENABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Enable timer
    __HAL_TIM_ENABLE(&LCD_TIM_Handle_PixelTransferTiming);
}

void LCD_DrawFrameBuffer(uint16_t* frameBuffer, uint32_t frameBufferLength, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
    // Abort any running DMA transfers
    HAL_DMA_Abort(&LCD_DMA_Handle_Data);
    HAL_DMA_Abort(&LCD_DMA_Handle_WR_Set);
    HAL_DMA_Abort(&LCD_DMA_Handle_WR_Rst);

    // ToDo: Move to menu so it's only called on change
    LCD_SetDrawArea(x0, y0, width, height);

    LCD_RST_CS;
    LCD_WriteAddr(LCD_REG_MEMORY_WRITE);

    // Set transfer frame buffer
    LCD_DMA_TransferFrameBuffer = frameBuffer;

    // Set transfer frame buffer size
    LCD_DMA_TransferFrameBufferSize = frameBufferLength;

    // Reset transfer complete flags
    LCD_DMA_TransferCompleteFlags = LCD_DMA_TRANSFER_NOT_COMPLETE;

    // Start DMA transfer; Check if frame buffer length is greater than max supported size for a single DMA transfer
    LCD_StartFrameBufferTransfer(LCD_DMA_TransferFrameBuffer, LCD_DMA_TransferFrameBufferOffset = 0, (frameBufferLength > LCD_DMA_MAX_SIZE_PER_TRANSFER) ? LCD_DMA_MAX_SIZE_PER_TRANSFER : frameBufferLength);
}

void LCD_DrawGBCFrameBuffer(void)
{
		// Set Draw Area to the middle 160x144px for non scaled display
    LCD_DrawFrameBuffer((uint16_t*)GBC_GPU_FrameBuffer, GBC_GPU_FRAME_SIZE, 80, 48, GBC_GPU_FRAME_SIZE_X, GBC_GPU_FRAME_SIZE_Y);
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

    LCD_DrawFrameBuffer((uint16_t*)LCD_FrameBuffer, LCD_DISPLAY_PIXELS, 0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);
}
