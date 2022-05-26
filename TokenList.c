#include "TokenList.h"

#include <stdlib.h>

#include "Assert.h"

TokenList createStringList()
{
    TokenList list =
    {
        .data = malloc(STRING_LIST_START_SIZE * sizeof(char*)),
        .allocated = STRING_LIST_START_SIZE,
        .length = 0,
    };

    assert(list.data, "createStringList: cannot allocate list data of size %lld", STRING_LIST_START_SIZE);

    return list;
}

void freeStringList(TokenList list, bool deep)
{
    assert(list.data, "freeStringList: list has been already freed");

    if (deep)
    {
        for (size_t i = 0; i < list.length; i++)
        {
            if (list.data[i])
                free(list.data[i]);
        }
    }
    free(list.data);
    list.data = NULL;
}

void stringListAdd(TokenList list, char* item)
{
    if (list.allocated == list.length)
    {
        size_t newSize = list.allocated < STRING_LIST_ALLOC_SIZE ? list.allocated * 2 : list.allocated + STRING_LIST_ALLOC_SIZE;
        char** newData = realloc(list.data, newSize * sizeof(char*));
        assert(newData, "stringListAdd: cannot reallocate memory of size %lld", newSize);
        list.data = newData;
        list.allocated = newSize;
    }
    list.data[list.length] = item;
    list.length++;
}