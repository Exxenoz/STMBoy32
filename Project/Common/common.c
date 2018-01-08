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

void copyChars(char *dest, const char *src, int destSize)
{
    int i;

    for (i = 0; i < destSize; i++)
    {
        dest[i] = src[i];
    }
}

void copyWithoutSuffix(char *dest, const char *src, int destSize)
{
    int i;

    for (i = 0; i < (destSize - 1) && src[i] != '\0' && (src[i] != '.' || src[i+1] != 'g' || src[i+2] != 'b'); i++)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

void appendString(char *dest, const char *src, int destSize)
{
    int i;

    for (i = 0; dest[i] != '\0'; i++);

    for (int j = 0; i < (destSize - 1) && src[j] != '\0'; i++, j++)
    {
        dest[i] = src[j];
    }

    dest[i] = '\0';
}
