#ifndef INPUT_INIT_H
#define INPUT_INIT_H

#include "common.h"
#include "input_config.h"

#pragma pack(1)
typedef struct
{
    GPIO_TypeDef* Port;
    uint32_t Pin;
}
Input_Pins_t;


extern TIM_HandleTypeDef Input_PollTimerHandle;



void Input_Initialize(void);

#endif //INPUT_INIT_H
