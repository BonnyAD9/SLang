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
#include "Errors.h"

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
 * @brief reads positive int from string
 * 
 * @param str string to read from
 * @param endptr pointer to the first non-digit char
 * @param base base in which to convert
 * @param overflow this is set to true if the integer overflows the long long limit
 * @return long readed integer
 */
long long _readInt(char* str, char** endptr, long long base, bool* overflow);

/**
 * @brief checks the keyword and adds it to the list
 * 
 * @param kw keyword to check for
 * @param type type of the keyword to add
 * @param span span to check
 * @param tokens list of tokens for valid keyword
 * @param errors list of errors for invalid keyword
 * @return 0 keyword not matched
 * @return 1 keyword matched successfully
 * @return -1 keyword matced unsuccessfully
 */
int _checkKeyword(const char* kw, TokenType type, FileSpan span, List* tokens, List* errors);

/**
 * @brief converts the given char to numnerical digit
 * 
 * @param digit char to convert
 * @return long the number it represents
 */
long long _getDigit(char digit);

List lex(FILE* in, List* errors)
{
    assert(in, "lex: parameter in was null");
    assert(errors, "lex: parameter errors was null");

    // read from file and prepare output lists
    List list = _tokenize(in);
    List errs = newList(ErrorSpan);
    List tokens = newList(Token);

    long long nest = 0;
    long long defd = -1;
    long long parm = -1;

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
            if (defd == nest)
            {
                Token* t = listGetP(tokens, tokens.length - 1);
                if (t->type == T_IDENTIFIER_STRUCT)
                    t->type = T_IDENTIFIER_FUNCTION;
            }
            listAdd(tokens, fileSpanIntToken(T_PUNCTUATION_BRACKET_OPEN, nest, span), Token);
            freeFileSpan(span);
            nest++;
            continue;
        // ] is always token by itself
        case ']':
            assert(span.length == 1, "lex: ] is not by itself in token '%s' at position :%I64d:%I64d", span.str, span.line, span.col);
            if (nest == defd || nest == parm)
            {
                defd = -1;
                parm = -1;
            }
            if (nest == 0)
            {
                listAdd(errs, createErrorSpan(E_ERROR, span, "missing [ before ]", "add opening bracket somwhere before this closing one"), ErrorSpan)
                continue;
            }
            nest--;
            listAdd(tokens, fileSpanIntToken(T_PUNCTUATION_BRACKET_CLOSE, nest, span), Token);
            freeFileSpan(span);
            continue;
        // check for comments (starts with // or /*)
        case '/':
            switch(span.str[1])
            {
            // line comment
            case '/':
                //listAdd(tokens, fileSpanTokenPart(COMMENT_LINE, span, 2, span.length - 2), Token);
                freeFileSpan(span);
                continue;
            // block comment
            case '*':
                // check if the block comment is closed
                if (span.str[span.length - 1] != '/' || span.str[span.length - 2] != '*')
                {
                    listAdd(errs, createErrorSpan(E_ERROR, span, "block comment is not closed", "close it with */"), ErrorSpan);
                    continue;
                }
                //listAdd(tokens, fileSpanTokenPart(COMMENT_BLOCK, span, 2, span.length < 5 ? 0 : span.length - 4), Token);
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
                listAdd(errs, createErrorSpan(E_ERROR, span, "string literal is not closed", "try adding closing \""), ErrorSpan);
                continue;
            }
            listAdd(tokens, fileSpanTokenPart(T_LITERAL_STRING, span, 1, span.length - 2), Token);
            freeFileSpan(span);
            continue;
        // char literal
        case '\'':
            // check if the char literal has only one character
            if (span.length != 3)
            {
                listAdd(errs, createErrorSpan(E_ERROR, span, "char literal can only contain one character", "maybe you want to use string (\")"), ErrorSpan);
                continue;
            }
            // check if the char literal is closed
            else if (span.str[3] != '\'')
            {
                listAdd(errs, createErrorSpan(E_ERROR, span, "char literal is not closed", "try adding closing '"), ErrorSpan);
                continue;
            }
            listAdd(tokens, fileSpanCharToken(T_LITERAL_CHAR, span.str[1], span), Token);
            freeFileSpan(span);
            continue;
        // nothing operator
        case '_':
            // check if it is only the operator
            if (span.length != 1)
                break;
            listAdd(tokens, fileSpanTokenPos(T_OPERATOR_NOTHING, span), Token);
            freeFileSpan(span);
            continue;
        default:
            break;
        }
        // checking numbers
        if (isdigit(span.str[0]) || (span.str[0] == '-' && isdigit(span.str[1])))
        {
            char* c = span.str;
            bool overflow = false;
            // check for negative values
            bool isNegative = *c == '-';
            // read whole part values
            long long num = _readInt(c + isNegative, &c, 10, &overflow);
            switch (*c)
            {
            // if it doesn't continue, it is integer
            case 0:
                listAdd(tokens, fileSpanIntToken(T_LITERAL_INTEGER, isNegative ? -num : num, span), Token);
                if (overflow)
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
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
                listAdd(tokens, fileSpanDecToken(T_LITERAL_FLOAT, isNegative ? -decimal : decimal, span), Token);
                // check for too large or precise numbers
                if (isinf(decimal))
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number is too large and will be trated as infinity", "use different type (string?)"), ErrorSpan) // there shouldn't be ;
                else if (digits > DECIMAL_WARNING_LIMIT)
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number has too many digits and may be rounded", "if you want all the digits maybe use string"), ErrorSpan) // there shouldn't be ;
                else
                    freeFileSpan(span);
                continue;
            }
            // reading hexadecimal numbers
            case 'x':
                num = _readInt(c + 1, &c, 16, &overflow);
                if (*c)
                    break;
                listAdd(tokens, fileSpanIntToken(T_LITERAL_INTEGER, isNegative ? -num : num, span), Token);
                if (overflow)
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else freeFileSpan(span);
                continue;
            // reading binary numbers
            case 'b':
                num = _readInt(c + 1, &c, 2, &overflow);
                if (*c)
                    break;
                listAdd(tokens, fileSpanIntToken(T_LITERAL_INTEGER, isNegative ? -num : num, span), Token);
                if (overflow)
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else freeFileSpan(span);
                continue;
            // reding numbers with the specified base
            case 'z':
                if (num < 2 || num > 36)
                {
                    listAdd(errs, createErrorSpan(E_ERROR, span, "base must be between 2 and 36 (inclusive)", "change the number before z to be in that range"), ErrorSpan);
                    continue;
                }
                num = _readInt(c + 1, &c, num, &overflow);
                if (*c)
                    break;
                listAdd(tokens, fileSpanIntToken(T_LITERAL_INTEGER, isNegative ? -num : num, span), Token);
                if (overflow)
                    listAdd(errs, createErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else freeFileSpan(span);
                continue;
            // otherwise break into error
            default:
                break;
            }
            listAdd(errs, createErrorSpan(E_ERROR, span, "invalid number literal", "number literals cannot contain other characters than digits and single ."), ErrorSpan);
            continue;
        }

        // tokens that are not enclosed in [] are incorrect
        if (nest == 0)
        {
            listAdd(errs, createErrorSpan(E_ERROR, span, "cannot use identifiers directly", "try ecapsulating it in []"), ErrorSpan);
            continue;
        }

#define __checkKeyword(__kws, __kwe) if(_checkKeyword(__kws,__kwe,span,&tokens,&errs))continue
        // check for the keywords
        __checkKeyword("struct", T_KEYWORD_STRUCT);
        __checkKeyword("set", T_KEYWORD_SET);
        switch (_checkKeyword("sign", T_KEYWORD_SIGN, span, &tokens, &errs))
        {
        case 0:
            break;
        case 1:
            if (defd == -1 && parm == -1)
                defd = nest;
            continue;
        default:
            continue;
        }
        switch (_checkKeyword("def", T_KEYWORD_DEF, span, &tokens, &errs))
        {
        case 0:
            break;
        case 1:
            if (defd == -1 && parm == -1)
                parm = nest + 1;
            continue;
        default:
            continue;
        }
#undef __checkKeyword

        if (strcmp(span.str, "true") == 0)
        {
            listAdd(tokens, fileSpanBoolToken(T_LITERAL_BOOL, true, span), Token);
            continue;
        }
        if (strcmp(span.str, "false") == 0)
        {
            listAdd(tokens, fileSpanBoolToken(T_LITERAL_BOOL, false, span), Token);
            continue;
        }

        if (defd != -1)
        {
            if (defd == nest)
            {
                Token* t = listGetP(tokens, tokens.length - 1);
                if (t->type == T_IDENTIFIER_STRUCT)
                    t->type = T_IDENTIFIER_VARIABLE;
            }
            listAdd(tokens, fileSpanToken(T_IDENTIFIER_STRUCT, span), Token);
            continue;
        }

        if (parm != -1)
        {
            if (parm > nest && listGet(tokens, tokens.length - 1, Token).type == T_PUNCTUATION_BRACKET_OPEN)
            {
                listAdd(tokens, fileSpanToken(T_IDENTIFIER_STRUCT, span), Token);
                continue;
            }
            listAdd(tokens, fileSpanToken(T_IDENTIFIER_PARAMETER, span), Token);
            continue;
        }

        // determine token type based on previous tokens
        switch (listGet(tokens, tokens.length - 1, Token).type)
        {
        // tokens directly after [ are function identifiers
        case T_PUNCTUATION_BRACKET_OPEN:
            listAdd(tokens, fileSpanToken(T_IDENTIFIER_FUNCTION, span), Token);
            continue;
        // tokens after struct keyword are type identifiers
        case T_KEYWORD_STRUCT:
            listAdd(tokens, fileSpanToken(T_IDENTIFIER_STRUCT, span), Token);
            continue;
        // other tokens are just variable identifiers
        default:
            listAdd(tokens, fileSpanToken(T_IDENTIFIER_VARIABLE, span), Token);
            continue;
        }
    }

    if (nest > 0)
        listAdd(errs, createErrorSpan(E_ERROR, copyFileSpanFrom("]", 1, 
            listGet(list, list.length - 1, FileSpan).line, 
            listGet(list, list.length - 1, FileSpan).col
            ), "missing 1 or more closing brackets", "try adding ]"), ErrorSpan);

    // free the list of strings
    freeList(list);
    // if errors is not null set them, otherwise free them
    if (errors)
        *errors = errs;
    else
        listDeepFree(list, ErrorSpan, t, freeErrorSpan(t));

    return tokens;
}

long long _getDigit(char digit)
{
    if (digit < '0')
        return 37;
    if (digit <= '9')
        return digit - '0';
    if (digit < 'A')
        return 37;
    if (digit <= 'Z')
        return digit - 'A' + 10;
    if (digit < 'a')
        return 37;
    return digit - 'a' + 10;
}

long long _readInt(char* str, char** endptr, long long base, bool* overflow)
{
    assert(str, "_readInt: parameter str was null");
    assert(base <= 36 && base >= 2, "_readInt: parameter base was out of range (%I64d)", base);

    long long num = 0;
    long long digit;
    bool ovfl = false;
    for (; *str && (digit = _getDigit(*str)) < base; str++)
    {
        if (num > LONG_LONG_MAX / base || (num == LONG_LONG_MAX / base && digit > LONG_LONG_MAX % base))
            ovfl = true;
        num = num * base + digit;
    }
    if (endptr)
        *endptr = str;
    if (overflow)
        *overflow = ovfl;
    return num;
}

int _checkKeyword(const char* kw, TokenType type, FileSpan span, List* tokens, List* errors)
{
    assert(kw, "_checkKeyword: parameter kw was null");
    assert(tokens, "_checkKeyword: parameter tokens was null");
    assert(errors, "_checkKeyword: parameter errors was null");

    if (strcmp(kw, span.str) == 0)
    {
        if (listGet(*tokens, tokens->length - 1, Token).type != T_PUNCTUATION_BRACKET_OPEN)
        {
            ErrorSpan t = createErrorSpan(E_ERROR, span, "keyword must be used as function", "try encapsulating the action in []");
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
        case 'x':
        {
            char buf[3];
            buf[2] = 0;
            (*col)++;
            if ((chr = fgetc(in)) == EOF)
            {
                buffer[pos] = 'x';
                return pos + 1;
            }
            buf[0] = chr;
            (*col)++;
            if ((chr = fgetc(in)) == EOF)
            {
                buffer[pos] = 'x';
                pos++;
                if (pos >= bufferSize)
                    except("_readQuote: :%I64d:%I64d: quoted token is too long, max size is %I64d", *line, *col, bufferSize);
                buffer[pos] = buf[0];
                return pos + 1;
            }
            buf[1] = chr;
            char* end;
            buffer[pos] = _readInt(buf, &end, 16, NULL);
            if (*end)
                except("_readQuote: :%I64d:%I64d: inavlid escape sequence", *line, *col);
            break;
        }
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