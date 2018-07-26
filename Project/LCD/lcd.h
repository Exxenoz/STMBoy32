#ifndef LCD_H
#define LCD_H

#include "gbc_gpu.h"

#include "lcd_init.h"
#include "lcd_types.h"
#include "lcd_config.h"
#include "lcd_regdef.h"


extern uint16_t*  LCD_DMA_TransferFrameBuffer;
extern uint32_t   LCD_DMA_TransferFrameBufferSize;
extern uint32_t   LCD_DMA_TransferFrameBufferOffset;

extern volatile LCD_DMA_TransferCompleteFlags_t LCD_DMA_TransferCompleteFlags;



/******************************************************************************/
/*                          LCD Low-Level Functions                           */
/******************************************************************************/

void LCD_WriteAddr(uint16_t addr);
void LCD_WriteCommand(uint16_t addr);
void LCD_WriteCommandWithData(uint16_t addr, uint16_t data);
void LCD_WriteCommandWithParameters(uint16_t addr, uint16_t parameters[], long length);


/******************************************************************************/
/*                        LCD Configuration Functions                         */
/******************************************************************************/

void LCD_DimBacklight(long percent);
void LCD_SetDrawArea(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void LCD_SetDrawBehaviour(bool writeBotToTop, bool writeRightToLeft, bool rowColExchange, bool refreshBotToTop, bool refreshRightToLeft, bool bgr);
void LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO_t divRatio, LCD_FRAME_RATE_t frameRate);


/******************************************************************************/
/*                          LCD Framebuffer Functions                         */
/******************************************************************************/

void LCD_StartFrameBufferTransfer(uint16_t* frameBuffer, uint32_t frameBufferOffset, uint16_t frameBufferLength);
void LCD_DrawGBCFrameBufferScaled(void);
void LCD_DrawGBCFrameBuffer(void);
void LCD_DrawFrameBuffer(uint16_t* frameBuffer, uint32_t frameBufferLength, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);

#endif //LCD_H
