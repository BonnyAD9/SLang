#include "Token.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "FileSpan.h"
#include "FilePos.h"
#include "String.h"
#include "Stream.h"

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

void tokenPrint(Stream* out, Token token)
{
    fpPrint(out, token.pos);
    stPrintf(out, ": ");
    switch (token.type)
    {
    case T_UNDEFINED:
        stPrintf(out, "undefined\n");
        return;
    case T_INVALID:
        stPrintf(out, "invalid(%s)\n", token.string.c);
        return;
    case T_COMMENT_LINE:
        stPrintf(out, "lineComment(%s)\n", token.string.c);
        return;
    case T_COMMENT_BLOCK:
        stPrintf(out, "blockComment(%s)\n", token.string.c);
        return;
    case T_PUNCTUATION_BRACKET_OPEN:
        stPrintf(out, "[(%zu)\n", token.integer);
        return;
    case T_PUNCTUATION_BRACKET_CLOSE:
        stPrintf(out, "](%zu)\n", token.integer);
        return;
    case T_IDENTIFIER_VARIABLE:
        stPrintf(out, "variable(%s)\n", token.string.c);
        return;
    case T_IDENTIFIER_FUNCTION:
        stPrintf(out, "function(%s)\n", token.string.c);
        return;
    case T_IDENTIFIER_STRUCT:
        stPrintf(out, "struct(%s)\n", token.string.c);
        return;
    case T_STORAGE_POINTER:
        stPrintf(out, "*\n");
        return;
    case T_STORAGE_CHAR:
        stPrintf(out, "char\n");
        return;
    case T_STORAGE_STRING:
        stPrintf(out, "string\n");
        return;
    case T_STORAGE_INT:
        stPrintf(out, "int\n");
        return;
    case T_STORAGE_FLOAT:
        stPrintf(out, "float\n");
        return;
    case T_STORAGE_BOOL:
        stPrintf(out, "bool\n");
        return;
    case T_IDENTIFIER_PARAMETER:
        stPrintf(out, "parameter(%s)\n", token.string.c);
        return;
    case T_LITERAL_INTEGER:
        stPrintf(out, "integer(%zu)\n", token.integer);
        return;
    case T_LITERAL_FLOAT:
        stPrintf(out, "float(%lf)\n", token.decimal);
        return;
    case T_LITERAL_CHAR:
        stPrintf(out, "char(%c)\n", token.character);
        return;
    case T_LITERAL_STRING:
        stPrintf(out, "string(%s)\n", token.string.c);
        return;
    case T_LITERAL_BOOL:
        stPrintf(out, "bool(%s)\n", token.boolean ? "true" : "false");
        return;
    case T_KEYWORD_DEF:
        stPrintf(out, "def\n");
        return;
    case T_KEYWORD_STRUCT:
        stPrintf(out, "struct\n");
        return;
    case T_KEYWORD_SET:
        stPrintf(out, "set\n");
        return;
    case T_KEYWORD_SIGN:
        stPrintf(out, "sign\n");
        return;
    case T_OPERATOR_NOTHING:
        stPrintf(out, "_\n");
        return;
    case T_ERROR:
        stPrintf(out, "error\n");
        return;
    default:
        stPrintf(out, "unknown\n");
        return;
    }
}