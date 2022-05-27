#include "Lexer.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Token.h"
#include "TokenArray.h"
#include "FileSpan.h"
#include "FileSpanList.h"
#include "Assert.h"

FileSpanList _tokenize(FILE* in);
/**
 * @brief determines whether the given character is whitespace
 * 
 * @param c character to examine
 * @return true given character is whitespace
 * @return false given character is not whitespace
 */
bool _isWhitespace(const char c);

/**
 * @brief reads quoted text
 * 
 * @param in where to read from
 * @param buffer where to read to
 * @param bufferSize maximum size of buffer
 * @param quoteChar character that quotes the text
 * @param line tracks line number
 * @param col tracks column number
 * @return size_t number of characters readed
 */
size_t _readQuote(FILE* in, char* buffer, size_t bufferSize, int quoteChar, size_t* line, size_t* col);

TokenArray lex(FILE* in)
{
    assert(in, "lex: parameter in was null");
    FileSpanList list = _tokenize(in);
    TokenArray arr = createTokenArray(list.length);
    for (size_t i = 0; i < arr.length; i++)
        arr.data[i] = fileSpanToken(LITERAL_STRING, list.data[i]);
    return arr;
}

FileSpanList _tokenize(FILE* in)
{
    assert(in, "_tokenize: parameter in was null");
    FileSpanList list = createFileSpanList();
    char buffer[LEXER_READ_BUFFER_SIZE];
    size_t pos = 0;
    size_t line = 1;
    size_t col = 0;
    int chr;
    char character;
    while ((chr = fgetc(in)) != EOF)
    {
        col++;
        switch (chr)
        {
        case '\n':
            line++;
            col = 0;
        case ' ':
        case '\t':
        case '\r':
            if (pos == 0)
                continue;
            fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col));
            pos = 0;
            continue;
        case '[':
        case ']':
            if (pos != 0)
            {
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col));
                pos = 0;
            }
            character = chr;
            fileSpanListAdd(&list, copyFileSpanFrom(&character, 1, line, col));
            continue;
        case '"':
        case '\'':
            if (pos != 0)
            {
                if (pos >= LEXER_READ_BUFFER_SIZE)
                    except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                buffer[pos] = chr;
                pos++;
                continue;
            }
            pos = _readQuote(in, buffer, LEXER_READ_BUFFER_SIZE, chr, &line, &col);
            continue;
        default:
            if (pos >= LEXER_READ_BUFFER_SIZE)
                except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
            buffer[pos] = chr;
            pos++;
            continue;
        }
    }
    return list;
}

size_t _readQuote(FILE* in, char* buffer, size_t bufferSize, int quoteChar, size_t* line, size_t* col)
{
    assert(in, "_readQuote: parameter in was null");
    assert(buffer, "_readQuote: parameter buffer was null");
    assert(bufferSize > 1, "_readQuote: parameter bufferSize was to small (%I64d)", bufferSize);
    assert(line, "_readQuote: parameter line was null");
    assert(col, "_readQuote: parameter col was null");

    *buffer = quoteChar;
    size_t pos = 0;
    int chr;
    while ((chr = fgetc(in)) != EOF)
    {
        (*col)++;
        pos++;
        if (pos >= bufferSize)
            except("_readQuote: quoted token is too long, max size is %I64d", bufferSize);
        buffer[pos] = chr;
        if (chr == quoteChar)
            return pos + 1;
        if (chr == '\n')
        {
            *col = 0;
            (*line)++;
        }
        if (chr != '\\')
            continue;
        if ((chr = fgetc(in)) == EOF)
            return pos;
        (*col)++;
        switch (chr)
        {
        case '0':
            buffer[pos] = '\0';
            continue;
        case 'n':
            buffer[pos] = '\n';
            continue;
        case 'r':
            buffer[pos] = '\r';
            continue;
        case 't':
            buffer[pos] = '\t';
            continue;
        default:
            if (chr == '\n')
            {
                *col = 0;
                (*line)++;
            }
            buffer[pos] = chr;
            continue;
        }
    }
    return pos + 1;
}