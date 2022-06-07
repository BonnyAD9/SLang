#ifndef sb_STRING_BUILDER
#define sb_STRING_BUILDER

#include <stddef.h>

#include "String.h"

#ifndef sb_INITIAL_SIZE
#define sb_INITIAL_SIZE 16
#endif // sb_INITIAL_SIZE

#ifndef sb_ALLOC_STEP
#define sb_ALLOC_STEP 128
#endif // sb_ALLOC_STEP

typedef struct StringBuilder
{
    char* buffer;
    size_t allocated;
    size_t length;
} StringBuilder;

/**
 * @brief creates news StringBuilder
 * 
 * @return StringBuilder new instance
 */
StringBuilder sbCreate();

/**
 * @brief frees the given StringBuilder
 * 
 * @param sb what to free
 */
void sbFree(StringBuilder* sb);

/**
 * @brief adds another char to the StringBuilder
 * 
 * @param sb where to add
 * @param c what to add
 */
void sbAdd(StringBuilder* sb, char c);

/**
 * @brief appends string to the StringBuilder
 * 
 * @param sb where to append
 * @param str what to append
 */
void sbAppend(StringBuilder* sb, const char* str);

/**
 * @brief appends the given number of characters
 * 
 * @param sb whare to append
 * @param str what to append
 * @param length how much append
 */
void sbAppendL(StringBuilder* sb, const char* str, size_t length);

/**
 * @brief gets the current contents
 * 
 * @param sb from
 * @return String result
 */
String sbGet(StringBuilder* sb);

/**
 * @brief clears the given StringBuilder
 * 
 * @param sb what ot clear
 */
void sbClear(StringBuilder* sb);

#endif // sb_STRING_BUILDER