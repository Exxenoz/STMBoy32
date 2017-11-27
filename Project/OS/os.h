#ifndef OS_H
#define OS_H

#define OS_MAIN_PAGE_BUTTON_LOCK_TIME 150

typedef enum
{
    OS_MAIN_PAGE,
    OS_SHOW_ALL,
    OS_SHOW_FAV,
    OS_OPTIONS,
    OS_INGAME_FROM_SDC,
    OS_INGAME_FROM_CARTRIDGE,
}
OS_State_t;

typedef enum
{
    OS_SWITCH_TO_STATE_INGAME_FC,  // Switch current state to OS_INGAME_FROM_CARTRIDGE
    OS_SWITCH_TO_STATE_INGAME_FSD, // Switch current state to OS_INGAME_FROM_SDC
    OS_SWITCH_TO_STATE_SHOWALL,    // Switch current state to OS_SHOW_ALL
    OS_SWITCH_TO_STATE_SHOWFAV,    // Switch current state to OS_SHOW_FAV
    OS_SWITCH_TO_STATE_OPTIONS,    // Switch current state to OS_OPTIONS
    // More TBI
}
OS_Action_t;

extern OS_State_t OS_CurrState;
extern OS_State_t OS_LastState;

void OS_DoAction(OS_Action_t action);

#endif
