#include "String.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Assert.h"
#include "Terminal.h"

String strEmpty()
{
    String str =
    {
        .data = NULL,
        .length = 0,
    };
    return str;
}

void strFree(String s)
{
    if (s.data)
        free(s.data);
}

String strCopy(String s)
{
    return strCLen(s.data, s.length);
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
        .data = malloc((length + 1) * sizeof(char)),
    };

    s.data[length] = 0;

    assert(s.data, "strLiteral: failed to allocate string of length %"term_SIZE_T, length + 1);

    strcpy(s.data, str);
    return s;
}

bool strEquals(String str1, String str2)
{
    if (str1.length != str2.length)
        return false;
    return strcmp(str1.data, str2.data) == 0;
}

bool strEqualsC(String str1, const char* str2, size_t str2Length)
{
    if (str1.length != str2Length)
        return false;
    return strcmp(str1.data, str2) == 0;
}