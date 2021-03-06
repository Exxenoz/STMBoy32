#ifndef INPUT_LOCKS_H
#define INPUT_LOCKS_H

#include "input.h"
#include "common.h"


#define INPUT_LOCK_UNTIL_RELEASED          0   // Per convention locks button until it's released (second unlock condition).

#define INPUT_MAX_DYNAMIC_LOCK_TIME        300
#define INPUT_MIN_DYNAMIC_LOCK_TIME        50
#define INPUT_DYNAMIC_LOCK_TIME_STEP       25


#pragma pack(1)
typedef struct
{
    Input_Button_ID_t ID;                    // ID of the button the lock is associated with.
    time_t            LockedSince;           // LockTimer value at the time lock got activated.
    time_t            LockedFor;             // Time the button should remain locked if not released in ms.
    bool              IsLocked;              // State of the lock.
    bool              WasUnlockedByTimeout;  // Indicates if last unlock was triggered by release or timeout.
}
Input_Lock_t;


extern Input_Lock_t Input_Locks[8];



void Input_UpdateLocks(void);

void Input_Lock(Input_Button_ID_t id, time_t lockTime);
void Input_LockAll(time_t lockTime);
void Input_LockDynamically(Input_Button_ID_t id);

bool Input_IsLocked(Input_Button_ID_t id);

#endif //INPUT_LOCKS_H
