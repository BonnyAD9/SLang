#ifndef fp_FILE_POS_INCLUDED
#define fp_FILE_POS_INCLUDED

#include <stddef.h>
#include <stdio.h>

#include "String.h"

typedef struct FilePos
{
    size_t line;
    size_t col;
    String* filename;
} FilePos;

/**
 * @brief creates new FilePos object
 * 
 * @param line line position
 * @param col column position
 * @param filename name of file
 * @return FilePos new instance
 */
FilePos fpCreate(size_t line, size_t col, String* filename);

/**
 * @brief prints FilePos into stream
 * 
 * @param out where to print
 * @param fp what to print
 * @return int number of printed characters
 */
int fpPrint(FILE* out, FilePos fp);

#endif // fp_FILE_POS_INCLUDED