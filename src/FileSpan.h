#ifndef fs_FILE_SPAN_INCLUDED
#define fs_FILE_SPAN_INCLUDED

#include <stdlib.h>

typedef struct FileSpan
{
    char* str;
    size_t length;
    size_t line;
    size_t col;
} FileSpan;

/**
 * @brief Copies file span from buffer
 * 
 * @param buffer buffer to copy from
 * @param length number of characters to copy
 * @param line line in file
 * @param col column in file
 * @return FileSpan new instance
 */
FileSpan fsCopyFrom(const char* buffer, size_t length, size_t line, size_t col);

/**
 * @brief frees this instance of file span
 * 
 * @param span span to free
 */
void fsFree(FileSpan span);

#endif // fs_FILE_SPAN_INCLUDED