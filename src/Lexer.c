#include "Lexer.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include "Token.h"
#include "FileSpan.h"
#include "List.h"
#include "Errors.h"
#include "DebugTools.h"

/**
 * @brief splits the file contents into FileSpans
 * 
 * @param in File to read from
 * @return List list of FileSpans
 */
List _lexTokenize(FILE* in, String* filename);

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
size_t _lexReadQuote(FILE* in, char* buffer, size_t bufferSize, int quoteChar, size_t* line, size_t* col);

/**
 * @brief reads positive int from string
 * 
 * @param str string to read from
 * @param endptr pointer to the first non-digit char
 * @param base base in which to convert
 * @param overflow this is set to true if the integer overflows the long long limit
 * @return long readed integer
 */
long long _lexReadInt(char* str, char** endptr, long long base, _Bool* overflow);

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
int _lexCheckKeyword(const char* kw, T_TokenType type, FileSpan span, List* tokens, List* errors);

/**
 * @brief converts the given char to numnerical digit
 * 
 * @param digit char to convert
 * @return long the number it represents
 */
long long _lexGetDigit(char digit);

List lexLex(FILE* in, List* errors, String* filename)
{
    assert(in);
    assert(errors);

    // read from file and prepare output lists
    List list = _lexTokenize(in, filename);
    List errs = listNew(ErrorSpan);
    List tokens = listNew(Token);

    long long nest = 0;
    long long defd = -1;
    long long parm = -1;
    long long strc = -1;

    for (size_t i = 0; i < list.length; i++)
    {
        // get tje string to examine
        FileSpan span = listGet(list, i, FileSpan);
        // check for tokens that can be recognized by their first few characters
        switch (span.str.c[0])
        {
        // this case should never happen
        case 0:
            dtPrintf("lex: empty token at position :%zu:%zu", span.pos.line, span.pos.col);
            fsFree(span);
            continue;
        // [ is always token by itself
        case '[':
            assert(span.str.length == 1);
            if (defd == nest)
            {
                Token* t = listGetP(tokens, tokens.length - 1);
                if (t->type == T_IDENTIFIER_STRUCT)
                    t->type = T_IDENTIFIER_FUNCTION;
            }
            listAdd(tokens, tokenInt(T_PUNCTUATION_BRACKET_OPEN, nest, span.pos), Token);
            fsFree(span);
            nest++;
            continue;
        // ] is always token by itself
        case ']':
            assert(span.str.length == 1);
            if (nest == defd || nest == parm || nest == strc)
            {
                defd = -1;
                parm = -1;
                strc = -1;
            }
            if (nest == 0)
            {
                listAdd(errs, errCreateErrorSpan(E_ERROR, span, "missing [ before ]", "add opening bracket somwhere before this closing one"), ErrorSpan)
                continue;
            }
            nest--;
            listAdd(tokens, tokenInt(T_PUNCTUATION_BRACKET_CLOSE, nest, span.pos), Token);
            fsFree(span);
            continue;
        // check for comments (starts with // or /*)
        case '/':
            switch(span.str.c[1])
            {
            // line comment
            case '/':
                //listAdd(tokens, fileSpanTokenPart(COMMENT_LINE, span, 2, span.length - 2), Token);
                fsFree(span);
                continue;
            // block comment
            case '*':
                // check if the block comment is closed
                if (span.str.c[span.str.length - 1] != '/' || span.str.c[span.str.length - 2] != '*')
                {
                    listAdd(errs, errCreateErrorSpan(E_ERROR, span, "block comment is not closed", "close it with */"), ErrorSpan);
                    continue;
                }
                //listAdd(tokens, fileSpanTokenPart(COMMENT_BLOCK, span, 2, span.length < 5 ? 0 : span.length - 4), Token);
                fsFree(span);
                continue;
            default:
                break;
            }
            break;
        // string literal
        case '"':
            // check if the string literal is ended
            if (span.str.length == 1 || span.str.c[span.str.length - 1] != '"')
            {
                listAdd(errs, errCreateErrorSpan(E_ERROR, span, "string literal is not closed", "try adding closing \""), ErrorSpan);
                continue;
            }
            listAdd(tokens, tokenFileSpanPart(T_LITERAL_STRING, span, 1, span.str.length - 2), Token);
            fsFree(span);
            continue;
        // char literal
        case '\'':
            // check if the char literal has only one character
            if (span.str.length != 3)
            {
                listAdd(errs, errCreateErrorSpan(E_ERROR, span, "char literal can only contain one character", "maybe you want to use string (\")"), ErrorSpan);
                continue;
            }
            // check if the char literal is closed
            else if (span.str.c[3] != '\'')
            {
                listAdd(errs, errCreateErrorSpan(E_ERROR, span, "char literal is not closed", "try adding closing '"), ErrorSpan);
                continue;
            }
            listAdd(tokens, tokenChar(T_LITERAL_CHAR, span.str.c[1], span.pos), Token);
            fsFree(span);
            continue;
        // nothing operator
        case '_':
            // check if it is only the operator
            if (span.str.length != 1)
                break;
            listAdd(tokens, tokenCreate(T_OPERATOR_NOTHING, span.pos), Token);
            fsFree(span);
            continue;
        default:
            break;
        }
        // checking numbers
        if (isdigit(span.str.c[0]) || (span.str.c[0] == '-' && isdigit(span.str.c[1])))
        {
            char* c = span.str.c;
            _Bool overflow = 0;
            // check for negative values
            _Bool isNegative = *c == '-';
            // read whole part values
            intmax_t num = _lexReadInt(c + isNegative, &c, 10, &overflow);
            switch (*c)
            {
            // if it doesn't continue, it is integer
            case 0:
                listAdd(tokens, tokenInt(T_LITERAL_INTEGER, isNegative ? -num : num, span.pos), Token);
                if (overflow)
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                else
                    fsFree(span);
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
                    for (c = span.str.c + isNegative; *c && isdigit(*c); c++, digits++)
                        decimal = decimal * 10 + *c - '0';
                }
                double div = 10;
                for (c++; *c && isdigit(*c); c++, div *= 10, digits++)
                    decimal += (*c - '0') / div;
                // if this is not end break into error
                if (*c)
                    break;
                listAdd(tokens, tokenFloat(T_LITERAL_FLOAT, isNegative ? -decimal : decimal, span.pos), Token);
                // check for too large or precise numbers
                if (isinf(decimal))
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number is too large and will be trated as infinity", "use different type (string?)"), ErrorSpan) // there shouldn't be ;
                else if (digits > lex_DECIMAL_WARNING_LIMIT)
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number has too many digits and may be rounded", "if you want all the digits maybe use string"), ErrorSpan) // there shouldn't be ;
                else
                    fsFree(span);
                continue;
            }
            // reading hexadecimal numbers
            case 'x':
                num = _lexReadInt(c + 1, &c, 16, &overflow);
                if (*c)
                    break;
                listAdd(tokens, tokenInt(T_LITERAL_INTEGER, isNegative ? -num : num, span.pos), Token);
                if (overflow)
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else fsFree(span);
                continue;
            // reading binary numbers
            case 'b':
                num = _lexReadInt(c + 1, &c, 2, &overflow);
                if (*c)
                    break;
                listAdd(tokens, tokenInt(T_LITERAL_INTEGER, isNegative ? -num : num, span.pos), Token);
                if (overflow)
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else fsFree(span);
                continue;
            // reding numbers with the specified base
            case 'z':
                if (num < 2 || num > 36)
                {
                    listAdd(errs, errCreateErrorSpan(E_ERROR, span, "base must be between 2 and 36 (inclusive)", "change the number before z to be in that range"), ErrorSpan);
                    continue;
                }
                num = _lexReadInt(c + 1, &c, num, &overflow);
                if (*c)
                    break;
                listAdd(tokens, tokenInt(T_LITERAL_INTEGER, isNegative ? -num : num, span.pos), Token);
                if (overflow)
                    listAdd(errs, errCreateErrorSpan(E_WARNING, span, "number is too large", "make the number smaller or use defferent type"), ErrorSpan) // there shouldn't be ;
                        else fsFree(span);
                continue;
            // otherwise break into error
            default:
                break;
            }
            listAdd(errs, errCreateErrorSpan(E_ERROR, span, "invalid number literal", "number literals cannot contain other characters than digits and single ."), ErrorSpan);
            continue;
        }

        // tokens that are not enclosed in [] are incorrect
        if (nest == 0)
        {
            listAdd(errs, errCreateErrorSpan(E_ERROR, span, "cannot use identifiers directly", "try ecapsulating it in []"), ErrorSpan);
            continue;
        }

#define __lexCheckKeyword(__kws, __kwe) if(_lexCheckKeyword(__kws,__kwe,span,&tokens,&errs))continue
        // check for the keywords
        __lexCheckKeyword("set", T_KEYWORD_SET);
        switch (_lexCheckKeyword("struct", T_KEYWORD_STRUCT, span, &tokens, &errs))
        {
        case 0:
            break;
        case 1:
            if (defd == -1 && parm == -1 && strc == -1)
                strc = nest;
            continue;
        default:
            continue;
        }
        switch (_lexCheckKeyword("sign", T_KEYWORD_SIGN, span, &tokens, &errs))
        {
        case 0:
            break;
        case 1:
            if (defd == -1 && parm == -1 && strc == -1)
                defd = nest;
            continue;
        default:
            continue;
        }
        switch (_lexCheckKeyword("def", T_KEYWORD_DEF, span, &tokens, &errs))
        {
        case 0:
            break;
        case 1:
            if (defd == -1 && parm == -1 && strc == -1)
                parm = nest + 1;
            continue;
        default:
            continue;
        }
#undef __checkKeyword

        if (strcmp(span.str.c, "true") == 0)
        {
            listAdd(tokens, tokenBool(T_LITERAL_BOOL, 1, span.pos), Token);
            continue;
        }
        if (strcmp(span.str.c, "false") == 0)
        {
            listAdd(tokens, tokenBool(T_LITERAL_BOOL, 0, span.pos), Token);
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
#define __lexCheckStorage(__str, __tpe) if(strcmp(span.str.c, __str) == 0){Token __t=tokenCreate(__tpe, span.pos);listAddP(&tokens, &__t);continue;}
            __lexCheckStorage("*", T_STORAGE_POINTER);
            __lexCheckStorage("char", T_STORAGE_CHAR);
            __lexCheckStorage("string", T_STORAGE_STRING);
            __lexCheckStorage("int", T_STORAGE_INT);
            __lexCheckStorage("float", T_STORAGE_FLOAT);
            __lexCheckStorage("bool", T_STORAGE_BOOL);
#undef __checkStorage
            listAdd(tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, span), Token);
            continue;
        }

        if (parm != -1)
        {
            if (parm > nest && listGet(tokens, tokens.length - 1, Token).type == T_PUNCTUATION_BRACKET_OPEN)
            {
                listAdd(tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, span), Token);
                continue;
            }
            listAdd(tokens, tokenFileSpan(T_IDENTIFIER_PARAMETER, span), Token);
            continue;
        }

        if (strc != -1)
        {
            if (strc == nest)
            {
                listAdd(tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, span), Token);
                continue;
            }
            switch (listGet(tokens, tokens.length - 1, Token).type)
            {
            case T_PUNCTUATION_BRACKET_OPEN:
#define __checkStorage(__str, __tpe) if(strcmp(span.str.c, __str) == 0){Token __t=tokenCreate(__tpe, span.pos);listAddP(&tokens, &__t);continue;}
                __lexCheckStorage("*", T_STORAGE_POINTER);
                __lexCheckStorage("char", T_STORAGE_CHAR);
                __lexCheckStorage("string", T_STORAGE_STRING);
                __lexCheckStorage("int", T_STORAGE_INT);
                __lexCheckStorage("float", T_STORAGE_FLOAT);
                __lexCheckStorage("bool", T_STORAGE_BOOL);
#undef __checkStorage
                listAdd(tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, span), Token);
                continue;
            default:
                listAdd(tokens, tokenFileSpan(T_IDENTIFIER_PARAMETER, span), Token);
                continue;
            }
        }

        // determine token type based on previous tokens
        switch (listGet(tokens, tokens.length - 1, Token).type)
        {
        // tokens directly after [ are function identifiers
        case T_PUNCTUATION_BRACKET_OPEN:
            listAdd(tokens, tokenFileSpan(T_IDENTIFIER_FUNCTION, span), Token);
            continue;
        // other tokens are just variable identifiers
        default:
            listAdd(tokens, tokenFileSpan(T_IDENTIFIER_VARIABLE, span), Token);
            continue;
        }
    }

    if (nest > 0)
        listAdd(errs, errCreateErrorSpan(E_ERROR, fsCreate(strLit("]"), 
            listGet(list, list.length - 1, FileSpan).pos
            ), "missing 1 or more closing brackets", "try adding ]"), ErrorSpan);

    // free the list of strings
    listFree(list);
    // if errors is not null set them, otherwise free them
    if (errors)
        *errors = errs;
    else
        listDeepFree(list, ErrorSpan, t, errFreeErrorSpan(t));

    return tokens;
}

intmax_t _lexGetDigit(char digit)
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

intmax_t _lexReadInt(char* str, char** endptr, intmax_t base, _Bool* overflow)
{
    assert(str);
    assert(base <= 36 && base >= 2);

    intmax_t num = 0;
    intmax_t digit;
    _Bool ovfl = 0;
    for (; *str && (digit = _lexGetDigit(*str)) < base; str++)
    {
        if (num > INTMAX_MAX / base || (num == INTMAX_MAX / base && digit > INTMAX_MAX % base))
            ovfl = 1;
        num = num * base + digit;
    }
    if (endptr)
        *endptr = str;
    if (overflow)
        *overflow = ovfl;
    return num;
}

int _lexCheckKeyword(const char* kw, T_TokenType type, FileSpan span, List* tokens, List* errors)
{
    assert(kw);
    assert(tokens);
    assert(errors);

    if (strcmp(kw, span.str.c) == 0)
    {
        if (listGet(*tokens, tokens->length - 1, Token).type != T_PUNCTUATION_BRACKET_OPEN)
        {
            ErrorSpan t = errCreateErrorSpan(E_ERROR, span, "keyword must be used as function", "try encapsulating the action in []");
            listAddP(errors, &t);
            return -1;
        }
        Token t = tokenCreate(type, span.pos);
        listAddP(tokens, &t);
        return 1;
    }
    return 0;
}

List _lexTokenize(FILE* in, String* filename)
{
    assert(in);
    _Static_assert(lex_LEXER_READ_BUFFER_SIZE > 1, "_tokenize: minimum LEXER_READ_BUFFER_SIZE is 2");

    List list = listNew(FileSpan);

    char buffer[lex_LEXER_READ_BUFFER_SIZE];
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
                listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(line, col - pos, filename)), FileSpan);
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
            listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(line, col - pos, filename)), FileSpan);
            pos = 0;
            continue;
        // brackets are always token by them self
        case '[':
        case ']':
            // end any currently readed token
            if (pos != 0)
            {
                listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(line, col - pos, filename)), FileSpan);
                pos = 0;
            }
            // read the bracket
            {
                char character = chr;
                listAdd(list, fsCreate(strCLen(&character, 1), fpCreate(line, col, filename)), FileSpan);
            }
            continue;
        // strings and chars are in single or double quotes
        case '"':
        case '\'':
            // if this is not first char of token, it is not string / char literal
            if (pos != 0)
            {
                if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                    dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
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
                pos = _lexReadQuote(in, buffer, lex_LEXER_READ_BUFFER_SIZE, chr, &line, &col);
                listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(qLine, qCol, filename)), FileSpan);
                pos = 0;
            }
            continue;
        // single line comments start with //
        case '/':
            // continue reading if there are not two /
            if (pos == 0 || buffer[pos - 1] != '/')
            {
                if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                    dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
                buffer[pos] = '/';
                pos++;
                continue;
            }
            // comments immidietly end the token, the / is not part of the token
            if (pos > 1)
                listAdd(list, fsCreate(strCLen(buffer, pos - 1), fpCreate(line, col - pos, filename)), FileSpan);
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
                    listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(line, col - pos, filename)), FileSpan);
                    pos = 0;
                    line++;
                    col = 0;
                    break;
                }
                if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                    dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
                buffer[pos] = chr;
                pos++;
            }
            continue;
        // block comments start with /* and end with */ (/*/ is valid block comment)
        case '*':
            // checking for the begginging of the comment
            if (pos == 0 || buffer[pos - 1] != '/')
            {
                if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                    dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
                buffer[pos] = '*';
                pos++;
                continue;
            }
            // comments immidietly end any previous token (/ is not part of the token)
            if (pos > 1)
                listAdd(list, fsCreate(strCLen(buffer, pos - 1), fpCreate(line, col - pos, filename)), FileSpan);
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
                    if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                        dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
                    buffer[pos] = chr;
                    pos++;
                    // check for the */ that ends the comment
                    if (chr == '/' && buffer[pos - 2] == '*')
                    {
                        listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(comLine, comCol, filename)), FileSpan);
                        pos = 0;
                        break;
                    }
                }
            }
            continue;
        // read any nonspecial characters
        default:
            if (pos >= lex_LEXER_READ_BUFFER_SIZE)
                dtExcept("_tokenize: token is too long, max size is %zu", lex_LEXER_READ_BUFFER_SIZE);
            buffer[pos] = chr;
            pos++;
            continue;
        }
    }
    // if file ends, read the last readed token
    if (pos != 0)
        listAdd(list, fsCreate(strCLen(buffer, pos), fpCreate(line, col - pos, filename)), FileSpan);
    return list;
}

size_t _lexReadQuote(FILE* in, char* buffer, size_t bufferSize, int quoteChar, size_t* line, size_t* col)
{
    assert(in);
    assert(buffer);
    assert(bufferSize > 1);
    assert(line);
    assert(col);

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
            dtExcept("_readQuote: quoted token is too long, max size is %zu", bufferSize);
        
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
                    dtExcept("_readQuote: :%zu:%zu: quoted token is too long, max size is %zu", *line, *col, bufferSize);
                buffer[pos] = buf[0];
                return pos + 1;
            }
            buf[1] = chr;
            char* end;
            buffer[pos] = _lexReadInt(buf, &end, 16, NULL);
            if (*end)
                dtExcept("_readQuote: :%zu:%zu: inavlid escape sequence", *line, *col);
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