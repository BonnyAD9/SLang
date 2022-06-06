#ifndef lex_LEXER_INCLUDED
#define lex_LEXER_INCLUDED

#include <stdio.h>

#include "List.h"

#ifndef lex_LEXER_READ_BUFFER_SIZE
#define lex_LEXER_READ_BUFFER_SIZE 1024LL
#endif // lex_LEXER_READ_BUFFER_SIZE

#ifndef lex_DECIMAL_WARNING_LIMIT
#define lex_DECIMAL_WARNING_LIMIT 17
#endif // lex_DECIMAL_WARNING_LIMIT

/**
 * @brief tokenizes the given stream
 * 
 * @param in stream to read from
 * @param errors where to put error tokens
 * @return List of tokens Tokenized result
 */
List lexLex(FILE* in, List* errors);

#endif // lex_LEXER_INCLUDED
