#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

void freeToken(Token token)
{
    switch (token.type)
    {
    case COMMENT_LINE:
    case COMMENT_BLOCK:
    case IDENTIFIER_VARIABLE:
    case IDENTIFIER_FUNCTION:
    case IDENTIFIER_STRUCT:
    case LITERAL_STRING:
    case INVALID:
        free(token.string);
        return;
    default:
        return;
    }
}

Token fileSpanToken(TokenType type, FileSpan span)
{
    return stringToken(type, span.str, span.line, span.col);
}

Token createToken(TokenType type, size_t line, size_t col)
{
    Token t =
    {
        .type = type,
        .line = line,
        .col = col,
        .integer = 0,
    };
    return t;
}

Token stringToken(TokenType type, char *string, size_t line, size_t col)
{
    Token t =
    {
        .type = type,
        .line = line,
        .col = col,
        .string = string,
    };
    return t;
}

Token integerToken(TokenType type, long long integer, size_t line, size_t col)
{
    Token t =
    {
        .type = type,
        .line = line,
        .col = col,
        .integer = integer,
    };
    return t;
}

Token decimalToken(TokenType type, double decimal, size_t line, size_t col)
{
    Token t =
    {
        .type = type,
        .line = line,
        .col = col,
        .decimal = decimal,
    };
    return t;
}

Token characterToken(TokenType type, char character, size_t line, size_t col)
{
    Token t =
    {
        .type = type,
        .line = line,
        .col = col,
        .character = character,
    };
    return t;
}

void printToken(FILE* out, Token token)
{
    switch (token.type)
    {
    case UNDEFINED:
        fprintf(out, "undefined");
        return;
    case INVALID:
        fprintf(out, "invalid(%s)", token.string);
        return;
    case COMMENT_LINE:
        fprintf(out, "lineComment(%s)", token.string);
        return;
    case COMMENT_BLOCK:
        fprintf(out, "blockComment(%s)", token.string);
        return;
    case PUNCTUATION_BRACKET_OPEN:
        fprintf(out, "(");
        return;
    case PUNCTUATION_BRACKET_CLOSE:
        fprintf(out, ")");
        return;
    case IDENTIFIER_VARIABLE:
        fprintf(out, "variable(%s)", token.string);
        return;
    case IDENTIFIER_FUNCTION:
        fprintf(out, "function(%s)", token.string);
        return;
    case IDENTIFIER_STRUCT:
        fprintf(out, "struct(%s)", token.string);
        return;
    case LITERAL_INTEGER:
        fprintf(out, "integer(%I64d)", token.integer);
        return;
    case LITERAL_FLOAT:
        fprintf(out, "float(%lf)", token.decimal);
        return;
    case LITERAL_CHAR:
        fprintf(out, "char(%c)", token.character);
        return;
    case LITERAL_STRING:
        fprintf(out, "string(%s)", token.string);
        return;
    case KEYWORD_DEF:
        fprintf(out, "def");
        return;
    case KEYWORD_STRUCT:
        fprintf(out, "struct");
        return;
    case KEYWORD_SET:
        fprintf(out, "set");
        return;
    case KEYWORD_EXTERN:
        fprintf(out, "extern");
        return;
    case OPERATOR_NOTHING:
        fprintf(out, "_");
        return;
    default:
        fprintf(out, "unknown");
        return;
    }
}

void printTokenPos(FILE* out, Token token, const char* filename)
{
    fprintf(out, "%s:%I64d:%I64d: ", filename, token.line, token.col);
    printToken(out, token);
}