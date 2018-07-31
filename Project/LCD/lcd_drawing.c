#include "lcd_drawing.h"



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

// This function doesn't use DrawPixel in order to avoid unneccessary function calls.
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
void LCD_DrawText(uint16_t x0, uint16_t y0, uint16_t bgColor, LCD_TextDef_t *text, const Fonts_FontDef_16_t *p_font)
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

void LCD_DrawSymbol(uint16_t x0, uint16_t y0, uint16_t color, const Fonts_SymbolDef_32_t *p_symbol)
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
