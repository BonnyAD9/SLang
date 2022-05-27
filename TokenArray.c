#include "TokenArray.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "Token.h"
#include "Assert.h"

void printTokenArray(FILE* out, TokenArray arr)
{
    assert(out, "printTokenArray: out was null\n");

    for (size_t i = 0; i < arr.length; i++)
    {
        printToken(out, arr.data[i]);
        fprintf(out, "\n");
    }
}

TokenArray createTokenArray(size_t length)
{
    TokenArray arr =
    {
        .length = length,
        .data = malloc(sizeof(Token) * length),
    };

    assert(arr.data, "createTokenArray: TokenArray.data failed to allocate data of size %lld\n", length);

    return arr;
}

void freeTokenArray(TokenArray arr, bool deep)
{
    if (deep)
    {
        for (size_t i = 0; i < arr.length; i++)
            freeToken(arr.data[i]);
    }
    free(arr.data);
}