#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "FileSpan.h"
#include "FilePos.h"
#include "String.h"

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
        strFree(token.string);
        return;
    default:
        return;
    }
}

Token tokenFileSpan(T_TokenType type, FileSpan span)
{
    return tokenStr(type, span.str, span.pos);
}

Token tokenCreate(T_TokenType type, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .integer = 0,
    };
    return t;
}

Token tokenStr(T_TokenType type, String string, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .string = string,
    };
    return t;
}

Token tokenInt(T_TokenType type, intmax_t integer, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .integer = integer,
    };
    return t;
}

Token tokenFloat(T_TokenType type, double decimal, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .decimal = decimal,
    };
    return t;
}

Token tokenChar(T_TokenType type, char character, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .character = character,
    };
    return t;
}

Token tokenBool(T_TokenType type, _Bool boolean, FilePos pos)
{
    Token t =
    {
        .type = type,
        .pos = pos,
        .boolean = boolean,
    };
    return t;
}

Token tokenFileSpanPart(T_TokenType type, FileSpan span, size_t start, size_t length)
{
    assert(span.str.c);
    assert(start + length <= span.str.length);

    return tokenStr(type, strCLen(span.str.c + start, length), span.pos);
}

void tokenPrint(FILE* out, Token token)
{
    fpPrint(out, token.pos);
    fprintf(out, ": ");
    switch (token.type)
    {
    case T_UNDEFINED:
        fprintf(out, "undefined\n");
        return;
    case T_INVALID:
        fprintf(out, "invalid(%s)\n", token.string.c);
        return;
    case T_COMMENT_LINE:
        fprintf(out, "lineComment(%s)\n", token.string.c);
        return;
    case T_COMMENT_BLOCK:
        fprintf(out, "blockComment(%s)\n", token.string.c);
        return;
    case T_PUNCTUATION_BRACKET_OPEN:
        fprintf(out, "[(%zu)\n", token.integer);
        return;
    case T_PUNCTUATION_BRACKET_CLOSE:
        fprintf(out, "](%zu)\n", token.integer);
        return;
    case T_IDENTIFIER_VARIABLE:
        fprintf(out, "variable(%s)\n", token.string.c);
        return;
    case T_IDENTIFIER_FUNCTION:
        fprintf(out, "function(%s)\n", token.string.c);
        return;
    case T_IDENTIFIER_STRUCT:
        fprintf(out, "struct(%s)\n", token.string.c);
        return;
    case T_STORAGE_POINTER:
        fprintf(out, "*\n");
        return;
    case T_STORAGE_CHAR:
        fprintf(out, "char\n");
        return;
    case T_STORAGE_STRING:
        fprintf(out, "string\n");
        return;
    case T_STORAGE_INT:
        fprintf(out, "int\n");
        return;
    case T_STORAGE_FLOAT:
        fprintf(out, "float\n");
        return;
    case T_STORAGE_BOOL:
        fprintf(out, "bool\n");
        return;
    case T_IDENTIFIER_PARAMETER:
        fprintf(out, "parameter(%s)\n", token.string.c);
        return;
    case T_LITERAL_INTEGER:
        fprintf(out, "integer(%zu)\n", token.integer);
        return;
    case T_LITERAL_FLOAT:
        fprintf(out, "float(%lf)\n", token.decimal);
        return;
    case T_LITERAL_CHAR:
        fprintf(out, "char(%c)\n", token.character);
        return;
    case T_LITERAL_STRING:
        fprintf(out, "string(%s)\n", token.string.c);
        return;
    case T_LITERAL_BOOL:
        fprintf(out, "bool(%s)\n", token.boolean ? "true" : "false");
        return;
    case T_KEYWORD_DEF:
        fprintf(out, "def\n");
        return;
    case T_KEYWORD_STRUCT:
        fprintf(out, "struct\n");
        return;
    case T_KEYWORD_SET:
        fprintf(out, "set\n");
        return;
    case T_KEYWORD_SIGN:
        fprintf(out, "sign\n");
        return;
    case T_OPERATOR_NOTHING:
        fprintf(out, "_\n");
        return;
    case T_ERROR:
        fprintf(out, "error\n");
        return;
    default:
        fprintf(out, "unknown\n");
        return;
    }
}