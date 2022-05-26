#ifndef LEXER_INCLUDED
#define LEXER_INCLUDED

#include <stdio.h>

#include "TokenList.h"

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @return Tokenized result
 */
TokenList lex(FILE* in);

#endif // LEXER_INCLUDED