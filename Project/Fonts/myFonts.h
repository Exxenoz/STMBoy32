#ifndef FONTS_H
#define FONTS_H

#include "common.h"

typedef struct
{
    int FontHeight;             // Height of all characters from the font (in pixel)
    const int      *CharWidth;  // Pointer to width of each character (in pixel)
    const uint16_t *FontData;   // Pointer to the font data
}
Fonts_FontDef_t;

extern Fonts_FontDef_t Fonts_MyFont; 

// Returns the length of a given string represented by a certain font (in pixel)
int GetStringLength(char *s, Fonts_FontDef_t *font);

#endif
