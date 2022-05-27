#include "List.h"

#include <stdlib.h>

#include "FileSpan.h"
#include "Assert.h"

List createList(size_t elementSize)
{
    List list =
    {
        .data = malloc(STRING_LIST_START_SIZE * sizeof(FileSpan)),
        .allocated = STRING_LIST_START_SIZE,
        .length = 0,
        .element = elementSize
    };

    assert(list.data, "createStringList: cannot allocate list data of size %I64d and element size %I64d", STRING_LIST_START_SIZE, elementSize);

    return list;
}

void freeList(List list)
{
    assert(list.data, "freeStringList: list has null as data");
    free(list.data);
}

void listAddP(List* list, void* pItem)
{
    assert(pItem, "fileSpanListAdd: parameter pItem was null");
    unsigned char* item = (unsigned char*)pItem;
    if (list->allocated == list->length)
    {
        size_t newSize = list->allocated < STRING_LIST_ALLOC_SIZE ? list->allocated * 2 : list->allocated + STRING_LIST_ALLOC_SIZE;
        unsigned char* newData = realloc(list->data, newSize * list->element);
        assert(newData, "stringListAdd: cannot reallocate memory of size %I64d", newSize);
        list->data = newData;
        list->allocated = newSize;
    }

    size_t offset = list->length * list->element;
    for (size_t i = 0; i < list->element; i++)
        list->data[offset + i] = item[i];
    list->length++;
}

void* listGetP(List list, size_t index)
{
    return &list.data[index * list.element];
}