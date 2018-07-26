#ifndef LCD_TYPES_H
#define LCD_TYPES_H

#include "common.h"


#define LCD_MAX_TEXT_LENGTH 20


typedef enum
{
    LCD_DMA_TRANSFER_NOT_COMPLETE         = 0,
    LCD_DMA_TRANSFER_COMPLETE_FLAG_DATA   = 1,
    LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_SET = 2,
    LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_RST = 4,
    LCD_DMA_TRANSFER_COMPLETE             = LCD_DMA_TRANSFER_COMPLETE_FLAG_DATA | LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_SET | LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_RST,
}
LCD_DMA_TransferCompleteFlags_t;

typedef enum
{
    LCD_FRAME_RATE_DIVISION_RATIO1 = 0x00,
    LCD_FRAME_RATE_DIVISION_RATIO2 = 0x01,
}
LCD_FRAME_RATE_DIVISION_RATIO_t;

typedef enum
{
    LCD_FRAME_RATE_61HZ  = 0x1F,
    LCD_FRAME_RATE_83HZ  = 0x17,
    LCD_FRAME_RATE_95HZ  = 0x14,
    LCD_FRAME_RATE_100HZ = 0x13,
    LCD_FRAME_RATE_106HZ = 0x12,
    LCD_FRAME_RATE_112HZ = 0x11,
    LCD_FRAME_RATE_119HZ = 0x10,
}
LCD_FRAME_RATE_t;

typedef enum
{
    LCD_HORIZONTAL,
    LCD_VERTICAL,
}
LCD_Orientation_t;

typedef struct
{
    uint16_t Width;
    uint16_t Color;
}
LCD_Border_t;

typedef struct
{
    uint16_t     Length;
    uint16_t     Height;
    uint16_t     Color;
    LCD_Border_t Border;
}
LCD_Brick_t;

typedef struct
{
    int Upper;
    int Right;
    int Lower;
    int Left;
}
LCD_Padding_t;

typedef struct
{
    char          Characters[LCD_MAX_TEXT_LENGTH];
    uint16_t      Spacing;
    uint16_t      Height;
    uint16_t      Color;
    LCD_Padding_t Padding;
    LCD_Border_t  Border;
}
LCD_TextDef_t;

#pragma pack(1)
typedef union LCD_Pixel_u
{
    uint16_t Color;

    #pragma pack(1)
    struct
    {
        unsigned int Blue  : 5; // LSB
        unsigned int Green : 6;
        unsigned int Red   : 5; // MSB
    };
}
LCD_Pixel_t;

#endif //LCD_TYPES_H
