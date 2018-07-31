#ifndef FONTS_H
#define FONTS_H

#include "common.h"


#pragma pack(1)
typedef struct
{
    const bool     LettersMirrored;
    const uint16_t FontHeight;        // Height of the font (in pixel)
    const uint16_t FontWidth;         // Width of the font (in pixel)
    const uint16_t *FontData;         // Pointer to the font data
}
Fonts_FontDef_16_t;

#pragma pack(1)
typedef struct
{
    uint16_t SymbolHeight;            // Height of the symbol (in pixel)
    uint16_t SymbolWidth;             // Width of the symbol (in pixel)
    const uint32_t *SymbolData;       // Pointer to the symbol data
}
Fonts_SymbolDef_32_t;


extern const Fonts_FontDef_16_t   Fonts_STMFont_16x24;
extern const Fonts_SymbolDef_32_t Fonts_Star_32x22;



int Fonts_GetStringLength(const char s[], uint16_t textSpacing, const Fonts_FontDef_16_t *font);
int Fonts_GetStringLengthWithoutSuffix(const char s[], uint16_t textSpacing, const Fonts_FontDef_16_t *font);

#endif //FONTS_H
