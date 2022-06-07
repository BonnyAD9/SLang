#include "FileSpan.h"

#include "String.h"
#include "FilePos.h"
#include "DebugTools.h"

FileSpan fsCreate(String str, FilePos position)
{
    FileSpan fs =
    {
        .str = str,
        .pos = position,
    };
    return fs;
}

void fsFree(FileSpan fs)
{
    strFree(fs.str);
}