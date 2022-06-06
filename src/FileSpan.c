#include "FileSpan.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

FileSpan copyFileSpanFrom(const char* buffer, size_t length, size_t line, size_t col)
{
    assert(buffer);
    char* str = malloc((length + 1) * sizeof(char));
    assert(str);
    str[length] = 0;
    strncpy_s(str, length + 1, buffer, length);
    FileSpan span =
    {
        .str = str,
        .length = length,
        .line = line,
        .col = col,
    };
    return span;
}

void freeFileSpan(FileSpan span)
{
    free(span.str);
}