#include "cmod.h"
#include "cmod_init.h"
#include "cmod_access.h"


static void CMOD_HandleRead(void);
static void CMOD_HandleWrite(void);
static void CMOD_HandleNOP(void);

void (*CMOD_OperationTable[3])(void) =
{
    CMOD_HandleRead,
    CMOD_HandleWrite,
    CMOD_HandleNOP
};

CMOD_Action_t  CMOD_Action       = CMOD_NOACTION;
CMOD_Status_t  CMOD_Status       = CMOD_WAITING;
uint16_t       CMOD_Address      = 0x0000;
uint8_t        *CMOD_DataOut     = NULL;
uint8_t        *CMOD_DataIn      = NULL;
int            CMOD_BytesToRead  = 0;
int            CMOD_BytesRead    = 0;
int            CMOD_BytesToWrite = 0;
int            CMOD_BytesWritten = 0;



void CMOD_TurnON(void)
{
    CMOD_ENABLE_LLC();

    HAL_TIM_PWM_Start_IT(&CMOD_TimerHandle, CMOD_TIM_CHANNEL);
    HAL_NVIC_EnableIRQ(CMOD_TIM_NVIC_CHANNEL);

    CMOD_Status = CMOD_WAITING;
}

void CMOD_TurnOFF(void)
{
    CMOD_DISABLE_LLC();

    HAL_TIM_PWM_Stop_IT(&CMOD_TimerHandle, CMOD_TIM_CHANNEL);
    HAL_NVIC_DisableIRQ(CMOD_TIM_NVIC_CHANNEL);
    
    CMOD_Status = CMOD_TURNED_OFF;
}

CMOD_Status_t CMOD_GetStatus(void)
{
    return CMOD_Status; 
}


/******************************************************************************/
/*                          CMOD Read/write Functions                         */
/******************************************************************************/

void CMOD_ResetCartridge(void)
{
    if (CMOD_Status == CMOD_TURNED_OFF) return;


    CMOD_RST_RESET;
    HAL_Delay(1);                       // Wait 1ms (at least some ns).
    CMOD_SET_RESET;
}

void CMOD_ReadByte(uint16_t address, uint8_t *data)
{
    if (CMOD_Status == CMOD_TURNED_OFF) return;


    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_RD;                        // RD goes low for a read at 30ns (in GB)

    CMOD_Action      = CMOD_READ;
    CMOD_Address     = address;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = 1;
    CMOD_BytesRead   = 0;
    CMOD_Status      = CMOD_PROCESSING;

    CMOD_DATA_MODE_IN();
    CMOD_ENABLE_INTERRUPT();
}

void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    if (CMOD_Status == CMOD_TURNED_OFF) return;


    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_RD;                        // RD goes low for a read at 30ns (in GB)

    CMOD_Action      = CMOD_READ;
    CMOD_Address     = startingAddress;
    CMOD_DataIn      = data;
    CMOD_BytesToRead = bytes;
    CMOD_BytesRead   = 0;
    CMOD_Status      = CMOD_PROCESSING;

    CMOD_DATA_MODE_IN();
    CMOD_ENABLE_INTERRUPT();
}

void CMOD_WriteByte(uint16_t address, uint8_t *data)
{
    if (CMOD_Status == CMOD_TURNED_OFF) return;


    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_WR;                        // WR goes low for a write at ? (in GB)

    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = address;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = 1;
    CMOD_BytesWritten = 0;
    CMOD_Status       = CMOD_PROCESSING;

    CMOD_DATA_MODE_OUT();  
    CMOD_ENABLE_INTERRUPT();
}

void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data)
{
    if (CMOD_Status == CMOD_TURNED_OFF) return;


    while (CMOD_Status == CMOD_PROCESSING);

    CMOD_RST_WR;                        // WR goes low for a write at ? (in GB)

    CMOD_Action       = CMOD_WRITE;
    CMOD_Address      = startingAddress;
    CMOD_DataOut      = data;
    CMOD_BytesToWrite = bytes;
    CMOD_BytesWritten = 0;
    CMOD_Status       = CMOD_PROCESSING;

    CMOD_DATA_MODE_OUT();
    CMOD_ENABLE_INTERRUPT();
}


/******************************************************************************/
/*                           CMOD Handler Functions                           */
/******************************************************************************/

void CMOD_HandleRead(void)
{
    CMOD_SET_CS;                                            // CS rises at 0ns (after CLK Flank)

    if (CMOD_BytesRead != 0)                                // If a Byte was read, store it
    {                                                       // Gameboy stops driving Data Bus at 0ns -> Data ready
        CMOD_DataIn[CMOD_BytesRead -1] = CMOD_GET_DATA();   // Store the nt byte at CMOD_DataIn[n-1]
    }

    if (CMOD_BytesRead == CMOD_BytesToRead)                 // Read all Data?
    {
        CMOD_SET_RD;                                        // RD rises after a read at 140ns (in GB)

        CMOD_Status = CMOD_DATA_READY;                      // -> Data Ready
        CMOD_Action = CMOD_NOACTION;                        // All actions finished
        CMOD_DISABLE_INTERRUPT();                           // Disable Interrupt until needed again
        return;
    }

    CMOD_SET_ADDR(CMOD_Address + CMOD_BytesRead);           // Address changes at 140ns (in GB)
    CMOD_RST_CS;                                            // CS goes low at 240ns (in GB)
    CMOD_BytesRead++;                                       // Increase BytesRead in good faith
}

void CMOD_HandleWrite(void)
{
    CMOD_SET_CS;                                            // CS rises at 0ns (after CLK Flank)

    if (CMOD_BytesWritten == CMOD_BytesToWrite)             // All Bytes written?
    {
        CMOD_SET_WR;                                        // WR rises after a write 20ns before the CLK rises (in GB)

        CMOD_Status = CMOD_WRITE_COMPLETE;                  // -> Write complete
        CMOD_Action = CMOD_NOACTION;                        // All actions finished
        CMOD_DISABLE_INTERRUPT();                           // Disable Interrupt until needed again
        return;
    }

    CMOD_SET_ADDR(CMOD_Address + CMOD_BytesWritten);        // Address changes at 140ns (in GB)
    CMOD_RST_CS;                                            // CS goes low at 240ns (in GB)
    CMOD_BytesWritten++;                                    // Increase BytesWritten in good faith
    CMOD_SET_DATA(CMOD_DataOut[CMOD_BytesWritten - 1]);     // Data Bus is driven at 480ns (falling CLK) (in GB)
}

void CMOD_HandleNOP(void)
{
    return;
}

void TIM4_IRQHandler(void)
{
    CMOD_OperationTable[CMOD_Action]();                       // Handle Read / Write / NOP

    __HAL_TIM_CLEAR_IT(&CMOD_TimerHandle, TIM_IT_CC1);
}
