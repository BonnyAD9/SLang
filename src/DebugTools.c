#include "DebugTools.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int dtPrintf(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    int res = vfprintf(stdout, msg, args);

    va_end(args);
    printf("\n");
    return res;
}

void _Noreturn dtExcept(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vfprintf(stdout, msg, args);

    va_end(args);
    printf("\n");
    exit(EXIT_FAILURE);
}