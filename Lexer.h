#ifndef LEXER_INCLUDED
#define LEXER_INCLUDED

#include <stdio.h>

#include "TokenArray.h"

#ifndef LEXER_READ_BUFFER_SIZE
#define LEXER_READ_BUFFER_SIZE 1024
#endif // LEXER_READ_BUFFER_SIZE

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @return Tokenized result
 */
TokenArray lex(FILE* in);



#endif // LEXER_INCLUDED