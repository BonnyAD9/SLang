#include "FileSpan.h"

#include <stdlib.h>
#include <string.h>

#include "Assert.h"

FileSpan createFileSpan(char* str, size_t line, size_t col)
{
    FileSpan span =
    {
        .str = str,
        .line = line,
        .col = col,
    };
    return span;
}

FileSpan copyFileSpanFrom(const char* buffer, size_t length, size_t line, size_t col)
{
    assert(buffer, "copyFileSpanFom: parameter buffer was null");
    char* str = malloc((length + 1) * sizeof(char));
    str[length] = 0;
    assert(str, "copyFileSpanFrom: failed to allocate string of length %I64d", length + 1);
    strncpy(str, buffer, length);
    return createFileSpan(str, line, col);
}

void freeFileSpan(FileSpan span)
{
    free(span.str);
}