#ifndef PARSER_TREE_INCLUDED
#define PARSER_TREE_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "Token.h"
#include "List.h"

typedef enum ParserNodeType
{
    P_DEFINED_FUNCTION,
    P_DEFINED_VARIABLE,
    P_STORAGE_POINTER,
    P_STORAGE_CHAR,
    P_STORAGE_STRING,
    P_STORAGE_INT,
    P_STORAGE_FLOAT,
    P_VALUE_INTEGER,
    P_VALUE_FLOAT,
    P_VALUE_CHAR,
    P_VALUE_STRING,
    P_IDENTIFIER,
    P_FUNCTION_DEFINITION,
    P_FUNCTION_CALL,
    P_VARIABLE_SETTER,
    P_FUNCTION_SETTER,
    P_VARIABLE,
    P_NOTHING,
    P_ERROR,
} ParserNodeType;

typedef struct ParserNode
{
    List nodes;
    ParserNodeType type;
    Token* token;
} ParserNode;

typedef struct ParserTree
{
    List nodes;
    const char* filename;
} ParserTree;

/**
 * @brief prints parser tree
 * 
 * @param out where to print
 * @param tree tree to print
 */
void printParserTree(FILE* out, ParserTree tree);

/**
 * @brief Create a Parser Tree object
 * 
 * @return ParserTree new instance
 */
ParserTree createParserTree();

/**
 * @brief frees the parser tree with all of its nodes
 * 
 * @param tree tree to free
 */
void freeParserTree(ParserTree tree);

/**
 * @brief Create a Parser Node object
 * 
 * @param type type of the node
 * @param token token of the node
 * @return ParserNode new instance
 */
ParserNode tokenParserNode(ParserNodeType type, Token token);

/**
 * @brief Create a Parser Node object
 * 
 * @param type type of the parser node
 * @return ParserNode new instance
 */
ParserNode createParserNode(ParserNodeType type);

/**
 * @brief frees the parser node
 * 
 * @param node node to free
 * @param recursive indicates whether to free all childs
 */
void freeParserNode(ParserNode node, bool recursive);

/**
 * @brief adds node to parser tree
 * 
 * @param tree where to add the parser node
 * @param node node to add
 */
void parserTreeAdd(ParserTree* tree, ParserNode node);

/**
 * @brief adds node to node
 * 
 * @param node where to add the node
 * @param n node to add
 */
void parserNodeAdd(ParserNode* node, ParserNode n);

#endif // PARSER_TREE_INCLUDED