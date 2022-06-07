#include "Terminal.h"

#include "Stream.h"

// FILE* stream functions defined in Stream.c
size_t _stFRead(void* stream, char* buffer, size_t length);
size_t _stFWrite(void* stream, const char* data, size_t length);
size_t _stFSeek(void* stream, long offset, int pos);

Stream _termCreate(FILE* str, StreamFlags flags);
int _nothing(void* p) { return 0; }

Stream* _termIn()
{
    static Stream s = { .flags = stNONE };
    if (s.flags == stNONE)
        s = _termCreate(stdin, stREAD);
    return &s;
}

Stream* _termOut()
{
    static Stream s = { .flags = stNONE };
    if (s.flags == stNONE)
        s = _termCreate(stdout, stWRITE);
    return &s;
}

Stream* _termErr()
{
    static Stream s = { .flags = stNONE };
    if (s.flags == stNONE)
        s = _termCreate(stderr, stWRITE);
    return &s;
}

Stream _termCreate(FILE* str, StreamFlags flags)
{
    return stCreate(flags, str, _stFWrite, _stFRead, _stFSeek, _nothing);
}