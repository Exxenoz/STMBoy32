#include "input_locks.h"
#include "input_config.h"


Input_Lock_t Input_Locks[8] =
{
    { INPUT_A_ID,          0x00, 0x00, false, false },
    { INPUT_B_ID,          0x00, 0x00, false, false },
    { INPUT_SELECT_ID,     0x00, 0x00, false, false },
    { INPUT_START_ID,      0x00, 0x00, false, false },
    { INPUT_FADE_RIGHT_ID, 0x00, 0x00, false, false },
    { INPUT_FADE_LEFT_ID,  0x00, 0x00, false, false },
    { INPUT_FADE_TOP_ID,   0x00, 0x00, false, false },
    { INPUT_FADE_BOT_ID,   0x00, 0x00, false, false },
};

time_t Input_DynamicLockTimes[8] =
{
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
    INPUT_MAX_DYNAMIC_LOCK_TIME,
};



void Input_UpdateLocks(void)
{
    for (int i = 0; i < 8; i ++)
    {
        if (Input_Locks[i].IsLocked)
        {
            // If lock is enabled calculate for how long it has been locked.
            long timeLocked = INPUT_LOCK_TIM->CNT - Input_Locks[i].LockedSince;

            // If timeLocked is negative timer has overflowed -> add max timer value to correct it.
            if (timeLocked < 0) timeLocked += MAX_16BIT_TIMER_PERIOD;

            // Disable the lock if button was released.
            if (!(Input_Interrupt_Flags.allFlags & (Input_Pins[Input_Locks[i].ID].Pin >> 1)))
            {
                Input_Locks[i].IsLocked             = false;
                Input_Locks[i].WasUnlockedByTimeout = false;
            }

            // Disable the lock if enough time has passed and unlocking by timeout is enabled.
            if (Input_Locks[i].LockedFor != 0 && timeLocked > Input_Locks[i].LockedFor)
            {
                Input_Locks[i].IsLocked             = false;
                Input_Locks[i].WasUnlockedByTimeout = true;
            }
        }
    }
}

// Lock button for lockTime in ms (max 16s). Setting lockTime to 0 disables unlocking by timeout.
void Input_Lock(Input_Button_ID_t id, time_t lockTime)
{
    Input_Locks[id].IsLocked    = true;
    Input_Locks[id].LockedFor   = ((INPUT_LOCK_TIM_FREQ / (INPUT_LOCK_TIM_PRESCALER + 1)) / 1000) * lockTime; // Multiply lockTime by actual timer frequency / 1000 to get ms.
    Input_Locks[id].LockedSince = INPUT_LOCK_TIM->CNT;
}

void Input_LockAll(time_t lockTime)
{
    for (int i = 0; i < 8; i++)
    {
        Input_Lock((Input_Button_ID_t)i, lockTime);
    }
}

void Input_LockDynamically(Input_Button_ID_t id)
{
    if (Input_Locks[id].WasUnlockedByTimeout && Input_DynamicLockTimes[id] > INPUT_MIN_DYNAMIC_LOCK_TIME)
    {
        Input_DynamicLockTimes[id] -= INPUT_DYNAMIC_LOCK_TIME_STEP;
    }
    else
    {
        Input_DynamicLockTimes[id] = INPUT_MAX_DYNAMIC_LOCK_TIME;
    }

    Input_Lock(id, Input_DynamicLockTimes[id]);
}


bool Input_IsLocked(Input_Button_ID_t id)
{
    return Input_Locks[id].IsLocked;
}
