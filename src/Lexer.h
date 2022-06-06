#ifndef LEXER_INCLUDED
#define LEXER_INCLUDED

#include <stdio.h>

#include "List.h"

#ifndef LEXER_READ_BUFFER_SIZE
#define LEXER_READ_BUFFER_SIZE 1024LL
#endif // LEXER_READ_BUFFER_SIZE

#ifndef DECIMAL_WARNING_LIMIT
#define DECIMAL_WARNING_LIMIT 17
#endif // DECIMAL_WARNING_LIMIT

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @param errors where to put error tokens
 * @return List of tokens Tokenized result
 */
List lex(FILE* in, List* errors);

#endif // LEXER_INCLUDED
