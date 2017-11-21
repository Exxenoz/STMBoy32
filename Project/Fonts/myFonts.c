#include "myFonts.h"
#include "fonts.h"

Fonts_FontDef_t Fonts_STMFont_16x24;
Fonts_FontDef_t Fonts_STMFont_12x12;

void Fonts_InitializeSTMFonts(void)
{
    Fonts_STMFont_16x24.FontHeight = Font16x24.Height;
    Fonts_STMFont_16x24.FontWidth = Font16x24.Width;
    Fonts_STMFont_16x24.fontData = Font16x24.table;
    
    Fonts_STMFont_12x12.FontHeight = Font12x12.Height;
    Fonts_STMFont_12x12.FontWidth = Font12x12.Width;
    Fonts_STMFont_12x12.fontData = Font12x12.table;
}
