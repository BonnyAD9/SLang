#include "ParserTree.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "Token.h"
#include "Assert.h"
#include "List.h"

/**
 * @brief prints the given parser node
 * 
 * @param out where to print
 * @param node node to print
 * @param depth depth of the node
 */
void _printParserNode(FILE* out, ParserNode node, size_t depth, const char* filename);

ParserTree createParserTree()
{
    ParserTree tree =
    {
        .nodes = newList(ParserNode),
        .filename = NULL,
    };
    return tree;
}

void printParserTree(FILE *out, ParserTree tree)
{
    fprintf(out, "%s\n", tree.filename);
    listForEach(tree.nodes, ParserNode, n,
        _printParserNode(out, n, 1, tree.filename);
    );
}

void _printParserNode(FILE *out, ParserNode node, size_t depth, const char* filename)
{
    for (size_t i = 0; i < depth; i++)
        fprintf(out, "\x1b[9%I64dm|", i % 8 + 1);

    switch (node.type)
    {
    case P_FUNCTION_CALL:
        fprintf(out, "FUNCTION_CALL\n");
        break;
    case P_IDENTIFIER:
        fprintf(out, "IDENTIFIER(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_NOTHING:
        fprintf(out, "NOTHING\n");
        break;
    case P_ERROR:
        fprintf(out, "ERROR\n");
        break;
    case P_VALUE_INTEGER:
        fprintf(out, "VALUE_INTEGER(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_VALUE_FLOAT:
        fprintf(out, "VALUE_FLOAT(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_VALUE_CHAR:
        fprintf(out, "VALUE_CHAR(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_VALUE_STRING:
        fprintf(out, "VALUE_STRING(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_FUNCTION_DEFINITION:
        fprintf(out, "FUNCTION_DEFINITION\n");
        break;
    case P_VARIABLE_SETTER:
        fprintf(out, "VARIABLE_SETTER(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    case P_FUNCTION_SETTER:
        fprintf(out, "FUNCTION_SETTER(");
        printTokenPos(out, *node.token, filename);
        fprintf(out, ")\n");
        break;
    default:
        fprintf(out, "OTHER\n");
        break;
    }

    listForEach(node.nodes, ParserNode, n,
        _printParserNode(out, n, depth + 1, filename);
    );
    fprintf(out, "\x1b[0m");
}

void freeParserTree(ParserTree tree)
{
    listDeepFree(tree.nodes, ParserNode, n, freeParserNode(n, true));
}

ParserNode tokenParserNode(ParserNodeType type, Token token)
{
    ParserNode node =
    {
        .nodes = newList(ParserNode),
        .type = type,
        .token = malloc(sizeof(Token))
    };
    assert(node.token, "tokenParserNode: failed to allocate token");
    *node.token = token;
    return node;
}

ParserNode createParserNode(ParserNodeType type)
{
    ParserNode node =
    {
        .nodes = newList(ParserNode),
        .type = type,
        .token = NULL,
    };
    return node;
}

void freeParserNode(ParserNode node, bool recursive)
{
    listDeepFree(node.nodes, ParserNode, n, freeParserNode(n, true));
    if (node.token)
        freeToken(*node.token);
}

void parserTreeAdd(ParserTree* tree, ParserNode node)
{
    listAdd(tree->nodes, node, ParserNode);
}

void parserNodeAdd(ParserNode* node, ParserNode n)
{
    listAdd(node->nodes, n, ParserNode);
}