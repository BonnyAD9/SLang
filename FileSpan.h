#ifndef STR_SPAN_INCLUDED
#define STR_SPAN_INCLUDED

#include <stdlib.h>

typedef struct FileSpan
{
    char* str;
    size_t line;
    size_t col;
} FileSpan;

/**
 * @brief Create a FileSpan object
 * 
 * @param str value of the span
 * @param line line number in file
 * @param col column number in file
 * @return FileSpan new instance
 */
FileSpan createFileSpan(char* str, size_t line, size_t col);

/**
 * @brief Copies file span from buffer
 * 
 * @param buffer buffer to copy from
 * @param length number of characters to copy
 * @param line line in file
 * @param col column in file
 * @return FileSpan new instance
 */
FileSpan copyFileSpanFrom(const char* buffer, size_t length, size_t line, size_t col);

/**
 * @brief frees this instance of file span
 * 
 * @param span span to free
 */
void freeFileSpan(FileSpan span);

#endif // STR_SPAN_INCLUDED