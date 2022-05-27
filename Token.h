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
    COMMENT_LINE, // hello
    COMMENT_BLOCK, // /* hello */
    PUNCTUATION_BRACKET_OPEN, // [
    PUNCTUATION_BRACKET_CLOSE, // ]
    IDENTIFIER_VARIABLE, // i
    IDENTIFIER_FUNCTION, // +
    IDENTIFIER_STRUCT, // list
    LITERAL_INTEGER, // 123
    LITERAL_FLOAT, // 123.0
    LITERAL_CHAR, // 'C'
    LITERAL_STRING, // "hello"
    KEYWORD_DEF, // def
    KEYWORD_STRUCT, // struct
    KEYWORD_SET, // set
    KEYWORD_EXTERN, // extern
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
 * @param span span with the string an file position
 * @return Token new instance
 */
Token fileSpanToken(TokenType type, FileSpan span);

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