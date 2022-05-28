#include "Lexer.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "Token.h"
#include "FileSpan.h"
#include "List.h"
#include "Assert.h"
#include "ErrorToken.h"

#ifndef DECIMAL_WARNING_LIMIT
#define DECIMAL_WARNING_LIMIT 17
#endif // DECIMAL_WARNING_LIMIT

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

/**
 * @brief checks the keyword and adds it to the list
 * 
 * @param kw keyword to check for
 * @param type type of the keyword to add
 * @param span span to check
 * @param tokens list of tokens for valid keyword
 * @param errors list of errors for invalid keyword
 * @return true keyword matched
 * @return false keyword not matched
 */
bool _checkKeyword(const char* kw, TokenType type, FileSpan span, List* tokens, List* errors);

List lex(FILE* in, List* errors)
{
    assert(in, "lex: parameter in was null");
    assert(errors, "lex: parameter errors was null");

    // read from file and prepare output lists
    List list = _tokenize(in);
    List errs = newList(ErrorToken);
    List tokens = newList(Token);

    for (size_t i = 0; i < list.length; i++)
    {
        // get tje string to examine
        FileSpan span = listGet(list, i, FileSpan);
        // check for tokens that can be recognized by their first few characters
        switch (span.str[0])
        {
        // this case should never happen
        case 0:
            dprintf("lex: empty token at position :%I64d:%I64d", span.line, span.col);
            freeFileSpan(span);
            continue;
        // [ is always token by itself
        case '[':
            assert(span.length == 1, "lex: [ is not by itself in token '%s' at position :%I64d:%I64d", span.str, span.line, span.col);
            listAdd(tokens, fileSpanTokenPos(PUNCTUATION_BRACKET_OPEN, span), Token);
            freeFileSpan(span);
            continue;
        // ] is always token by itself
        case ']':
            assert(span.length == 1, "lex: ] is not by itself in token '%s' at position :%I64d:%I64d", span.str, span.line, span.col);
            listAdd(tokens, fileSpanTokenPos(PUNCTUATION_BRACKET_CLOSE, span), Token);
            freeFileSpan(span);
            continue;
        // check for comments (starts with // or /*)
        case '/':
            switch(span.str[1])
            {
            // line comment
            case '/':
                listAdd(tokens, fileSpanTokenPart(COMMENT_LINE, span, 2, span.length - 2), Token);
                freeFileSpan(span);
                continue;
            // block comment
            case '*':
                // check if the block comment is closed
                if (span.str[span.length - 1] != '/' || span.str[span.length - 2] != '*')
                {
                    listAdd(errs, createErrorToken(ERROR, span, "block comment is not closed", "close it with */"), ErrorToken);
                    continue;
                }
                listAdd(tokens, fileSpanTokenPart(COMMENT_BLOCK, span, 2, span.length < 5 ? 0 : span.length - 4), Token);
                freeFileSpan(span);
                continue;
            default:
                break;
            }
            break;
        // string literal
        case '"':
            // check if the string literal is ended
            if (span.length == 1 || span.str[span.length - 1] != '"')
            {
                listAdd(errs, createErrorToken(ERROR, span, "string literal is not closed", "try adding closing \""), ErrorToken);
                continue;
            }
            listAdd(tokens, fileSpanTokenPart(LITERAL_STRING, span, 1, span.length - 2), Token);
            freeFileSpan(span);
            continue;
        // char literal
        case '\'':
            // check if the char literal has only one character
            if (span.length != 3)
            {
                listAdd(errs, createErrorToken(ERROR, span, "char literal can only contain one character", "maybe you want to use string (\")"), ErrorToken);
                continue;
            }
            // check if the char literal is closed
            else if (span.str[3] != '\'')
            {
                listAdd(errs, createErrorToken(ERROR, span, "char literal is not closed", "try adding closing '"), ErrorToken);
                continue;
            }
            listAdd(tokens, fileSpanCharToken(LITERAL_CHAR, span.str[1], span), Token);
            freeFileSpan(span);
            continue;
        // nothing operator
        case '_':
            // check if it is only the operator
            if (span.length != 1)
                break;
            listAdd(tokens, fileSpanTokenPos(OPERATOR_NOTHING, span), Token);
            freeFileSpan(span);
            continue;
        default:
            break;
        }
        // checking numbers
        if (isdigit(span.str[0]) || (span.str[0] == '-' && isdigit(span.str[1])))
        {
            long long num = 0;
            char* c = span.str;
            bool overflow = false;
            // check for negative values
            bool isNegative = *c == '-';
            c += isNegative;
            // read whole part values
            for (; *c && isdigit(*c); c++)
            {
                // check for overflow
                if (num > LONG_LONG_MAX / 10 || (num == LONG_LONG_MAX / 10 && *c - '0' > LONG_LONG_MAX % 10))
                    overflow = true;
                num = num * 10 + *c - '0';
            }
            switch (*c)
            {
            // if it doesn't continue, it is integer
            case 0:
                listAdd(tokens, fileSpanIntToken(LITERAL_INTEGER, isNegative ? -num : num, span), Token);
                if (overflow)
                    listAdd(errs, createErrorToken(WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorToken) // there shouldn't be ;
                else
                    freeFileSpan(span);
                continue;
            // if there is . read decimal values
            case '.':
            {
                // read decimal values
                double decimal = num;
                size_t digits = 0;
                // to preserve the magnitude of the number read it again as double
                if (overflow)
                {
                    decimal = 0;
                    for (c = span.str + isNegative; *c && isdigit(*c); c++, digits++)
                        decimal = decimal * 10 + *c - '0';
                }
                double div = 10;
                for (c++; *c && isdigit(*c); c++, div *= 10, digits++)
                    decimal += (*c - '0') / div;
                // if this is not end break into error
                if (*c)
                    break;
                listAdd(tokens, fileSpanDecToken(LITERAL_FLOAT, isNegative ? -decimal : decimal, span), Token);
                // check for too large or precise numbers
                if (isinf(decimal))
                    listAdd(errs, createErrorToken(WARNING, span, "number is too large and will be trated as infinity", "use different type (string?)"), ErrorToken) // there shouldn't be ;
                else if (digits > DECIMAL_WARNING_LIMIT)
                    listAdd(errs, createErrorToken(WARNING, span, "number has too many digits and may be rounded", "if you want all the digits maybe use string"), ErrorToken) // there shouldn't be ;
                else
                    freeFileSpan(span);
                continue;
            }
            // otherwise break into error
            default:
                break;
            }
            listAdd(errs, createErrorToken(ERROR, span, "invalid number literal", "number literals cannot contain other characters than digits and single ."), ErrorToken);
            continue;
        }

        // if the list is empty, this is surely incorrect token
        if (list.length == 0)
        {
            listAdd(errs, createErrorToken(ERROR, span, "cannot use identifiers directly", "try ecapsulating it in []"), ErrorToken);
            continue;
        }

#define __checkKeyword(__kws, __kwe) if(_checkKeyword(__kws,__kwe,span,&tokens,&errs))continue
        // check for the keywords
        __checkKeyword("def", KEYWORD_DEF);
        __checkKeyword("struct", KEYWORD_STRUCT);
        __checkKeyword("set", KEYWORD_SET);
        __checkKeyword("defined", KEYWORD_DEFINED);
#undef __checkKeyword

        // determine token type based on previous tokens
        switch (listGet(tokens, list.length - 1, Token).type)
        {
        // tokens directly after [ are function identifiers
        case PUNCTUATION_BRACKET_OPEN:
            listAdd(tokens, fileSpanToken(IDENTIFIER_FUNCTION, span), Token);
            continue;
        // tokens after struct keyword are type identifiers
        case KEYWORD_STRUCT:
            listAdd(tokens, fileSpanToken(IDENTIFIER_STRUCT, span), Token);
            continue;
        // other tokens are just variable identifiers
        default:
            listAdd(tokens, fileSpanToken(IDENTIFIER_VARIABLE, span), Token);
            continue;
        }
    }
    // free the list of strings
    freeList(list);
    // if errors is not null set them, otherwise free them
    if (errors)
        *errors = errs;
    else
        listDeepFree(list, ErrorToken, t, freeErrorToken(t));

    return tokens;
}

bool _checkKeyword(const char* kw, TokenType type, FileSpan span, List* tokens, List* errors)
{
    if (strcmp(kw, span.str) == 0)
    {
        if (listGet(*tokens, tokens->length - 1, Token).type != PUNCTUATION_BRACKET_OPEN)
        {
            ErrorToken t = createErrorToken(ERROR, span, "keyword must be used as function", "try encapsulating the action in []");
            listAddP(errors, &t);
            return true;
        }
        Token t = fileSpanTokenPos(type, span);
        listAddP(tokens, &t);
        return true;
    }
    return false;
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