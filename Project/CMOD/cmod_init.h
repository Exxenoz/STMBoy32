#ifndef CMOD_INIT_H
#define CMOD_INIT_H

#include "common.h"


extern bool              CMOD_Initialized;
extern TIM_HandleTypeDef CMOD_TimerHandle;



void CMOD_Initialize(void); 

#endif //CMOD_INIT_H
