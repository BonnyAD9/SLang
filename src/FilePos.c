#include "FilePos.h"

#include <stddef.h>
#include <stdio.h>

#include "String.h"
#include "Stream.h"

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

int fpPrint(Stream* out, FilePos fp)
{
    return stPrintf(out, "%s:%zu:%zu", fp.filename->c, fp.line, fp.col);
}