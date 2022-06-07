#include "ParserTree.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "Token.h"
#include "List.h"

/**
 * @brief prints the given parser node
 * 
 * @param out where to print
 * @param node node to print
 * @param depth depth of the node
 */
void _ptPrintNode(FILE* out, ParserNode node, size_t depth, const char* filename);

ParserTree ptCreate()
{
    ParserTree tree =
    {
        .nodes = listNew(ParserNode),
        .filename = NULL,
    };
    return tree;
}

void ptPrint(FILE *out, ParserTree tree)
{
    fprintf(out, "%s\n", tree.filename);
    listForEach(tree.nodes, ParserNode, n,
        _ptPrintNode(out, n, 1, tree.filename);
    );
}

void _ptPrintNode(FILE *out, ParserNode node, size_t depth, const char* filename)
{
    for (size_t i = 0; i < depth; i++)
        fprintf(out, "\x1b[9%zum|", i % 8 + 1);

    switch (node.type)
    {
    case P_FUNCTION_CALL:
        fprintf(out, "FUNCTION_CALL\n");
        break;
    case P_IDENTIFIER:
        fprintf(out, "IDENTIFIER(");
        tokenPrint(out, *node.token);
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
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_VALUE_FLOAT:
        fprintf(out, "VALUE_FLOAT(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_VALUE_CHAR:
        fprintf(out, "VALUE_CHAR(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_VALUE_STRING:
        fprintf(out, "VALUE_STRING(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_VALUE_BOOL:
        fprintf(out, "VALUE_BOOL(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_FUNCTION_DEFINITION:
        fprintf(out, "FUNCTION_DEFINITION\n");
        break;
    case P_VARIABLE_SETTER:
        fprintf(out, "VARIABLE_SETTER(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    case P_FUNCTION_SETTER:
        fprintf(out, "FUNCTION_SETTER(");
        tokenPrint(out, *node.token);
        fprintf(out, ")\n");
        break;
    default:
        fprintf(out, "OTHER\n");
        break;
    }

    listForEach(node.nodes, ParserNode, n,
        _ptPrintNode(out, n, depth + 1, filename);
    );
    fprintf(out, "\x1b[0m");
}

void ptFree(ParserTree tree)
{
    listDeepFree(tree.nodes, ParserNode, n, ptFreeNode(n, true));
}

ParserNode ptTokenNode(ParserNodeType type, Token token)
{
    ParserNode node =
    {
        .nodes = listNew(ParserNode),
        .type = type,
        .token = malloc(sizeof(Token))
    };
    assert(node.token);
    *node.token = token;
    return node;
}

ParserNode ptCreateNode(ParserNodeType type)
{
    ParserNode node =
    {
        .nodes = listNew(ParserNode),
        .type = type,
        .token = NULL,
    };
    return node;
}

void ptFreeNode(ParserNode node, bool recursive)
{
    listDeepFree(node.nodes, ParserNode, n, ptFreeNode(n, true));
    if (node.token)
        tokenFree(*node.token);
}

void ptAdd(ParserTree* tree, ParserNode node)
{
    listAdd(tree->nodes, node, ParserNode);
}

void ptNodeAdd(ParserNode* node, ParserNode n)
{
    listAdd(node->nodes, n, ParserNode);
}