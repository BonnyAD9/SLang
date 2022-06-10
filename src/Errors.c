#include "Errors.h"

#include <string.h>
#include <assert.h>

#include "FileSpan.h"
#include "Token.h"
#include "Terminal.h"
#include "FilePos.h"
#include "Stream.h"

char* _errCopyString(const char* str);

ErrorSpan errCreateErrorSpan(ErrorLevel level, FileSpan span, String message, String help)
{
    ErrorSpan error =
    {
        .level = level,
        .span = span,
        .message = message,
        .help = help,
    };
    return error;
}

void errPrintErrorSpan(Stream* out, ErrorSpan token)
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

    fpPrint(out, token.span.pos);
    stPrintf(out, ":\t%s%s"term_COLRESET" %s\n\t%s%s"term_COLRESET"\n\t\x1b"term_BGREEN"help:\x1b[0m %s",
            msgColor, msgType, token.message.c,
            msgColor, token.span.str.c,
            token.help.c);
}

void errFreeErrorSpan(ErrorSpan error)
{
    fsFree(error.span);
    strFree(error.message);
    strFree(error.help);
}

ErrorToken errCreateErrorToken(ErrorLevel level, Token token, const char* message, const char* help)
{
    ErrorToken t =
    {
        .level = level,
        .token = token,
        .message = _errCopyString(message),
        .help = _errCopyString(help),
    };
    return t;
}

void errPrintErrorToken(Stream* out, ErrorToken error, const char* filename)
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

    fpPrint(out, error.token.pos);
    stPrintf(out, ":\t%s%s"term_COLRESET" %s\n\t%s",
            msgColor, msgType, error.message,
            msgColor);
    tokenPrint(out, error.token);
    stPrintf(out, term_COLRESET"\n\t"term_BGREEN"help: "term_COLRESET" %s", error.help);
}

void errFreeErrorToken(ErrorToken error)
{
    tokenFree(error.token);
    free(error.message);
    free(error.help);
}

char* _errCopyString(const char* str)
{
    size_t len = strlen(str);
    char* cpy = malloc((len + 1) * sizeof(char));
    strcpy_s(cpy,len + 1, str);
    assert(cpy);
    return cpy;
}