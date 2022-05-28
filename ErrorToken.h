#ifndef ERROR_TOKEN_INCLUDED
#define ERROR_TOKEN_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#include "FileSpan.h"

typedef enum ErrorLevel
{
    ERROR,
    WARNING,
    INFO,
} ErrorLevel;

typedef struct ErrorToken
{
    ErrorLevel level;
    size_t line;
    size_t col;
    char* token;
    char* message;
    char* help;
} ErrorToken;

/**
 * @brief Create a Error Token object
 * 
 * @param level error level
 * @param span span with the token and position in file
 * @param message messege describing the error
 * @param help suggestion for fixing the error
 * @return ErrorToken 
 */
ErrorToken createErrorToken(ErrorLevel level, FileSpan span, const char* message, const char* help);

/**
 * @brief prints this error token into the given stream
 * 
 * @param out stream to print to
 * @param token token to print
 * @param filename file in which this token is
 */
void printErrorToken(FILE* out, ErrorToken token, const char* filename);

/**
 * @brief frees error token
 * 
 * @param error error token to free
 */
void freeErrorToken(ErrorToken error);

#endif // ERROR_TOKEN_INCLUDED