#include "FileSpan.h"

#include <stdlib.h>
#include <string.h>

#include "Assert.h"
#include "Terminal.h"

FileSpan copyFileSpanFrom(const char* buffer, size_t length, size_t line, size_t col)
{
    assert(buffer, "copyFileSpanFom: parameter buffer was null");
    char* str = malloc((length + 1) * sizeof(char));
    str[length] = 0;
    assert(str, "copyFileSpanFrom: failed to allocate string of length %"term_SIZE_T, length + 1);
    strncpy(str, buffer, length);
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