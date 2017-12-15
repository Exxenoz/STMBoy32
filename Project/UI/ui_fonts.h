#ifndef FONTS_H
#define FONTS_H

#include "common.h"

typedef struct
{
    bool LettersMirrored;
    uint16_t FontHeight;        // Height of the font (in pixel)
    uint16_t FontWidth;         // Width of the font (in pixel)
    const uint16_t *FontData;   // Pointer to the font data
}
Fonts_FontDef_16_t;

typedef struct
{
    uint16_t SymbolHeight;        // Height of the symbol (in pixel)
    uint16_t SymbolWidth;         // Width of the symbol (in pixel)
    const uint32_t *SymbolData;   // Pointer to the symbol data
}
Fonts_SymbolDef_32_t;

extern Fonts_FontDef_16_t Fonts_STMFont_16x24;
extern Fonts_FontDef_16_t Fonts_STMFont_12x12;

extern Fonts_SymbolDef_32_t Fonts_Star_32x22;


void Fonts_InitializeSTMFonts(void);
int  Fonts_GetStringLength(const char s[], uint16_t textSpacing, Fonts_FontDef_16_t *font);
int  Fonts_GetStringLengthWithoutSuffix(const char s[], uint16_t textSpacing, Fonts_FontDef_16_t *font);

#endif
