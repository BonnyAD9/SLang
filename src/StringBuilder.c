#include "StringBuilder.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "String.h"

StringBuilder sbCreate()
{
    StringBuilder sb =
    {
        .buffer = malloc(sb_INITIAL_SIZE * sizeof(char)),
        .allocated = sb_INITIAL_SIZE,
        .length = 0,
    };
    assert(sb.buffer);
    return sb;
}

void sbFree(StringBuilder* sb)
{
    free(sb->buffer);
    sb->allocated = 0;
    sb->length = 0;
}

void sbAdd(StringBuilder* sb, char c)
{
    if (sb->allocated <= sb->length)
    {
        size_t newSize = sb->allocated < sb_ALLOC_STEP ? sb->allocated * 2 : sb->allocated + sb_ALLOC_STEP;
        if (newSize <= 0)
            newSize = 1;
        char* newMem = realloc(sb->buffer, newSize * sizeof(char));
        assert(newMem);
        sb->buffer = newMem;
        sb->allocated = newSize;
    }
    sb->buffer[sb->length] = c;
    sb->length++;
}

void sbAppend(StringBuilder* sb, const char* str)
{
    for (; *str; str++)
        sbAdd(sb, *str);
}

void sbAppendL(StringBuilder* sb, const char* str, size_t length)
{
    size_t need = length + sb->length;
    if (need > sb->allocated)
    {
        size_t newSize = sb->allocated + length;
        if (newSize <= 0)
            newSize = 1;
        char* newMem = realloc(sb->buffer, newSize * sizeof(char));
        assert(newMem);
        sb->buffer = newMem;
        sb->allocated = newSize;
    }
    memcpy_s(sb->buffer + sb->length, sb->allocated - sb->length, str, length);
    sb->length += length;
}

String sbGet(StringBuilder* sb)
{
    return strCLen(sb->buffer, sb->length);
}

void sbClear(StringBuilder* sb)
{
    sb->length = 0;
}