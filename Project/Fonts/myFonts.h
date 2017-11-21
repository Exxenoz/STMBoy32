#ifndef FONTS_H
#define FONTS_H

#include "common.h"

typedef struct
{
    uint16_t FontHeight;        // Height the font (in pixel)
    uint16_t FontWidth;         // Width the font (in pixel)
    const uint16_t *fontData;   // Pointer to the font data
}
Fonts_FontDef_t;

extern Fonts_FontDef_t Fonts_STMFont_16x24;
extern Fonts_FontDef_t Fonts_STMFont_12x12;


void Fonts_InitializeSTMFonts(void);

#endif
