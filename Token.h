#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "FileSpan.h"

/**
 * @brief represents type of token
 * 
 */
typedef enum TokenType
{
    UNDEFINED = 0,
    INVALID,
    COMMENT_LINE, // // hello
    COMMENT_BLOCK, // /* hello */
    PUNCTUATION_BRACKET_OPEN, // [
    PUNCTUATION_BRACKET_CLOSE, // ]
    IDENTIFIER_VARIABLE, // i
    IDENTIFIER_FUNCTION, // +
    IDENTIFIER_STRUCT, // list
    IDENTIFIER_PARAMETER, // value
    LITERAL_INTEGER, // 123
    LITERAL_FLOAT, // 123.0
    LITERAL_CHAR, // 'C'
    LITERAL_STRING, // "hello"
    KEYWORD_DEF, // def
    KEYWORD_STRUCT, // struct
    KEYWORD_SET, // set
    KEYWORD_DEFINED, // defined
    OPERATOR_NOTHING, // _
} TokenType;

/**
 * @brief Represents lexical token
 * 
 */
typedef struct Token
{
    TokenType type;
    size_t line;
    size_t col;
    union
    {
        char* string;
        long long integer;
        double decimal;
        char character;
    };
} Token;

/**
 * @brief prints representation of this token into the given stream
 * 
 * @param out stream to print to
 * @param token token which will be printed
 */
void printToken(FILE* out, Token token);

/**
 * @brief prints token with its position
 * 
 * @param out where to print
 * @param token token to print
 * @param filename file in which the token is
 */
void printTokenPos(FILE* out, Token token, const char* filename);

/**
 * @brief creates token from file span
 * 
 * @param type type of the token
 * @param span span with the string and position in file
 * @return Token new instance
 */
Token fileSpanToken(TokenType type, FileSpan span);

/**
 * @brief creates token from file span with only part of the string
 * 
 * @param type type of the token
 * @param span span with the string and position in file
 * @param start index of first character to take
 * @param length total number of characters to take
 * @return Token bew instance
 */
Token fileSpanTokenPart(TokenType type, FileSpan span, size_t start, size_t length);

/**
 * @brief creates token from file span
 * 
 * @param type type of the token
 * @param span span with the position in file
 * @return Token new instance
 */
Token fileSpanTokenPos(TokenType type, FileSpan span);

/**
 * @brief creates token with int value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanIntToken(TokenType type, long long value, FileSpan span);

/**
 * @brief creates token with decimal value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanDecToken(TokenType type, double value, FileSpan span);

/**
 * @brief creates token with char value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanCharToken(TokenType type, char value, FileSpan span);

/**
 * @brief Create a Token object with no data
 * 
 * @param type type of the token
 * @param line line on which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token createToken(TokenType type, size_t line, size_t col);

/**
 * @brief Create a Token object with string data
 * 
 * @param type type of the token
 * @param string string data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token stringToken(TokenType type, char* string, size_t line, size_t col);

/**
 * @brief Create a Token object with integer data
 *
 * @param type type of the token
 * @param integer integer data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token integerToken(TokenType type, long long integer, size_t line, size_t col);

/**
 * @brief Create a Token object with decimal data
 *
 * @param type type of the token
 * @param decimal decimal data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token decimalToken(TokenType type, double decimal, size_t line, size_t col);

/**
 * @brief Create a Token object with character data
 *
 * @param type type of the token
 * @param character character data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token characterToken(TokenType type, char character, size_t line, size_t col);

/**
 * @brief frees this token string (if it is that type)
 * 
 * @param token token to free
 */
void freeToken(Token token);

#endif // TOKEN_INCLUDED