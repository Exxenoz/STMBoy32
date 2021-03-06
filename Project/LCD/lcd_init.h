#ifndef LCD_INIT_H
#define LCD_INIT_H

#include "common.h"


extern TIM_HandleTypeDef  LCD_TIM_Handle_PixelTransferTiming;
extern TIM_HandleTypeDef  LCD_TIM_Handle_Backlight;

extern DMA_HandleTypeDef  LCD_DMA_Handle_WR_Set;
extern DMA_HandleTypeDef  LCD_DMA_Handle_WR_Rst;
extern DMA_HandleTypeDef  LCD_DMA_Handle_Data;



void LCD_Initialize(void);

#endif //LCD_INIT_H
