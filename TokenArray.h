#ifndef TOKEN_ARRAY_INCLUDED
#define TOKEN_ARRAY_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "Token.h"

/**
 * @brief Represents a array of tokens
 * 
 */
typedef struct TokenArray
{
    size_t length;
    Token* data;
} TokenArray;

/**
 * @brief prints all the tokens on separate lines
 * 
 * @param out stream to print to
 * @param arr The TokenArray to print
 */
void printTokenArray(FILE* out, TokenArray arr);

/**
 * @brief Create a Token Array object
 * 
 * @param length length of the array
 * @return TokenArray
 */
TokenArray createTokenArray(size_t length);

/**
 * @brief frees this token array
 * 
 * @param arr array to free
 * @param deep indicates whether each token should also be freed
 */
void freeTokenArray(TokenArray arr, bool deep);

#endif // TOKEN_ARRAY_INCLUDED