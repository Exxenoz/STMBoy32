#include "lcd_init.h"

#include "lcd.h"
#include "lcd_config.h"
#include "lcd_regdef.h"
#include "lcd_drawing.h"
#include "input_config.h"


uint16_t LCD_REG_VCOM_CONTROL_DATA[2] =
{
    LCD_REG_VCOM_CONTROL_PARAM1,
    LCD_REG_VCOM_CONTROL_PARAM2,
};

uint16_t LCD_REG_DISPLAY_FUNCTION_CONTROL_DATA[4] =
{
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM1,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM2,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM3,
    LCD_REG_DISPLAY_FUNCTION_CONTROL_PARAM4,
};

uint16_t LCD_REG_POSITIVE_GAMMA_CORRECTION_DATA[15] =
{
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM1,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM2,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM3,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM4,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM5,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM6,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM7,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM8,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM9,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM10,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM11,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM12,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM13,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM14,
    LCD_REG_POSITIVE_GAMMA_CORRECTION_PARAM15,
};

uint16_t LCD_REG_NEGATIVE_GAMMA_CORRECTION_DATA[15] =
{
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM1,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM2,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM3,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM4,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM5,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM6,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM7,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM8,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM9,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM10,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM11,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM12,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM13,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM14,
    LCD_REG_NEGATIVE_GAMMA_CORRECTION_PARAM15,
};


TIM_OC_InitTypeDef OC_Config_Data;
TIM_OC_InitTypeDef OC_Config_WR_Rst;
TIM_OC_InitTypeDef OC_Config_WR_Set;

TIM_HandleTypeDef  LCD_TIM_Handle_Backlight;
TIM_HandleTypeDef  LCD_TIM_Handle_PixelTransferTiming;

DMA_HandleTypeDef  LCD_DMA_Handle_Data;
DMA_HandleTypeDef  LCD_DMA_Handle_WR_Set;
DMA_HandleTypeDef  LCD_DMA_Handle_WR_Rst;



void LCD_InitializePins(void)
{
    GPIO_InitTypeDef GPIO_InitObject;

    #define INITIALIZE_OUTPUT_PIN(PORT, PIN)       \
    GPIO_InitObject.Mode  = GPIO_MODE_OUTPUT_PP;    \
    GPIO_InitObject.Pin   = PIN;                     \
    GPIO_InitObject.Pull  = GPIO_NOPULL;              \
    GPIO_InitObject.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
    HAL_GPIO_Init(PORT, &GPIO_InitObject);              \

    INITIALIZE_OUTPUT_PIN(LCD_RESET_PORT, LCD_RESET_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RS_PORT,    LCD_RS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_CS_PORT,    LCD_CS_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_RD_PORT,    LCD_RD_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_WR_PORT,    LCD_WR_PIN);
    INITIALIZE_OUTPUT_PIN(LCD_DATA_PORT,  GPIO_PIN_All);

    LCD_SET_RESET;
    LCD_SET_RS;
    LCD_SET_CS;
    LCD_SET_RD;
    LCD_SET_WR;
}

void LCD_InitializeBacklight()
{
    GPIO_InitTypeDef GPIO_InitObject;
    GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitObject.Pin       = LCD_PIN_BACKLIGHT;
    GPIO_InitObject.Pull      = GPIO_NOPULL;
    GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitObject.Alternate = LCD_ALT_BACKLIGHT;
    HAL_GPIO_Init(LCD_PORT_BACKLIGHT, &GPIO_InitObject);

    LCD_TIM_Handle_Backlight.Instance               = LCD_TIM_BACKLIGHT;
    LCD_TIM_Handle_Backlight.Init.Prescaler         = 39;
    LCD_TIM_Handle_Backlight.Init.CounterMode       = TIM_COUNTERMODE_UP;
    LCD_TIM_Handle_Backlight.Init.Period            = 99;
    LCD_TIM_Handle_Backlight.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    LCD_TIM_Handle_Backlight.Init.RepetitionCounter = 0;

    static TIM_OC_InitTypeDef LCD_TIM_OC_Init_Backlight;
    LCD_TIM_OC_Init_Backlight.Pulse        = 80; 
    LCD_TIM_OC_Init_Backlight.OCMode       = TIM_OCMODE_PWM1;
    LCD_TIM_OC_Init_Backlight.OCFastMode   = TIM_OCFAST_DISABLE;
    LCD_TIM_OC_Init_Backlight.OCPolarity   = TIM_OCPOLARITY_HIGH;
    LCD_TIM_OC_Init_Backlight.OCNPolarity  = TIM_OCNPOLARITY_LOW;
    LCD_TIM_OC_Init_Backlight.OCIdleState  = TIM_OCIDLESTATE_RESET;
    LCD_TIM_OC_Init_Backlight.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_Init(&LCD_TIM_Handle_Backlight);
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_Backlight, &LCD_TIM_OC_Init_Backlight, LCD_TIM_BACKLIGHT_CHANNEL);
    HAL_TIM_PWM_Start(&LCD_TIM_Handle_Backlight, LCD_TIM_BACKLIGHT_CHANNEL);
}

void LCD_InitializeDMATimer(void)
{
    // DEBUG: Configure alternate function for WR pin
    //GPIO_InitTypeDef GPIO_InitObject;
    //GPIO_InitObject.Mode      = GPIO_MODE_AF_PP;
    //GPIO_InitObject.Pin       = GPIO_PIN_6;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = GPIO_AF3_TIM8;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitObject);

    // DEBUG: Configure data transfer timer signal output pin
    //GPIO_InitTypeDef GPIO_InitObject;
    //GPIO_InitObject.Mode      = GPIO_MODE_OUTPUT_PP;
    //GPIO_InitObject.Pin       = GPIO_PIN_7;
    //GPIO_InitObject.Pull      = GPIO_NOPULL;
    //GPIO_InitObject.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    //GPIO_InitObject.Alternate = 0;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitObject);

    // Configure pixel transfer timing
    LCD_TIM_Handle_PixelTransferTiming.Instance                = LCD_DATA_WR_TIM;
    LCD_TIM_Handle_PixelTransferTiming.Init.Period             = 40; // 200 MHz / 40 = ~5 MHz, fast enough to send 76800 pixels in about 15-17ms
    LCD_TIM_Handle_PixelTransferTiming.Init.Prescaler          = 0;
    LCD_TIM_Handle_PixelTransferTiming.Init.CounterMode        = TIM_COUNTERMODE_UP;
    LCD_TIM_Handle_PixelTransferTiming.Init.ClockDivision      = TIM_CLOCKDIVISION_DIV1;
    LCD_TIM_Handle_PixelTransferTiming.Init.RepetitionCounter  = 0;
    LCD_TIM_Handle_PixelTransferTiming.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&LCD_TIM_Handle_PixelTransferTiming);

    // Configure data pulse width modulation
    OC_Config_Data.Pulse        = 1;
    OC_Config_Data.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_Data.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_Data.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_Data.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_Data.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_Data.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_Data, LCD_DATA_TIM_CHANNEL);

    // Configure WR reset pulse width modulation
    OC_Config_WR_Rst.Pulse        = 1;
    OC_Config_WR_Rst.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_WR_Rst.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_WR_Rst.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_WR_Rst.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_WR_Rst.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_WR_Rst.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_WR_Rst, LCD_WR_RST_TIM_CHANNEL);

    // Configure WR set pulse width modulation
    OC_Config_WR_Set.Pulse        = 1;
    OC_Config_WR_Set.OCMode       = TIM_OCMODE_PWM1;
    OC_Config_WR_Set.OCFastMode   = TIM_OCFAST_DISABLE;
    OC_Config_WR_Set.OCPolarity   = TIM_OCPOLARITY_LOW;
    OC_Config_WR_Set.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    OC_Config_WR_Set.OCIdleState  = TIM_OCIDLESTATE_RESET;
    OC_Config_WR_Set.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&LCD_TIM_Handle_PixelTransferTiming, &OC_Config_WR_Set, LCD_WR_SET_TIM_CHANNEL);
}

void LCD_InitializeDMA(void)
{
    // Configure DMA for data
    LCD_DMA_Handle_Data.Instance                 = DMA1_Stream0;
    LCD_DMA_Handle_Data.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_Data.Init.Request             = DMA_REQUEST_TIM8_CH2;
    LCD_DMA_Handle_Data.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_Data.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_Data.Init.MemInc              = DMA_MINC_ENABLE;
    LCD_DMA_Handle_Data.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_Data.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_Data.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_Data.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    LCD_DMA_Handle_Data.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    LCD_DMA_Handle_Data.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_Data.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_Data.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_Data);

    // Configure DMA for WR set
    LCD_DMA_Handle_WR_Set.Instance                 = DMA1_Stream1;
    LCD_DMA_Handle_WR_Set.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_WR_Set.Init.Request             = DMA_REQUEST_TIM8_CH1;
    LCD_DMA_Handle_WR_Set.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_WR_Set.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_WR_Set.Init.MemInc              = DMA_MINC_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_WR_Set.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Set.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Set.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_WR_Set.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_WR_Set.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_WR_Set);

    // Configure DMA for WR rst
    LCD_DMA_Handle_WR_Rst.Instance                 = DMA1_Stream2;
    LCD_DMA_Handle_WR_Rst.State                    = HAL_DMA_STATE_RESET;
    LCD_DMA_Handle_WR_Rst.Init.Request             = DMA_REQUEST_TIM8_CH3;
    LCD_DMA_Handle_WR_Rst.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    LCD_DMA_Handle_WR_Rst.Init.Mode                = DMA_NORMAL;
    LCD_DMA_Handle_WR_Rst.Init.MemInc              = DMA_MINC_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphInc           = DMA_PINC_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.MemBurst            = DMA_MBURST_SINGLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    LCD_DMA_Handle_WR_Rst.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Rst.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    LCD_DMA_Handle_WR_Rst.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    LCD_DMA_Handle_WR_Rst.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    LCD_DMA_Handle_WR_Rst.Init.Priority            = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&LCD_DMA_Handle_WR_Rst);

    // Link DMA with pixel transfer timing
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC2], LCD_DMA_Handle_Data);
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC1], LCD_DMA_Handle_WR_Set);
    __HAL_LINKDMA(&LCD_TIM_Handle_PixelTransferTiming, hdma[TIM_DMA_ID_CC3], LCD_DMA_Handle_WR_Rst);

    // Enable interrupt handler
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);

    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

void LCD_InitializeInterrupt(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin  = INPUT_FRAME_PIN;
    HAL_GPIO_Init(INPUT_FRAME_PORT, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void LCD_Initialize(void)
{
		// Reminder: Porching increases available time to write from RAM to GRAM

    LCD_InitializePins();
    LCD_InitializeBacklight();
    LCD_InitializeInterrupt();
    LCD_InitializeDMATimer();
    LCD_InitializeDMA();
    

    LCD_WriteCommand(LCD_REG_SOFTWARE_RESET);
    HAL_Delay(120); 													// Wait ~120ms (ILI9341 Datasheet p. 90)
    LCD_WriteCommand(LCD_REG_DISPLAY_OFF);


    LCD_WriteCommandWithData(LCD_REG_POWER_CONTROL_1, LCD_REG_POWER_CONTROL1_DATA);
    LCD_WriteCommandWithData(LCD_REG_POWER_CONTROL_2, LCD_REG_POWER_CONTROL2_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_VCOM_CONTROL_1, LCD_REG_VCOM_CONTROL_DATA, 2);
    LCD_WriteCommandWithData(LCD_REG_VCOM_CONTROL_2, LCD_REG_VCOM_CONTROL2_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_DISPLAY_FUNCTION_CONTROL, LCD_REG_DISPLAY_FUNCTION_CONTROL_DATA, 4);
    LCD_WriteCommandWithData(LCD_REG_PIXEL_FORMAT_SET, LCD_REG_PIXEL_FORMAT_DATA);
    LCD_WriteCommandWithData(LCD_REG_ENTRY_MODE_SET, LCD_REG_ENTRY_MODE_DATA);

    LCD_WriteCommandWithParameters(LCD_REG_POSITIVE_GAMMA_CORRECTION, LCD_REG_POSITIVE_GAMMA_CORRECTION_DATA, 15);
    LCD_WriteCommandWithParameters(LCD_REG_NEGATIVE_GAMMA_CORRECTION, LCD_REG_NEGATIVE_GAMMA_CORRECTION_DATA, 15);

    LCD_SetFrameRate(LCD_FRAME_RATE_DIVISION_RATIO1, LCD_FRAME_RATE_61HZ);
    LCD_SetDrawBehaviour(false, false, true, true, false, true);

    LCD_WriteCommandWithData(LCD_REG_TEARING_EFFECT_LINE_ON, 0);
    LCD_SetDrawArea(0, 0, LCD_DISPLAY_SIZE_X, LCD_DISPLAY_SIZE_Y);

    LCD_WriteCommand(LCD_REG_SLEEP_OUT);
    LCD_WriteCommand(LCD_REG_DISPLAY_ON);
}
