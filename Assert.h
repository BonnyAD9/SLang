#ifndef ASSERT_INCLUDED
#define ASSERT_INCLUDED

#ifdef _DEBUG

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

int assert(bool val, const char* msg, ...)
{
    if (val)
        return 0;

    va_list args;
    va_start(args, msg);

    int res = vfprintf(stdout, msg, args);

    va_end(args);

#ifdef ASSERT_EXIT
    exit(EXIT_FAILURE);
#endif // ASSERT_EXIT
    return res;
}

#else // _DEBUG
#define assert(val, msg, ...)
#endif // _DEBUG

#endif // ASSERT_INCLUDED