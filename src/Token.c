#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void tokenFree(Token token)
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

Token tokenFileSpan(T_TokenType type, FileSpan span)
{
    return tokenString(type, span.str, span.line, span.col);
}

Token tokenCreate(T_TokenType type, size_t line, size_t col)
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

Token tokenString(T_TokenType type, char *string, size_t line, size_t col)
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

Token tokenInteger(T_TokenType type, long long integer, size_t line, size_t col)
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

Token tokenDecimal(T_TokenType type, double decimal, size_t line, size_t col)
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

Token tokenCharacter(T_TokenType type, char character, size_t line, size_t col)
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

Token tokenFileSpanPos(T_TokenType type, FileSpan span)
{
    return tokenCreate(type, span.line, span.col);
}

Token tokenFileSpanInt(T_TokenType type, long long value, FileSpan span)
{
    return tokenInteger(type, value, span.line, span.col);
}

Token tokenFileSpanDec(T_TokenType type, double value, FileSpan span)
{
    return tokenDecimal(type, value, span.line, span.col);
}

Token tokenFileSpanChar(T_TokenType type, char value, FileSpan span)
{
    return tokenCharacter(type, value, span.line, span.col);
}

Token tokenFileSpanBool(T_TokenType type, _Bool value, FileSpan span)
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

Token tokenFileSpanPart(T_TokenType type, FileSpan span, size_t start, size_t length)
{
    assert(span.str);
    assert(start + length <= span.length);

    char* str = malloc((length + 1) * sizeof(char));
    
    assert(str);
    
    str[length] = 0;
    strncpy_s(str, length + 1, span.str + start, length);

    return tokenString(type, str, span.line, span.col);
}

void tokenPrint(FILE* out, Token token)
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
        fprintf(out, "[(%zu)", token.integer);
        return;
    case T_PUNCTUATION_BRACKET_CLOSE:
        fprintf(out, "](%zu)", token.integer);
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
        fprintf(out, "integer(%zu)", token.integer);
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

void tokenPrintPos(FILE* out, Token token, const char* filename)
{
    fprintf(out, "%s:%zu:%zu: ", filename, token.line, token.col);
    tokenPrint(out, token);
}