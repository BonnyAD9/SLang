#include "Lexer.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Token.h"
#include "TokenArray.h"
#include "FileSpan.h"
#include "List.h"
#include "Assert.h"
#include "ErrorToken.h"

/**
 * @brief splits the file contents into FileSpans
 * 
 * @param in File to read from
 * @return List list of FileSpans
 */
List _tokenize(FILE* in);

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

TokenArray lex(FILE* in, List* errors)
{
    assert(in, "lex: parameter in was null");
    assert(errors, "lex: parameter errors was null");

    List errs = newList(ErrorToken);
    List list = _tokenize(in);
    TokenArray arr = createTokenArray(list.length);

    for (size_t i = 0; i < arr.length; i++)
    {
        FileSpan span = listGet(list, i, FileSpan);
        switch (span.str[0])
        {
        case 0:
            dprintf("lex: empty token at position %I64d:%I64d", span.line, span.col);
            freeFileSpan(span);
            continue;
        case '[':
            arr.data[i] = fileSpanTokenPos(PUNCTUATION_BRACKET_OPEN, span);
            freeFileSpan(span);
            continue;
        case ']':
            arr.data[i] = fileSpanTokenPos(PUNCTUATION_BRACKET_CLOSE, span);
            freeFileSpan(span);
            continue;
        case '/':
            switch(span.str[1])
            {
            case '/':
                arr.data[i] = fileSpanTokenPart(COMMENT_LINE, span, 2, span.length - 2);
                freeFileSpan(span);
                continue;
            case '*':
                arr.data[i] = fileSpanTokenPart(COMMENT_BLOCK, span, 2, span.length < 5 ? 0 : span.length - 4);
                freeFileSpan(span);
                continue;
            default:
                break;
            }
            break;
        case '"':
            if (span.length == 1 || span.str[span.length - 1] != '"')
            {
                listAdd(errs, createErrorToken(ERROR, span, "string literal is not closed", "try adding closing \""), ErrorToken);
                continue;
            }
            arr.data[i] = fileSpanTokenPart(LITERAL_STRING, span, 1, span.length - 2);
            freeFileSpan(span);
            continue;
        case '\'':
            if (span.length != 3)
            {
                listAdd(errs, createErrorToken(ERROR, span, "char literal has invalid length", "char literal can only contain one character"), ErrorToken);
                continue;
            }
            else if (span.str[3] != '\'')
            {
                listAdd(errs, createErrorToken(ERROR, span, "char literal is not closed", "try adding closing '"), ErrorToken);
                continue;
            }
            arr.data[i] = fileSpanCharToken(LITERAL_CHAR, span.str[1], span);
            freeFileSpan(span);
            continue;
        case ';':
            if (span.length != 1)
                break;
            arr.data[i] = fileSpanTokenPos(OPERATOR_TRUST, span);
            freeFileSpan(span);
            continue;
        case '_':
            if (span.length != 1)
                break;
            arr.data[i] = fileSpanTokenPos(OPERATOR_NOTHING, span);
            freeFileSpan(span);
            continue;
        default:
            break;
        }
        if (isdigit(span.str[0]))
        {
            long long num = 0;
            char* c = span.str;
            for (; *c && isdigit(*c); c++)
                num = num * 10 + *c - '0';
            switch (*c)
            {
            case 0:
                arr.data[i] = fileSpanIntToken(LITERAL_INTEGER, num, span);
                freeFileSpan(span);
                continue;
            case '.':
            {
                double decimal = num;
                double div = 10;
                for (c++; *c && isdigit(*c); c++, div *= 10)
                    decimal += (*c - '0') / div;
                if (*c)
                    break;
                arr.data[i] = fileSpanDecToken(LITERAL_FLOAT, decimal, span);
                freeFileSpan(span);
                continue;
            }
            default:
                break;
            }
            listAdd(errs, createErrorToken(ERROR, span, "invalid number literal", "number literals cannot contain other characters than digits and single ."), ErrorToken);
            continue;
        }
        if (strcmp(span.str, "def") == 0)
        {
            arr.data[i] = fileSpanTokenPos(KEYWORD_DEF, span);
            freeFileSpan(span);
            continue;
        }
        if (strcmp(span.str, "struct") == 0)
        {
            arr.data[i] = fileSpanTokenPos(KEYWORD_STRUCT, span);
            freeFileSpan(span);
            continue;
        }
        if (strcmp(span.str, "set") == 0)
        {
            arr.data[i] = fileSpanTokenPos(KEYWORD_SET, span);
            freeFileSpan(span);
            continue;
        }
        if (i == 0)
        {
            listAdd(errs, createErrorToken(ERROR, span, "cannot use identifiers directly", "try ecapsulating it in []"), ErrorToken);
            continue;
        }
        switch (arr.data[i - 1].type)
        {
        case PUNCTUATION_BRACKET_OPEN:
            arr.data[i] = fileSpanToken(IDENTIFIER_FUNCTION, span);
            continue;
        case KEYWORD_STRUCT:
            arr.data[i] = fileSpanToken(IDENTIFIER_STRUCT, span);
            continue;
        default:
            arr.data[i] = fileSpanToken(IDENTIFIER_VARIABLE, span);
            continue;
        }
    }
    freeList(list);
    if (errors)
        *errors = errs;
    else
        listDeepFree(list, ErrorToken, t, freeErrorToken(t));
    return arr;
}

List _tokenize(FILE* in)
{
    assert(in, "_tokenize: parameter in was null");
    assert(LEXER_READ_BUFFER_SIZE > 1, "_tokenize: minimum LEXER_READ_BUFFER_SIZE is 2 but it is %I64d", LEXER_READ_BUFFER_SIZE);

    List list = newList(FileSpan);

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
                listAdd(list, copyFileSpanFrom(buffer, pos, line, col - pos), FileSpan);
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
            listAdd(list, copyFileSpanFrom(buffer, pos, line, col - pos), FileSpan);
            pos = 0;
            continue;
        // brackets are always token by them self
        case '[':
        case ']':
            // end any currently readed token
            if (pos != 0)
            {
                listAdd(list, copyFileSpanFrom(buffer, pos, line, col - pos), FileSpan);
                pos = 0;
            }
            // read the bracket
            {
                char character = chr;
                listAdd(list, copyFileSpanFrom(&character, 1, line, col), FileSpan);
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
                listAdd(list, copyFileSpanFrom(buffer, pos, qLine, qCol), FileSpan);
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
                listAdd(list, copyFileSpanFrom(buffer, pos - 1, line, col - pos), FileSpan);
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
                    listAdd(list, copyFileSpanFrom(buffer, pos, line, col - pos), FileSpan);
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
                listAdd(list, copyFileSpanFrom(buffer, pos - 1, line, col - pos), FileSpan);
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
                        listAdd(list, copyFileSpanFrom(buffer, pos, comLine, comCol), FileSpan);
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
        listAdd(list, copyFileSpanFrom(buffer, pos, line, col - pos), FileSpan);
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