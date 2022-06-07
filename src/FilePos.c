#include "FilePos.h"

#include <stddef.h>
#include <stdio.h>

#include "String.h"

FilePos fpCreate(size_t line, size_t col, String* filename)
{
    FilePos fp =
    {
        .line = line,
        .col = col,
        .filename = filename,
    };
    return fp;
}

int fpPrint(FILE* out, FilePos fp)
{
    return fprintf(out, "%s:%zu:%zu", fp.filename->data, fp.line, fp.col);
}