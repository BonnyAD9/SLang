#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "StringBuilder.h"

typedef struct _StBufferStream
{
    char* buffer;
    size_t length;
    size_t pos;
} _StBufferStream;

typedef struct _StStringBuilderStream
{
    StringBuilder* sb;
    size_t pos;
} _StStringBuilderStream;

// FILE* stream functions
size_t _stFRead(void* stream, char* buffer, size_t length);
size_t _stFWrite(void* stream, const char* data, size_t length);
size_t _stFSeek(void* stream, long offset, int pos);

// _StBufferStream* stream functions
size_t _stBRead(void* stream, char* buffer, size_t length);
size_t _stBWrite(void* stream, const char* data, size_t length);
size_t _stBSeek(void* stream, long offset, int pos);
int _stClose(void* stream);

// _StBufferStream* stream functions
size_t _stSBRead(void* stream, char* buffer, size_t length);
size_t _stSBWrite(void* stream, const char* data, size_t length);
size_t _stSBSeek(void* stream, long offset, int pos);
int _stSBClose(void* stream);

Stream stCreate(StreamFlags flags, void* stream, StreamWriteFun write, StreamReadFun read, StreamSeekFun seek, StreamCloseFun close)
{
    Stream st =
    {
        .flags = flags,
        .stream = stream,
        .write = write,
        .read = read,
        .seek = seek,
        .close = close,
    };
    return st;
}

Stream* stRecreate(Stream* st, StreamFlags flags, void* stream, StreamWriteFun write, StreamReadFun read, StreamSeekFun seek, StreamCloseFun close)
{
    stClose(st);
    st->flags = flags;
    st->stream = stream;
    st->write = write;
    st->read = read;
    st->seek = seek;
    st->close = close;
    return st;
}

int stClose(Stream* st)
{
    if (st->flags & stCLOSED)
        return st_UNSUPPORTED;
    st->flags &= stCLOSED;
    return st->close(st->stream);
}

size_t stWrite(Stream* st, const char* data, size_t length)
{
    return st->flags & stWRITE
        ? st->write(st->stream, data, length)
        : st_UNSUPPORTED;
}

size_t stRead(Stream* st, char* buffer, size_t length)
{
    return st->flags & stREAD
        ? st->read(st->stream, buffer, length)
        : st_UNSUPPORTED;
}

size_t stSeek(Stream* st, long offset, int pos)
{
    return st->flags & stSEEK
        ? st->seek(st->seek, offset, pos)
        : st_UNSUPPORTED;
}

int stFileStream(Stream* st, const char* filename, const char* mode)
{
    StreamFlags flags = stSEEK;
    for (const char* i = mode; *i; i++)
    {
        switch (*i)
        {
        case 'r':
            flags |= stREAD;
            continue;
        case 'a':
        case 'w':
            flags |= stWRITE;
            continue;
        case '+':
            flags |= stREAD | stWRITE;
            continue;
        default:
            continue;
        }
    }

    st->flags = flags;
    st->read = _stFRead;
    st->write = _stFWrite;
    st->seek = _stFSeek;
    st->close = (StreamCloseFun)fclose;

    return fopen_s((FILE**)&st->stream, filename, mode);
}

int stGetChar(Stream* st)
{
    unsigned char c;
    int err;
    if ((err = stRead(st, (char*)&c, 1)) != 1)
        return err == 0 ? EOF : -llabs(err);
    return c;
}

int stBufferStream(Stream* st, char* buffer, size_t length, StreamFlags flags)
{
    _StBufferStream* bs = malloc(sizeof(_StBufferStream));
    if (!bs)
        return -1;
    
    bs->buffer = buffer;
    bs->length = length;
    bs->pos = 0;

    st->flags = flags;
    st->stream = bs;
    st->write = _stBWrite;
    st->read = _stBRead;
    st->seek = _stBSeek;
    st->close = _stClose;

    return 0;
}

int stWriteChar(Stream* st, char c)
{
    return stWrite(st, &c, 1) == 1
        ? 0
        : EOF;
}

int stStringBuilderStream(Stream* st, StringBuilder* sb, StreamFlags flags)
{
    _StStringBuilderStream* sbs = malloc(sizeof(_StStringBuilderStream));
    if (!sbs)
        return -1;

    sbs->pos = 0;
    sbs->sb = sb;

    st->flags = flags;
    st->stream = sbs;
    st->write = _stSBWrite;
    st->read = _stSBRead;
    st->seek = _stSBSeek;
    st->close = _stClose;

    return 0;
}

size_t _stFRead(void* stream, char* buffer, size_t length)
{
    return fread(buffer, sizeof(char), length, (FILE*)stream);
}

size_t _stFWrite(void* stream, const char* data, size_t length)
{
    return fwrite(data, sizeof(char), length, (FILE*)stream);
}

size_t _stFSeek(void* stream, long offset, int pos)
{
    if (offset == 0 && pos == SEEK_CUR)
        return ftell((FILE*)stream);
    fseek((FILE*)stream, offset, pos);
    return ftell((FILE*)stream);
}

size_t _stBRead(void* stream, char* restrict buffer, size_t length)
{
    _StBufferStream* restrict bs = stream;
    size_t i = 0;
    for (; i < length && bs->pos < bs->length; i++, bs->pos++)
        buffer[i] = bs->buffer[bs->pos];
    return i;
}

size_t _stBWrite(void* stream, const char* data, size_t length)
{
    _StBufferStream* restrict bs = stream;
    size_t i = 0;
    for (; i < length && bs->pos < bs->length; i++, bs->pos++)
        bs->buffer[bs->pos] = data[i];
    return i;
}

size_t _stBSeek(void* stream, long offset, int st)
{
    _StBufferStream* restrict bs = stream;

    if (offset == 0 && st == SEEK_CUR)
        return bs->pos;

    size_t pos;
    switch (st)
    {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = bs->pos + offset;
        break;
    case SEEK_END:
        pos = bs->length + offset;
        break;
    default:
        return -3;
    }

    if (pos < 0 || pos > bs->length)
        return -4;

    return bs->pos = pos;
}

int _stClose(void* stream)
{
    free(stream);
    return 0;
}

size_t _stSBRead(void* stream, char* buffer, size_t length)
{
    _StStringBuilderStream* restrict sb = stream;

    size_t i = 0;
    for (; i < length && sb->pos < sb->sb->length; i++, sb->pos++)
        buffer[i] = sb->sb->buffer[sb->pos];
    return i;
}

size_t _stSBWrite(void* stream, const char* data, size_t length)
{
    _StStringBuilderStream* restrict sb = stream;
    if (sb->pos == sb->sb->length)
    {
        sbAppendL(sb->sb, data, length);
        return length;
    }

    size_t l = sb->pos + length;
    if (l < sb->sb->allocated)
    {
        memcpy_s(sb->sb->buffer + sb->pos, sb->sb->allocated - sb->pos, data, length);
        if (l > sb->sb->length)
            sb->sb->length = l;
        return length;
    }

    size_t w = l - sb->sb->allocated;
    memcpy_s(sb->sb->buffer + sb->pos, sb->sb->allocated - sb->pos, data, length);
    sb->sb->length = sb->sb->allocated;
    sbAppendL(sb->sb, data + w, l - w);
    return length;
}

size_t _stSBSeek(void* stream, long offset, int st)
{
    _StStringBuilderStream* restrict sb = stream;

    if (offset == 0 && st == SEEK_CUR)
        return sb->pos;

    size_t pos;
    switch (st)
    {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = sb->pos + offset;
        break;
    case SEEK_END:
        pos = sb->sb->length + offset;
        break;
    default:
        return -3;
    }

    if (pos < 0 || pos > sb->sb->length)
        return -4;

    return sb->pos = pos;
}