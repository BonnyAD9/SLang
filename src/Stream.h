#ifndef st_STREAM_INCLUDED
#define st_STREAM_INCLUDED

#include <stdio.h>

#include "StringBuilder.h"

#define st_UNSUPPORTED (EOF - 1)

typedef enum StreamFlags
{
    stNONE = 0,
    stREAD = 1,
    stWRITE = 2,
    stSEEK = 4,
    stALL = 7,
    stCLOSED = 8,
} StreamFlags;

typedef size_t (*StreamWriteFun)(void* stream, const char* data, size_t length);
typedef size_t (*StreamReadFun)(void* stream, char* buffer, size_t length);
typedef size_t (*StreamSeekFun)(void* stream, long offset, int pos);
typedef int (*StreamCloseFun)(void* stream);

typedef struct Stream
{
    StreamFlags flags;
    void* stream;
    StreamWriteFun write;
    StreamReadFun read;
    StreamSeekFun seek;
    StreamCloseFun close;
} Stream;

/**
 * @brief creates new custom stream
 * 
 * @param flags flags for the stream
 * @param stream the stream object
 * @param write write function
 * @param read read function
 * @param seek seek functin
 * @param close close function
 * @return Stream new instance
 */
Stream stCreate(StreamFlags flags, void* stream, StreamWriteFun write, StreamReadFun read, StreamSeekFun seek, StreamCloseFun close);

/**
 * @brief recreates stream
 * 
 * @param st stream to recreate
 * @param flags flags for the stream
 * @param stream stream object
 * @param write write function
 * @param read read function
 * @param seek seek function
 * @param close close function
 * @return Stream the recreated stream
 */
Stream* stRecreate(Stream* st, StreamFlags flags, void* stream, StreamWriteFun write, StreamReadFun read, StreamSeekFun seek, StreamCloseFun close);

/**
 * @brief closes the given stream
 * 
 * @param st stream to close
 * @return int error code, 0 on success
 */
int stClose(Stream* st);

/**
 * @brief writes into the stream
 * 
 * @param st stream to write to
 * @param data what to write
 * @param length length of data
 * @return size_t number of written characters
 */
size_t stWrite(Stream* st, const char* data, size_t length);

/**
 * @brief reads from the given stream
 * 
 * @param st where to read from
 * @param buffer where to read to
 * @param length maximum number of characters to read
 * @return size_t number of characters that was readed
 */
size_t stRead(Stream* st, char* buffer, size_t length);

/**
 * @brief seeks in the stream
 * 
 * @param st stream to seek
 * @param offset offset
 * @param pos whence to seek
 * @return size_t position in stream after seeking
 */
size_t stSeek(Stream* st, long offset, int pos);

/**
 * @brief opens stream as file
 * 
 * @param st where to save the stream
 * @param filename file to open
 * @param mode mode
 * @return int error code (0 on success)
 */
int stFileStream(Stream* st, const char* filename, const char* mode);

/**
 * @brief reads one char from the stream
 * 
 * @param st where to read from
 * @return int the char cast to int on succes, otherwise negative
 */
int stGetChar(Stream* st);

/**
 * @brief writes the given char to the stream
 * 
 * @param st stream to write to
 * @param c char to erite
 * @return int error code (0 on success)
 */
int stWriteChar(Stream* st, char c);

/**
 * @brief creates stream from buffer
 *
 * @param st Where to initialize the stream
 * @param buffer stream buffer
 * @param length length of the buffer
 * @param flags flags
 * @return Stream error code (0 on success)
 */
int stBufferStream(Stream* st, char* buffer, size_t length, StreamFlags flags);

/**
 * @brief creates new StringBuilder stream
 * 
 * @param st where to create the stream
 * @param flags flags
 * @return int error code (0 on success)
 */
int stStringBuilderStream(Stream* st, StringBuilder* sb, StreamFlags flags);

#endif // st_STREAM_INCLUDED