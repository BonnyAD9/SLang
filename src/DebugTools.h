#ifndef ASSERT_INCLUDED
#define ASSERT_INCLUDED

#ifdef NDEBUG

#define dprintf(msg, ...)

#else // NDEBUG
int dprintf(const char* msg, ...);
#endif // NEDUBG

void _Noreturn except(const char* msg, ...);

#endif // ASSERT_INCLUDED