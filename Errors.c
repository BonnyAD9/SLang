#include "Errors.h"

#include <string.h>

#include "FileSpan.h"
#include "Assert.h"
#include "Token.h"

/**
 * @brief creates copy of the given string
 * 
 * @param str string to copy
 * @return char* copy of the string
 */
char* _copyString(const char* str);

ErrorSpan createErrorSpan(ErrorLevel level, FileSpan span, const char* message, const char* help)
{
    ErrorSpan error =
    {
        .level = level,
        .span = span,
        .message = _copyString(message),
        .help = _copyString(help),
    };
    return error;
}

void printErrorSpan(FILE* out, ErrorSpan token, const char* filename)
{
    const char* msgType;
    const char* msgColor;
    switch (token.level)
    {
    case E_INFO:
        msgType = "info:";
        msgColor = "\x1b[94m";
        break;
    case E_WARNING:
        msgType = "warning:";
        msgColor = "\x1b[95m";
        break;
    case E_ERROR:
        msgType = "error:";
        msgColor = "\x1b[91m";
        break;
    default:
        msgType = "message:";
        msgColor = "\x1b[93m";
        break;
    }

    fprintf(out, "%s:%I64d:%I64d:\t%s%s\x1b[0m %s\n\t%s%s\x1b[0m\n\t\x1b[92mhelp:\x1b[0m %s",
            filename, token.span.line, token.span.col,
            msgColor, msgType, token.message,
            msgColor, token.span.str,
            token.help);
}

void freeErrorSpan(ErrorSpan error)
{
    freeFileSpan(error.span);
    free(error.message);
    free(error.help);
}

ErrorToken createErrorToken(ErrorLevel level, Token token, const char* message, const char* help)
{
    ErrorToken t =
    {
        .level = level,
        .token = token,
        .message = _copyString(message),
        .help = _copyString(help),
    };
    return t;
}

void printErrorToken(FILE* out, ErrorToken error, const char* filename)
{
    const char *msgType;
    const char *msgColor;
    switch (error.level)
    {
    case E_INFO:
        msgType = "info:";
        msgColor = "\x1b[94m";
        break;
    case E_WARNING:
        msgType = "warning:";
        msgColor = "\x1b[95m";
        break;
    case E_ERROR:
        msgType = "error:";
        msgColor = "\x1b[91m";
        break;
    default:
        msgType = "message:";
        msgColor = "\x1b[93m";
        break;
    }

    fprintf(out, "%s:%I64d:%I64d:\t%s%s\x1b[0m %s\n\t%s",
            filename, error.token.line, error.token.col,
            msgColor, msgType, error.message,
            msgColor);
    printToken(out, error.token);
    fprintf(out, "\x1b[0m\n\t\x1b[92mhelp: \x1b[0m %s", error.help);
}

void freeErrorToken(ErrorToken error)
{
    freeToken(error.token);
    free(error.message);
    free(error.help);
}

char* _copyString(const char* str)
{
    size_t len = strlen(str);
    char *cpy = malloc((len + 1) * sizeof(char));
    assert(cpy, "_copyString: failed to allocate string of size %I64d", len + 1);
    return cpy;
}