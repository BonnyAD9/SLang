#ifndef fs_FILE_SPAN_INCLUDED
#define fs_FILE_SPAN_INCLUDED

#include "FilePos.h"
#include "String.h"

typedef struct FileSpan
{
    String str;
    FilePos pos;
} FileSpan;

/**
 * @brief creates new FileSpan
 * 
 * @param str contents
 * @param position position in file
 * @return FileSpan 
 */
FileSpan fsCreate(String str, FilePos position);

/**
 * @brief frees this instance of file span
 * 
 * @param span span to free
 */
void fsFree(FileSpan span);

#endif // fs_FILE_SPAN_INCLUDED