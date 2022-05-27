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

void freeErrorToken(ErrorToken error)
{
    free(error.token);
    free(error.message);
    free(error.help);
}