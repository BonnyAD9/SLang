#ifndef STRING_LIST_INCLUDED
#define STRING_LIST_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

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
typedef struct TokenList
{
    char** data;
    size_t allocated;
    size_t length;
} TokenList;

/**
 * @brief Create a String List object
 * 
 * @return new StringList
 */
TokenList createStringList();

/**
 * @brief frees this string list
 * 
 * @param list list to free
 * @param deep indicates whether each string should be freed
 */
void freeStringList(TokenList list, bool deep);

/**
 * @brief adds string to the list
 * 
 * @param list list to which the string will be added
 * @param item string to add to the list
 */
void stringListAdd(TokenList list, char* item);

/**
 * @brief returns the last added element
 * 
 * @param list 
 */
void stringListLast(TokenList list);

#endif // STRING_LIST_INCLUDED