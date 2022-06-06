#ifndef STRING_LIST_INCLUDED
#define STRING_LIST_INCLUDED

#include <stdlib.h>

#include "FileSpan.h"

#ifndef STRING_LIST_START_SIZE
#define STRING_LIST_START_SIZE 16LL
#endif // STRING_LIST_START_SIZE

#ifndef STRING_LIST_ALLOC_SIZE
#define STRING_LIST_ALLOC_SIZE 128LL
#endif // STRING_LIST_ALLOC_SIZE

#define newList(__type) createList(sizeof(__type))

#define listAdd(__list, __item, __type) \
{\
    __type __i = __item;\
    listAddP(&__list, &__i);\
}

#define listGet(__list, __index, __type) (*(__type*)listGetP(__list, __index))

#define listDeepFree(__list, __type, __name, __free) \
{\
    for (size_t __i = 0; __i < __list.length; __i++)\
    {\
        __type __name = *(__type*)listGetP(__list,__i);\
        __free;\
    }\
    freeList(__list);\
}

#define listForEach(__list, __type, __name, __code) \
{\
    for (size_t __i = 0; __i < __list.length; __i++)\
    {\
        __type __name = *(__type*)listGetP(__list,__i);\
        __code;\
    }\
}

#define liNext(__iterator, __type, __out, __ifnot) \
{\
    if (liMove(__iterator))\
        __out = *(__type*)listGetP(*(__iterator)->list, (__iterator)->pos);\
    else {__ifnot;}\
}

#define liGet(__iterator, __type) (*(__type*)liGetP(__iterator))

#define liAdd(__iterator, __element, __type) \
{\
    __type __e = __element;\
    liAddP(__iterator, &__e);\
}

/**
 * @brief represents list of strings
 * 
 */
typedef struct List
{
    unsigned char* data;
    size_t element;
    size_t allocated;
    size_t length;
} List;

/**
 * @brief structure that iterates trough list
 * 
 */
typedef struct ListIterator
{
    List* list;
    size_t pos;
} ListIterator;

/**
 * @brief creates new list iterator
 * 
 * @param list list to iterate
 * @return ListIterator new instance
 */
ListIterator liCreate(List* list);

/**
 * @brief gets the current element in the list iterator
 * 
 * @param iterator iterator
 * @return void* pointer to the current element
 */
void* liGetP(ListIterator* iterator);

/**
 * @brief moves the iterator to the next position
 * 
 * @param iterator iterator
 * @return true moved successfully
 * @return false moved unsuccessfully
 */
_Bool liMove(ListIterator* iterator);

/**
 * @brief returns value indicating whether the iterator reached end
 * 
 * @param iterator iteraotor
 * @return true did not reach end
 * @return false did reach end
 */
_Bool liCan(ListIterator* iterator);

/**
 * @brief adds element to the end of the list and moves the position there
 * 
 * @param iterator iterator
 * @param element element to add
 */
void liAddP(ListIterator* iterator, void* element);

/**
 * @brief Create a String List object
 * 
 * @return new StringList
 */
List createList(size_t elementSize);

/**
 * @brief frees this string list
 * 
 * @param list list to free
 */
void freeList(List list);

/**
 * @brief adds string to the list
 * 
 * @param list list to which the string will be added
 * @param item pointer to item to add to the list
 */
void listAddP(List* list, void* pItem);

/**
 * @brief gets item at the given index
 * 
 * @param list list to get the item from
 * @param index index of the item
 * @return void* pointer to the item
 */
void* listGetP(List list, size_t index);

#endif // STRING_LIST_INCLUDED