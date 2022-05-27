#ifndef STRING_LIST_INCLUDED
#define STRING_LIST_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

#include "FileSpan.h"

#ifndef STRING_LIST_START_SIZE
#define STRING_LIST_START_SIZE 16LL
#endif // STRING_LIST_START_SIZE

#ifndef STRING_LIST_ALLOC_SIZE
#define STRING_LIST_ALLOC_SIZE 128LL
#endif // STRING_LIST_ALLOC_SIZE

/**
 * @brief represents list of strings
 * 
 */
typedef struct FileSpanList
{
    FileSpan* data;
    size_t allocated;
    size_t length;
} FileSpanList;

/**
 * @brief Create a String List object
 * 
 * @return new StringList
 */
FileSpanList createFileSpanList();

/**
 * @brief frees this string list
 * 
 * @param list list to free
 * @param deep indicates whether each string should be freed
 */
void freeFileSpanList(FileSpanList list, bool deep);

/**
 * @brief adds string to the list
 * 
 * @param list list to which the string will be added
 * @param item string to add to the list
 */
void fileSpanListAdd(FileSpanList* list, FileSpan item);

#endif // STRING_LIST_INCLUDED