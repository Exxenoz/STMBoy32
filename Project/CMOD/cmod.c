#include "cmod.h"

void CMOD_Initialize(void)
{
    RCC_AHB1PeriphClockCmd(CMOD_RESET_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CS_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_RD_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_WR_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_CLK_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_ADDR_BUS, ENABLE);
    RCC_AHB1PeriphClockCmd(CMOD_DATA_BUS, ENABLE);
}
