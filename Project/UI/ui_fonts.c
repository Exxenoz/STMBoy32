#include "ui_fonts.h"
#include "fonts.h"

Fonts_FontDef_t Fonts_STMFont_16x24;
Fonts_FontDef_t Fonts_STMFont_12x12;

void Fonts_InitializeSTMFonts(void)
{
    Fonts_STMFont_16x24.LettersMirrored = true;
    Fonts_STMFont_16x24.FontHeight = Font16x24.Height;
    Fonts_STMFont_16x24.FontWidth = Font16x24.Width;
    Fonts_STMFont_16x24.FontData = Font16x24.table;

    Fonts_STMFont_12x12.LettersMirrored = true;
    Fonts_STMFont_12x12.FontHeight = Font12x12.Height;
    Fonts_STMFont_12x12.FontWidth = Font12x12.Width;
    Fonts_STMFont_12x12.FontData = Font12x12.table;
}

// Returns the length of a given text represented by a certain font (in pixel)
int  Fonts_GetStringLength(const char text[], uint16_t textSpacing, Fonts_FontDef_t *font)
{
    int length = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        length += font->FontWidth + textSpacing;
    }

    return length;
}
