#include "lcd_event_handlers.h"

bool LCD_READY_FLAG;



void LCD_OnTransferComplete(LCD_DMA_TransferCompleteFlags_t completeFlag)
{
    LCD_DMA_TransferCompleteFlags |= completeFlag;

    if (LCD_DMA_TransferCompleteFlags != LCD_DMA_TRANSFER_COMPLETE)
    {
        return;
    }

    // Calculate next transfer frame buffer offset
    LCD_DMA_TransferFrameBufferOffset += LCD_DMA_MAX_SIZE_PER_TRANSFER;

    // Disable timer
    __HAL_TIM_DISABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Disable capture/compare
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_DATA_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_WR_SET_TIM_CHANNEL);
    LCD_TIM_Handle_PixelTransferTiming.Instance->CCER &= ~(uint32_t)(TIM_CCx_ENABLE << LCD_WR_RST_TIM_CHANNEL);

    // Disable main output
    __HAL_TIM_MOE_DISABLE(&LCD_TIM_Handle_PixelTransferTiming);

    // Reset timer state
    LCD_TIM_Handle_PixelTransferTiming.State = HAL_TIM_STATE_READY;

    LCD_SET_WR;

    if (LCD_DMA_TransferFrameBufferOffset >= LCD_DMA_TransferFrameBufferSize)
    {
        LCD_SET_CS;

        LCD_DMA_TransferFrameBufferOffset = 0;
        LCD_DMA_TransferFrameBufferSize = 0;
        LCD_DMA_TransferFrameBuffer = NULL;
    }
    else
    {
        // Reset transfer complete flags
        LCD_DMA_TransferCompleteFlags = LCD_DMA_TRANSFER_NOT_COMPLETE;

        // Calculate remaining transfer size
        uint32_t remainingTransferSize = LCD_DMA_TransferFrameBufferSize - LCD_DMA_TransferFrameBufferOffset;

   			// Start the next DMA transfer; Check if remaining transfer size is greater than max supported size for a single DMA transfer
        LCD_StartFrameBufferTransfer((uint16_t*)LCD_DMA_TransferFrameBuffer, LCD_DMA_TransferFrameBufferOffset, (remainingTransferSize > LCD_DMA_MAX_SIZE_PER_TRANSFER) ? LCD_DMA_MAX_SIZE_PER_TRANSFER : remainingTransferSize);
    }
}

void DMA1_Stream0_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_Data);

    LCD_OnTransferComplete(LCD_DMA_TRANSFER_COMPLETE_FLAG_DATA);
}

void DMA1_Stream1_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_WR_Set);

    LCD_OnTransferComplete(LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_SET);
}

void DMA1_Stream2_IRQHandler(void)
{
    // Handle DMA interrupt
    HAL_DMA_IRQHandler(&LCD_DMA_Handle_WR_Rst);

    LCD_OnTransferComplete(LCD_DMA_TRANSFER_COMPLETE_FLAG_WR_RST);
}

// ToDo: Disable when not needed
void EXTI0_IRQHandler(void)
{ 
    if (__HAL_GPIO_EXTI_GET_IT(INPUT_FRAME_PIN) != RESET) 
    {
        if (INPUT_FRAME_PORT->IDR & INPUT_FRAME_PIN)
        {
            LCD_SET_READY_FLAG;
        }
        else
        {
            LCD_RST_READY_FLAG;
        }

        __HAL_GPIO_EXTI_CLEAR_IT(INPUT_FRAME_PIN);        
    }
}
