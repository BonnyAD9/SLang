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
    assert(LEXER_READ_BUFFER_SIZE > 1, "_tokenize: minimum LEXER_READ_BUFFER_SIZE is 2 but it is %I64d", LEXER_READ_BUFFER_SIZE);

    FileSpanList list = createFileSpanList();
    
    char buffer[LEXER_READ_BUFFER_SIZE];
    size_t pos = 0;

    // tracking position in file
    size_t line = 1;
    size_t col = 0;

    // proccessing char by char
    int chr;
    while ((chr = fgetc(in)) != EOF)
    {
        // each char changes column in file by 1
        col++;

        switch (chr)
        {
        // newline ends any currently readed token and updates position in file
        case '\n':
            if (pos != 0)
            {
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col - pos));
                pos = 0;
            }
            line++;
            col = 0;
            continue;
        // whitespaces end any currently readed token
        case ' ':
        case '\t':
        case '\r':
            if (pos == 0)
                continue;
            fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col - pos));
            pos = 0;
            continue;
        // brackets are always token by them self
        case '[':
        case ']':
            // end any currently readed token
            if (pos != 0)
            {
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col - pos));
                pos = 0;
            }
            // read the bracket
            {
                char character = chr;
                fileSpanListAdd(&list, copyFileSpanFrom(&character, 1, line, col));
            }
            continue;
        // strings and chars are in single or double quotes
        case '"':
        case '\'':
            // if this is not first char of token, it is not string / char literal
            if (pos != 0)
            {
                if (pos >= LEXER_READ_BUFFER_SIZE)
                    except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                buffer[pos] = chr;
                pos++;
                continue;
            }
            // otherwise read all the text within the quotes
            {
                // save the position of the beggining of the literal
                size_t qLine = line;
                size_t qCol = col;
                // read the text (keep track of the position in file       v      v)
                pos = _readQuote(in, buffer, LEXER_READ_BUFFER_SIZE, chr, &line, &col);
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, qLine, qCol));
                pos = 0;
            }
            continue;
        // single line comments start with //
        case '/':
            // continue reading if there are not two /
            if (pos == 0 || buffer[pos - 1] != '/')
            {
                if (pos >= LEXER_READ_BUFFER_SIZE)
                    except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                buffer[pos] = '/';
                pos++;
                continue;
            }
            // comments immidietly end the token, the / is not part of the token
            if (pos > 1)
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos - 1, line, col - pos));
            // read the comment with the //
            pos = 2;
            buffer[0] = '/';
            buffer[1] = '/';
            while ((chr = fgetc(in)) != EOF)
            {
                // keep track of the position in file
                col++;
                // newline ends the line comment
                if (chr == '\n')
                {
                    fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col - pos));
                    pos = 0;
                    line++;
                    col = 0;
                    break;
                }
                if (pos >= LEXER_READ_BUFFER_SIZE)
                    except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                buffer[pos] = chr;
                pos++;
            }
            continue;
        // block comments start with /* and end with */ (/*/ is valid block comment)
        case '*':
            // checking for the begginging of the comment
            if (pos == 0 || buffer[pos - 1] != '/')
            {
                if (pos >= LEXER_READ_BUFFER_SIZE)
                    except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                buffer[pos] = '*';
                pos++;
                continue;
            }
            // comments immidietly end any previous token (/ is not part of the token)
            if (pos > 1)
                fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos - 1, line, col - pos));
            // read the comment with the /*
            pos = 2;
            buffer[0] = '/';
            buffer[1] = '*';
            {
                // save the position of the comment in file
                size_t comLine = line;
                size_t comCol = col - 1;
                while ((chr = fgetc(in)) != EOF)
                {
                    // keep track of position in file
                    col++;
                    if (chr == '\n')
                    {
                        line++;
                        col = 0;
                    }
                    if (pos >= LEXER_READ_BUFFER_SIZE)
                        except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
                    buffer[pos] = chr;
                    pos++;
                    // check for the */ that ends the comment
                    if (chr == '/' && buffer[pos - 2] == '*')
                    {
                        fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, comLine, comCol));
                        pos = 0;
                        break;
                    }
                }
            }
            continue;
        // read any nonspecial characters
        default:
            if (pos >= LEXER_READ_BUFFER_SIZE)
                except("_tokenize: token is too long, max size is %I64d", LEXER_READ_BUFFER_SIZE);
            buffer[pos] = chr;
            pos++;
            continue;
        }
    }
    // if file ends, read the last readed token
    if (pos != 0)
        fileSpanListAdd(&list, copyFileSpanFrom(buffer, pos, line, col - pos));
    return list;
}

size_t _readQuote(FILE* in, char* buffer, size_t bufferSize, int quoteChar, size_t* line, size_t* col)
{
    assert(in, "_readQuote: parameter in was null");
    assert(buffer, "_readQuote: parameter buffer was null");
    assert(bufferSize > 1, "_readQuote: parameter bufferSize was to small (%I64d)", bufferSize);
    assert(line, "_readQuote: parameter line was null");
    assert(col, "_readQuote: parameter col was null");

    // the first quote is part of the token
    *buffer = quoteChar;
    size_t pos = 0;

    int chr;
    while ((chr = fgetc(in)) != EOF)
    {
        // keep track of position in file and buffer
        (*col)++;
        pos++;
        if (pos >= bufferSize)
            except("_readQuote: quoted token is too long, max size is %I64d", bufferSize);
        
        buffer[pos] = chr;

        // check for ending quote
        if (chr == quoteChar)
            return pos + 1; // pos is position of last readed character

        // keep track of position in file across lines
        if (chr == '\n')
        {
            *col = 0;
            (*line)++;
        }

        // escaping
        if (chr != '\\')
            continue;

        // reading the escaped character
        if ((chr = fgetc(in)) == EOF)
            return pos + 1; // pos is position of last readed character
        (*col)++;

        switch (chr)
        {
        // NULL is escaped with \0
        case '0':
            buffer[pos] = '\0';
            continue;
        // newline is escaped with \n
        case 'n':
            buffer[pos] = '\n';
            continue;
        // carrige return is escaped with \r
        case 'r':
            buffer[pos] = '\r';
            continue;
        // tab is escaped with \t
        case 't':
            buffer[pos] = '\t';
            continue;
        // any other character after \ will be readed literaly (\\, \")
        default:
            // keep track of position in file
            if (chr == '\n')
            {
                *col = 0;
                (*line)++;
            }
            buffer[pos] = chr;
            continue;
        }
    }
    return pos + 1; // pos is position of last readed character
}