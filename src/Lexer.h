#ifndef lex_LEXER_INCLUDED
#define lex_LEXER_INCLUDED

#include "List.h"
#include "Stream.h"

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
List lexLex(Stream* in, List* errors, String* filename);

#endif // lex_LEXER_INCLUDED
