#include "String.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "DebugTools.h"

String strEmpty()
{
    String str =
    {
        .c = NULL,
        .length = 0,
    };
    return str;
}

void strFree(String s)
{
    if (s.c)
        free(s.c);
}

String strCopy(String s)
{
    return strCLen(s.c, s.length);
}

String strC(const char* str)
{
    return strCLen(str, strlen(str));
}

String strCLen(const char* str, size_t length)
{
    String s =
    {
        .length = length,
        .c = malloc((length + 1) * sizeof(char)),
    };

    assert(s.c);

    s.c[length] = 0;

    strncpy_s(s.c, length + 1, str, length);
    return s;
}

_Bool strEquals(String str1, String str2)
{
    if (str1.length != str2.length)
        return 0;
    return strcmp(str1.c, str2.c) == 0;
}

_Bool strEqualsC(String str1, const char* str2, size_t str2Length)
{
    if (str1.length != str2Length)
        return 1;
    return strcmp(str1.c, str2) == 0;
}