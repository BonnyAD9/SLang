#include "Errors.h"

#include <string.h>

#include "FileSpan.h"
#include "Assert.h"
#include "Token.h"
#include "Terminal.h"

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
        msgColor = term_BCYAN;
        break;
    case E_WARNING:
        msgType = "warning:";
        msgColor = term_BMAGENTA;
        break;
    case E_ERROR:
        msgType = "error:";
        msgColor = term_BRED;
        break;
    default:
        msgType = "message:";
        msgColor = term_BYELLOW;
        break;
    }

    fprintf(out, "%s:%"term_SIZE_T":%"term_SIZE_T":\t%s%s"term_COLRESET" %s\n\t%s%s"term_COLRESET"\n\t\x1b"term_BGREEN"help:\x1b[0m %s",
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
        msgColor = term_BCYAN;
        break;
    case E_WARNING:
        msgType = "warning:";
        msgColor = term_BMAGENTA;
        break;
    case E_ERROR:
        msgType = "error:";
        msgColor = term_BRED;
        break;
    default:
        msgType = "message:";
        msgColor = term_BYELLOW;
        break;
    }

    fprintf(out, "%s:%"term_SIZE_T":%"term_SIZE_T":\t%s%s"term_COLRESET" %s\n\t%s",
            filename, error.token.line, error.token.col,
            msgColor, msgType, error.message,
            msgColor);
    printToken(out, error.token);
    fprintf(out, term_COLRESET"\n\t"term_BGREEN"help: "term_COLRESET" %s", error.help);
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
    char* cpy = malloc((len + 1) * sizeof(char));
    strcpy(cpy, str);
    assert(cpy, "_copyString: failed to allocate string of size %"term_SIZE_T, len + 1);
    return cpy;
}