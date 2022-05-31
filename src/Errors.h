#ifndef ERROR_TOKEN_INCLUDED
#define ERROR_TOKEN_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include "FileSpan.h"
#include "Token.h"

typedef enum ErrorLevel
{
    E_ERROR,
    E_WARNING,
    E_INFO,
} ErrorLevel;

typedef struct ErrorSpan
{
    ErrorLevel level;
    FileSpan span;
    char* message;
    char* help;
} ErrorSpan;

typedef struct ErrorToken
{
    ErrorLevel level;
    Token token;
    char* message;
    char* help;
} ErrorToken;

/**
 * @brief Create a Error Span object
 * 
 * @param level error level
 * @param span span with the token and position in file
 * @param message messege describing the error
 * @param help suggestion for fixing the error
 * @return ErrorToken 
 */
ErrorSpan createErrorSpan(ErrorLevel level, FileSpan span, const char* message, const char* help);

/**
 * @brief prints this error span into the given stream
 * 
 * @param out stream to print to
 * @param error error to print
 * @param filename file in which this span is
 */
void printErrorSpan(FILE* out, ErrorSpan error, const char* filename);

/**
 * @brief frees error span
 * 
 * @param error error span to free
 */
void freeErrorSpan(ErrorSpan error);

/**
 * @brief Create a Error Token object
 * 
 * @param level error level
 * @param token problematic token
 * @param message message describing the error
 * @param help suggestion for fixing the error
 * @return ErrorToken new instance
 */
ErrorToken createErrorToken(ErrorLevel level, Token token, const char* message, const char* help);

/**
 * @brief prints the given error to the stream
 * 
 * @param out where to print
 * @param error error to print
 * @param filename file in which the token is
 */
void printErrorToken(FILE* out, ErrorToken error, const char* filename);

/**
 * @brief frees the given error token
 * 
 * @param error error token to free
 */
void freeErrorToken(ErrorToken error);

#endif // ERROR_TOKEN_INCLUDED