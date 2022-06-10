#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

#include <stdio.h>
#include <stdint.h>

#include "FileSpan.h"
#include "FilePos.h"
#include "String.h"
#include "Stream.h"

/**
 * @brief represents type of token
 * 
 */
typedef enum T_TokenType
{
    T_UNDEFINED = 0,
    T_INVALID,
    T_COMMENT_LINE, // // hello
    T_COMMENT_BLOCK, // /* hello */
    T_PUNCTUATION_BRACKET_OPEN, // [
    T_PUNCTUATION_BRACKET_CLOSE, // ]
    T_IDENTIFIER_VARIABLE, // i
    T_IDENTIFIER_FUNCTION, // +
    T_IDENTIFIER_STRUCT, // list
    T_STORAGE_POINTER, // *
    T_STORAGE_CHAR, // char
    T_STORAGE_STRING, // string
    T_STORAGE_INT, // int
    T_STORAGE_FLOAT, // float
    T_STORAGE_BOOL, // bool
    T_IDENTIFIER_PARAMETER, // value
    T_LITERAL_INTEGER, // 123
    T_LITERAL_FLOAT, // 123.0
    T_LITERAL_CHAR, // 'C'
    T_LITERAL_STRING, // "hello"
    T_LITERAL_BOOL, // true false
    T_KEYWORD_DEF, // def
    T_KEYWORD_STRUCT, // struct
    T_KEYWORD_SET, // set
    T_KEYWORD_SIGN, // sign
    T_OPERATOR_NOTHING, // _
    T_ERROR,
} T_TokenType;

/**
 * @brief Represents lexical token
 * 
 */
typedef struct Token
{
    T_TokenType type;
    FilePos pos;
    union
    {
        String string;
        intmax_t integer;
        double decimal;
        char character;
        _Bool boolean;
    };
} Token;

/**
 * @brief prints representation of this token into the given stream
 * 
 * @param out stream to print to
 * @param token token which will be printed
 */
void tokenPrint(Stream* out, Token token);

/**
 * @brief creates token from file span
 * 
 * @param type type of the token
 * @param span span with the string and position in file
 * @return Token new instance
 */
Token tokenFileSpan(T_TokenType type, FileSpan span);

/**
 * @brief creates token from file span with only part of the string
 * 
 * @param type type of the token
 * @param span span with the string and position in file
 * @param start index of first character to take
 * @param length total number of characters to take
 * @return Token bew instance
 */
Token tokenFileSpanPart(T_TokenType type, FileSpan span, size_t start, size_t length);

/**
 * @brief Create a Token object with no data
 * 
 * @param type type of the token
 * @param pos position in file
 * @return new Token
 */
Token tokenCreate(T_TokenType type, FilePos pos);

/**
 * @brief Create a Token object with string data
 * 
 * @param type type of the token
 * @param string string data for the token
 * @param pos position in file
 * @return new Token
 */
Token tokenStr(T_TokenType type, String string, FilePos pos);

/**
 * @brief Create a Token object with integer data
 *
 * @param type type of the token
 * @param integer integer data for the token
 * @param pos position in file
 * @return new Token
 */
Token tokenInt(T_TokenType type, intmax_t integer, FilePos pos);

/**
 * @brief Create a Token object with decimal data
 *
 * @param type type of the token
 * @param decimal decimal data for the token
 * @param pos position in file
 * @return new Token
 */
Token tokenFloat(T_TokenType type, double decimal, FilePos pos);

/**
 * @brief Create a Token object with character data
 *
 * @param type type of the token
 * @param character character data for the token
 * @param pos position in file
 * @return new Token
 */
Token tokenChar(T_TokenType type, char character, FilePos pos);

/**
 * @brief creates new token with bool value
 * 
 * @param type token type
 * @param boolean value of token
 * @param pos position in file
 * @return Token new instance
 */
Token tokenBool(T_TokenType type, _Bool boolean, FilePos pos);

/**
 * @brief frees this token string (if it is that type)
 * 
 * @param token token to free
 */
void tokenFree(Token token);

#endif // TOKEN_INCLUDED