#include "List.h"

#include <stdlib.h>
#include <assert.h>

#include "FileSpan.h"

List listCreate(size_t elementSize)
{
    List list =
    {
        .data = malloc(list_START_SIZE * elementSize),
        .allocated = list_START_SIZE,
        .length = 0,
        .element = elementSize
    };

    assert(list.data);

    return list;
}

void listFree(List list)
{
    assert(list.data);
    free(list.data);
}

void listAddP(List* list, void* pItem)
{
    assert(pItem);
    unsigned char* item = (unsigned char*)pItem;
    if (list->allocated == list->length)
    {
        size_t newSize = list->allocated < list_ALLOC_SIZE ? list->allocated * 2 : list->allocated + list_ALLOC_SIZE;
        unsigned char* newData = realloc(list->data, newSize * list->element);
        assert(newData);
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

ListIterator liCreate(List* list)
{
    ListIterator li =
    {
        .list = list,
        .pos = 0,
    };
    return li;
}

void* liGetP(ListIterator* iterator)
{
    return listGetP(*iterator->list, iterator->pos);
}

_Bool liMove(ListIterator* iterator)
{
    iterator->pos++;
    return iterator->pos < iterator->list->length;
}

_Bool liCan(ListIterator* iterator)
{
    return iterator->pos < iterator->list->length;
}

void liAddP(ListIterator* iterator, void* element)
{
    listAddP(iterator->list, element);
}