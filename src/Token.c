#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Assert.h"
#include "Terminal.h"

void freeToken(Token token)
{
    switch (token.type)
    {
    case T_COMMENT_LINE:
    case T_COMMENT_BLOCK:
    case T_IDENTIFIER_VARIABLE:
    case T_IDENTIFIER_FUNCTION:
    case T_IDENTIFIER_STRUCT:
    case T_LITERAL_STRING:
    case T_IDENTIFIER_PARAMETER:
    case T_INVALID:
        free(token.string);
        return;
    default:
        return;
    }
}

Token fileSpanToken(T_TokenType type, FileSpan span)
{
    return stringToken(type, span.str, span.line, span.col);
}

Token createToken(T_TokenType type, size_t line, size_t col)
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

Token stringToken(T_TokenType type, char *string, size_t line, size_t col)
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

Token integerToken(T_TokenType type, long long integer, size_t line, size_t col)
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

Token decimalToken(T_TokenType type, double decimal, size_t line, size_t col)
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

Token characterToken(T_TokenType type, char character, size_t line, size_t col)
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

Token fileSpanTokenPos(T_TokenType type, FileSpan span)
{
    return createToken(type, span.line, span.col);
}

Token fileSpanIntToken(T_TokenType type, long long value, FileSpan span)
{
    return integerToken(type, value, span.line, span.col);
}

Token fileSpanDecToken(T_TokenType type, double value, FileSpan span)
{
    return decimalToken(type, value, span.line, span.col);
}

Token fileSpanCharToken(T_TokenType type, char value, FileSpan span)
{
    return characterToken(type, value, span.line, span.col);
}

Token fileSpanBoolToken(T_TokenType type, bool value, FileSpan span)
{
    Token t =
    {
        .type = type,
        .line = span.line,
        .col = span.col,
        .boolean = value,
    };
    return t;
}

Token fileSpanTokenPart(T_TokenType type, FileSpan span, size_t start, size_t length)
{
    assert(span.str, "fileSpanTokenPart: given span has no string");
    assert(start + length <= span.length, "fileSpanTokenPart: geven span of the span is outside of range (max: %"term_SIZE_T", is: %"term_SIZE_T")", span.length, start + length);

    char* str = malloc((length + 1) * sizeof(char));
    
    assert(str, "fileSpanTokenPart: failed to allocate string of size %"term_SIZE_T, length + 1);
    
    str[length] = 0;
    strncpy_s(str, length + 1, span.str + start, length);

    return stringToken(type, str, span.line, span.col);
}

void printToken(FILE* out, Token token)
{
    switch (token.type)
    {
    case T_UNDEFINED:
        fprintf(out, "undefined");
        return;
    case T_INVALID:
        fprintf(out, "invalid(%s)", token.string);
        return;
    case T_COMMENT_LINE:
        fprintf(out, "lineComment(%s)", token.string);
        return;
    case T_COMMENT_BLOCK:
        fprintf(out, "blockComment(%s)", token.string);
        return;
    case T_PUNCTUATION_BRACKET_OPEN:
        fprintf(out, "[(%"term_SIZE_T")", token.integer);
        return;
    case T_PUNCTUATION_BRACKET_CLOSE:
        fprintf(out, "](%"term_SIZE_T")", token.integer);
        return;
    case T_IDENTIFIER_VARIABLE:
        fprintf(out, "variable(%s)", token.string);
        return;
    case T_IDENTIFIER_FUNCTION:
        fprintf(out, "function(%s)", token.string);
        return;
    case T_IDENTIFIER_STRUCT:
        fprintf(out, "struct(%s)", token.string);
        return;
    case T_STORAGE_POINTER:
        fprintf(out, "*");
        return;
    case T_STORAGE_CHAR:
        fprintf(out, "char");
        return;
    case T_STORAGE_STRING:
        fprintf(out, "string");
        return;
    case T_STORAGE_INT:
        fprintf(out, "int");
        return;
    case T_STORAGE_FLOAT:
        fprintf(out, "float");
        return;
    case T_STORAGE_BOOL:
        fprintf(out, "bool");
        return;
    case T_IDENTIFIER_PARAMETER:
        fprintf(out, "parameter(%s)", token.string);
        return;
    case T_LITERAL_INTEGER:
        fprintf(out, "integer(%"term_SIZE_T")", token.integer);
        return;
    case T_LITERAL_FLOAT:
        fprintf(out, "float(%lf)", token.decimal);
        return;
    case T_LITERAL_CHAR:
        fprintf(out, "char(%c)", token.character);
        return;
    case T_LITERAL_STRING:
        fprintf(out, "string(%s)", token.string);
        return;
    case T_LITERAL_BOOL:
        fprintf(out, "bool(%s)", token.boolean ? "true" : "false");
        return;
    case T_KEYWORD_DEF:
        fprintf(out, "def");
        return;
    case T_KEYWORD_STRUCT:
        fprintf(out, "struct");
        return;
    case T_KEYWORD_SET:
        fprintf(out, "set");
        return;
    case T_KEYWORD_SIGN:
        fprintf(out, "sign");
        return;
    case T_OPERATOR_NOTHING:
        fprintf(out, "_");
        return;
    case T_ERROR:
        fprintf(out, "error");
        return;
    default:
        fprintf(out, "unknown");
        return;
    }
}

void printTokenPos(FILE* out, Token token, const char* filename)
{
    fprintf(out, "%s:%"term_SIZE_T":%"term_SIZE_T": ", filename, token.line, token.col);
    printToken(out, token);
}