#include "os.h"

OS_State_t OS_CurrState = OS_MAIN_PAGE;
OS_State_t OS_LastState = OS_MAIN_PAGE;

void OS_DoAction(OS_Action_t action)
{
    switch (action)
    {
        case OS_SWITCH_TO_STATE_SHOWALL:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_SHOW_ALL;
            break;
        case OS_SWITCH_TO_STATE_SHOWFAV:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_SHOW_FAV;
            break;
        case OS_SWITCH_TO_STATE_OPTIONS:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_OPTIONS;
            break;
        case OS_SWITCH_TO_STATE_INGAME_FC:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_INGAME_FROM_CARTRIDGE;
            break;
        case OS_SWITCH_TO_STATE_INGAME_FSD:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_INGAME_FROM_SDC;
            break;
    }
}
