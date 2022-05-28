#ifndef LEXER_INCLUDED
#define LEXER_INCLUDED

#include <stdio.h>

#include "List.h"

#ifndef LEXER_READ_BUFFER_SIZE
#define LEXER_READ_BUFFER_SIZE 1024
#endif // LEXER_READ_BUFFER_SIZE

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @param errors where to put error tokens
 * @return List of tokens Tokenized result
 */
List lex(FILE* in, List* errors);

#endif // LEXER_INCLUDED