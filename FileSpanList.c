#include "FileSpanList.h"

#include <stdlib.h>

#include "FileSpan.h"
#include "Assert.h"

FileSpanList createFileSpanList()
{
    FileSpanList list =
    {
        .data = malloc(STRING_LIST_START_SIZE * sizeof(FileSpan)),
        .allocated = STRING_LIST_START_SIZE,
        .length = 0,
    };

    assert(list.data, "createStringList: cannot allocate list data of size %I64d", STRING_LIST_START_SIZE);

    return list;
}

void freeFileSpanList(FileSpanList list, bool deep)
{
    assert(list.data, "freeStringList: list has null as data");

    if (deep)
    {
        for (size_t i = 0; i < list.length; i++)
        {
            freeFileSpan(list.data[i]);
        }
    }
    free(list.data);
}

void fileSpanListAdd(FileSpanList* list, FileSpan item)
{
    if (list->allocated == list->length)
    {
        size_t newSize = list->allocated < STRING_LIST_ALLOC_SIZE ? list->allocated * 2 : list->allocated + STRING_LIST_ALLOC_SIZE;
        FileSpan* newData = realloc(list->data, newSize * sizeof(FileSpan));
        assert(newData, "stringListAdd: cannot reallocate memory of size %I64d", newSize);
        list->data = newData;
        list->allocated = newSize;
    }
    list->data[list->length] = item;
    list->length++;
}