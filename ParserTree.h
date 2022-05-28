#ifndef PARSER_TREE_INCLUDED
#define PARSER_TREE_INCLUDED

#include <stdlib.h>

typedef enum ParserNodeType
{
    DEFINED_FUNCTION,
    DEFINED_VARIABLE,
    STORAGE_POINTER,
    STORAGE_CHAR,
    STORAGE_STRING,
    STORAGE_INT,
    STORAGE_FLOAT,
    FUNCTION_DEFINITION,
    FUNCTION_INVOCATION,
    VARIABLE_SETTER,
    FUNCTION_SETTER,
    VARIABLE,
    PARAMETER,
} ParserNodeType;

typedef struct ParserNode
{
    ParserNode* nodes;
    size_t count;
    ParserNodeType type;
    size_t line;
    size_t col;
    char* str;
} ParserNode;

typedef struct ParserTree
{
    ParserNode* nodes;
    size_t count;
    char* filename;
} ParserTree;

#endif // PARSER_TREE_INCLUDED