#ifndef err_ERRORS_INCLUDED
#define err_ERRORS_INCLUDED

#include <stdlib.h>

#include "FileSpan.h"
#include "Token.h"
#include "Stream.h"

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
    String message;
    String help;
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
ErrorSpan errCreateErrorSpan(ErrorLevel level, FileSpan span, String message, String help);

/**
 * @brief prints this error span into the given stream
 * 
 * @param out stream to print to
 * @param error error to print
 * @param filename file in which this span is
 */
void errPrintErrorSpan(Stream* out, ErrorSpan error);

/**
 * @brief frees error span
 * 
 * @param error error span to free
 */
void errFreeErrorSpan(ErrorSpan error);

/**
 * @brief Create a Error Token object
 * 
 * @param level error level
 * @param token problematic token
 * @param message message describing the error
 * @param help suggestion for fixing the error
 * @return ErrorToken new instance
 */
ErrorToken errCreateErrorToken(ErrorLevel level, Token token, const char* message, const char* help);

/**
 * @brief prints the given error to the stream
 * 
 * @param out where to print
 * @param error error to print
 * @param filename file in which the token is
 */
void errPrintErrorToken(Stream* out, ErrorToken error, const char* filename);

/**
 * @brief frees the given error token
 * 
 * @param error error token to free
 */
void errFreeErrorToken(ErrorToken error);

#endif // err_ERRORS_INCLUDED