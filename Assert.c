#include "Assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

int assert(bool val, const char *msg, ...)
{
    if (val)
        return 0;

    va_list args;
    va_start(args, msg);

    int res = vfprintf(stdout, msg, args);

    va_end(args);
    printf("\n");

#ifdef ASSERT_EXIT
    exit(EXIT_FAILURE);
#endif // ASSERT_EXIT
    return res;
}

int except(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    int res = vfprintf(stdout, msg, args);

    va_end(args);
    printf("\n");
    exit(EXIT_FAILURE);
    return res;
}