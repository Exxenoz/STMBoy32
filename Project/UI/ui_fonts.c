#include "ui_fonts.h"
#include "fonts.h"

const uint32_t star_data_32x22[22] = {
    0x00008000, 0x0001C000, 0x0001C000, 0x0003E000,
    0x0003E000, 0x0007F000, 0x0007F000, 0x000FF800,
    0x7FFFFFFF, 0x1FFFFFFC, 0x0FFFFFF8, 0x03FFFFE0,
    0x00FFFF80, 0x007FFF00, 0x00FFFF80, 0x00FFFF80,
    0x01FFFFC0, 0x01FF7FC0, 0x03FC1F70, 0x07E003F0,
    0x078000F0, 0x0E000038,
};

Fonts_SymbolDef_32_t Fonts_Star_32x22 = { 22, 32, star_data_32x22 };

Fonts_FontDef_16_t Fonts_STMFont_16x24;
Fonts_FontDef_16_t Fonts_STMFont_12x12;


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
int Fonts_GetStringLength(const char s[], uint16_t textSpacing, Fonts_FontDef_16_t *font)
{
    int length = 0;

    for (int i = 0; s[i] != '\0'; i++)
    {
        length += font->FontWidth + textSpacing;
    }

    return length;
}

int Fonts_GetStringLengthWithoutSuffix(const char s[], uint16_t textSpacing, Fonts_FontDef_16_t *font)
{
    int length = 0;

    for (int i = 0; s[i] != '\0' && (s[i] != '.' || s[i+1] != 'g' || s[i+2] != 'b'); i++)
    {
        length += font->FontWidth + textSpacing;
    }

    return length;
}
