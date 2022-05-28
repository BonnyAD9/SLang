#include "ErrorToken.h"

#include <string.h>

#include "Assert.h"

ErrorToken createErrorToken(ErrorLevel level, FileSpan span, const char* message, const char* help)
{
    ErrorToken error =
    {
        .level = level,
        .line = span.line,
        .col = span.col,
        .token = span.str,
    };
    size_t len = strlen(message);
    char *str = malloc((len + 1) * sizeof(char));
    assert(str, "failed to allocate string of size %I64d", len + 1);
    error.message = strcpy(str, message);
    len = strlen(help);
    str = malloc((len + 1) * sizeof(char));
    assert(str, "failed to allocate string of size %I64d", len + 1);
    error.help = strcpy(str, help);
    return error;
}

void printErrorToken(FILE* out, ErrorToken token, const char* filename)
{
    const char* msgType;
    const char* msgColor;
    switch (token.level)
    {
    case INFO:
        msgType = "info:";
        msgColor = "\x1b[94m";
        break;
    case WARNING:
        msgType = "warning:";
        msgColor = "\x1b[95m";
        break;
    case ERROR:
        msgType = "error:";
        msgColor = "\x1b[91m";
        break;
    default:
        msgType = "message:";
        msgColor = "\x1b[93m";
        break;
    }

    fprintf(out, "%s:%I64d:%I64d: %s%s\x1b[0m %s\n\t%s%s\x1b[0m\n\t\x1b[92mhelp:\x1b[0m %s",
            filename, token.line, token.col,
            msgColor, msgType, token.message,
            msgColor, token.token,
            token.help);
}

void freeErrorToken(ErrorToken error)
{
    free(error.token);
    free(error.message);
    free(error.help);
}