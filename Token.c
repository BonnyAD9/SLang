#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Assert.h"

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

Token fileSpanTokenPos(TokenType type, FileSpan span)
{
    return createToken(type, span.line, span.col);
}

Token fileSpanIntToken(TokenType type, long long value, FileSpan span)
{
    return integerToken(type, value, span.line, span.col);
}

Token fileSpanDecToken(TokenType type, double value, FileSpan span)
{
    return decimalToken(type, value, span.line, span.col);
}

Token fileSpanCharToken(TokenType type, char value, FileSpan span)
{
    return characterToken(type, value, span.line, span.col);
}

Token fileSpanTokenPart(TokenType type, FileSpan span, size_t start, size_t length)
{
    assert(span.str, "fileSpanTokenPart: given span has no string");
    assert(start + length <= span.length, "fileSpanTokenPart: geven span of the span is outside of range (max: %I64d, is: %I64d)", span.length, start + length);

    char* str = malloc((length + 1) * sizeof(char));
    
    assert(str, "fileSpanTokenPart: failed to allocate string of size %I64d", length + 1);
    
    str[length] = 0;
    strncpy(str, span.str + start, length);

    return stringToken(type, str, span.line, span.col);
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
        fprintf(out, "[(%I64d)", token.integer);
        return;
    case PUNCTUATION_BRACKET_CLOSE:
        fprintf(out, "](%I64d)", token.integer);
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
    case IDENTIFIER_PARAMETER:
        fprintf(out, "parameter(%s)", token.string);
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
    case KEYWORD_DEFINED:
        fprintf(out, "defined");
        return;
    case OPERATOR_NOTHING:
        fprintf(out, "_");
        return;
    case STORAGE_INTEGER:
        fprintf(out, "int");
        return;
    case STORAGE_FLOAT:
        fprintf(out, "float");
        return;
    case STORAGE_STRING:
        fprintf(out, "string");
        return;
    case STORAGE_CHAR:
        fprintf(out, "char");
        return;
    case MODIFIER_POINTER:
        fprintf(out, "*");
        return;
    default:
        fprintf(out, "unknown");
        return;
    }
}

void printTokenPos(FILE* out, Token token, const char* filename)
{
    fprintf(out, "%s:%I64d:%I64d:\t", filename, token.line, token.col);
    printToken(out, token);
}