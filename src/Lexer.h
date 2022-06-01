#ifndef LEXER_INCLUDED
#define LEXER_INCLUDED

#include <stdio.h>

#include "List.h"

#ifndef LEXER_READ_BUFFER_SIZE
#define LEXER_READ_BUFFER_SIZE 1024LL
#endif // LEXER_READ_BUFFER_SIZE

#ifndef LONG_LONG_MAX
#define LONG_LONG_MAX 9223372036854775807LL
#endif // LONG_LONG_MAX

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @param errors where to put error tokens
 * @return List of tokens Tokenized result
 */
List lex(FILE* in, List* errors);

#endif // LEXER_INCLUDED
