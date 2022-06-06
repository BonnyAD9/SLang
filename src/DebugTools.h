#ifndef dt_DEBUG_TOOLS_INCLUDED
#define dt_DEBUG_TOOLS_INCLUDED

#ifdef NDEBUG

#define dprintf(msg, ...)

#else // NDEBUG
int dtPrintf(const char* msg, ...);
#endif // NEDUBG

void _Noreturn dtExcept(const char* msg, ...);

#endif // dt_DEBUG_TOOLS_INCLUDED