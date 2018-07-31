#include "common.h"
#include "string.h"



// Compare two strings if string a is alphabetically 'higher' the return value is > 0 (0 for equal strings)
int CmpStrings(const void *a, const void *b)
{
    char *string1 = (char*)a;
    char *string2 = (char*)b;

    // Get the first different letters and compare them (case insensitive)
    int i = -1;
    do
    {
        i++;
        if (string1[i] >= 97 && string1[i] <= 122)
        {
            string1[i] -= 32;
        }
        if (string2[i] >= 97 && string2[i] <= 122)
        {
            string2[i] -= 32;
        }

    } while (string1[i] == string2[i] && string1[i] != '\0');

    return (string1[i] - string2[i]);
}

void CopyString(char *dest, const char *src, int destSize)
{
    // Clear destination
    memset(dest, 0, destSize);

    int i;

    for (i = 0; i < (destSize - 1) && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

void CopyChars(char *dest, const char *src, int destSize, int srcSize)
{
    // Clear destination
    memset(dest, 0, destSize);

    for (int i = 0; i < destSize && i < srcSize; i++)
    {
        dest[i] = src[i];
    }
}

void CopyWithoutSuffix(char *dest, const char *src, int destSize)
{
    // Clear destination
    memset(dest, 0, destSize);

    int i;

    for (i = 0; i < (destSize - 1) && src[i] != '\0' && (src[i] != '.' || src[i+1] != 'g' || src[i+2] != 'b'); i++)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

void AppendString(char *dest, const char *src, int destSize)
{
    int i;

    for (i = 0; dest[i] != '\0'; i++);

    for (int j = 0; i < (destSize - 1) && src[j] != '\0'; i++, j++)
    {
        dest[i] = src[j];
    }

    dest[i] = '\0';
}

void AppendChars(char *dest, const char *src, int destSize, int srcSize)
{
    int i;

    for (i = 0; dest[i] != '\0'; i++);

    for (int j = 0; i < destSize && j < srcSize; i++, j++)
    {
        dest[i] = src[j];
    }
}

int CountChars(const char *src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++);
    return i;
}
