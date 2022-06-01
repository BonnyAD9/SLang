#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "FileSpan.h"

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
    size_t line;
    size_t col;
    union
    {
        char* string;
        long long integer;
        double decimal;
        char character;
        bool boolean;
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
Token fileSpanToken(T_TokenType type, FileSpan span);

/**
 * @brief creates token from file span with only part of the string
 * 
 * @param type type of the token
 * @param span span with the string and position in file
 * @param start index of first character to take
 * @param length total number of characters to take
 * @return Token bew instance
 */
Token fileSpanTokenPart(T_TokenType type, FileSpan span, size_t start, size_t length);

/**
 * @brief creates token from file span
 * 
 * @param type type of the token
 * @param span span with the position in file
 * @return Token new instance
 */
Token fileSpanTokenPos(T_TokenType type, FileSpan span);

/**
 * @brief creates token with bool value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with position in file
 * @return Token new instance
 */
Token fileSpanBoolToken(T_TokenType type, bool value, FileSpan span);

/**
 * @brief creates token with int value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanIntToken(T_TokenType type, long long value, FileSpan span);

/**
 * @brief creates token with decimal value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanDecToken(T_TokenType type, double value, FileSpan span);

/**
 * @brief creates token with char value
 * 
 * @param type type of the token
 * @param value value of the token
 * @param span span with the position in file
 * @return Token 
 */
Token fileSpanCharToken(T_TokenType type, char value, FileSpan span);

/**
 * @brief Create a Token object with no data
 * 
 * @param type type of the token
 * @param line line on which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token createToken(T_TokenType type, size_t line, size_t col);

/**
 * @brief Create a Token object with string data
 * 
 * @param type type of the token
 * @param string string data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token stringToken(T_TokenType type, char* string, size_t line, size_t col);

/**
 * @brief Create a Token object with integer data
 *
 * @param type type of the token
 * @param integer integer data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token integerToken(T_TokenType type, long long integer, size_t line, size_t col);

/**
 * @brief Create a Token object with decimal data
 *
 * @param type type of the token
 * @param decimal decimal data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token decimalToken(T_TokenType type, double decimal, size_t line, size_t col);

/**
 * @brief Create a Token object with character data
 *
 * @param type type of the token
 * @param character character data for the token
 * @param line line in which the token is
 * @param col collumn of the first character of the token
 * @return new Token
 */
Token characterToken(T_TokenType type, char character, size_t line, size_t col);

/**
 * @brief frees this token string (if it is that type)
 * 
 * @param token token to free
 */
void freeToken(Token token);

#endif // TOKEN_INCLUDED