#include "common.h"

void copyString(char *dest, const char *src, int destSize)
{
    int i;

    for (i = 0; i < (destSize - 1) && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}
