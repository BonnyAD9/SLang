#ifndef ASSERT_INCLUDED
#define ASSERT_INCLUDED

#include <stdbool.h>

#ifdef _DEBUG

int assert(bool val, const char* msg, ...);

int dprintf(const char* msg, ...);

#else // _DEBUG
#define assert(val, msg, ...)
#define dprintf(msg, ...)
#endif // _DEBUG

int except(const char* msg, ...);

#endif // ASSERT_INCLUDED