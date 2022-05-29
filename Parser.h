#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "ParserTree.h"
#include "List.h"

/**
 * @brief parses the tokens into a tree
 * 
 * @param tokens tokens to parse
 * @param errors output for errors
 * @return ParserTree parsed tokens
 */
ParserTree parse(List tokens, List* errors);

#endif // PARSER_INCLUDED