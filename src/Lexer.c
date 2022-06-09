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
#include "Stream.h"
#include "StringBuilder.h"

typedef struct _LexTContext
{
    Stream* in;
    FilePos pos;
    StringBuilder sb;
    List spans;
    List errors;
} _LexTContext;

/**
 * @brief splits the file contents into FileSpans
 * 
 * @param in File to read from
 * @return List list of FileSpans
 */
List _lexTokenize(Stream* in, String* filename);

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

/**
 * @brief adds currentyl readed token if any and moves to the next line
 * 
 * @param ltc context
 */
void _lexOnNewline(_LexTContext* restrict ltc);

/**
 * @brief moves position to the next line
 * 
 * @param ltc where to move the position
 */
void _lexNewline(_LexTContext* restrict ltc);

/**
 * @brief adds string from the stringbuilder to spans list
 * 
 * @param ltc context
 */
void _lexSbAdd(_LexTContext* restrict ltc);

/**
 * @brief gets the position of the token currently in the sb
 * 
 */
FilePos _lexGetPos(_LexTContext* restrict ltc);

/**
 * @brief adds currently readed token if any
 * 
 * @param ltc context
 */
void _lexOnWhitespace(_LexTContext* restrict ltc);

/**
 * @brief adds currently readed token and the bracket
 * 
 * @param ltc context
 * @param chr bracket that was readed
 */
void _lexOnBracket(_LexTContext* restrict ltc, char chr);

/**
 * @brief if this is first character of token reads quote literal, otherwise reads character
 * 
 * @param ltc context
 * @param chr currently readed character
 */
void _lexOnQuote(_LexTContext* restrict ltc, int chr);

/**
 * @brief reads escaped character
 * 
 * @param ltc context
 * @param qchr endqote character
 * @return int last unused character
 */
int _lexOnEscape(_LexTContext* restrict ltc, int qchr);

/**
 * @brief reads \x escape code
 * 
 * @param ltc context
 * @param qchr endquote character
 * @return int last unused character
 */
int _lexOnHexEscape(_LexTContext* restrict ltc, int qchr);

/**
 * @brief reads comments or just returns the next character
 * 
 * @param ltc context
 * @return int the last unused character
 */
int _lexOnSlash(_LexTContext* restrict ltc);

/**
 * @brief reads line comment
 * 
 * @param ltc context
 */
void _lexReadLineComment(_LexTContext* restrict ltc);

/**
 * @brief reads block comment
 * 
 * @param ltc context
 */
void _lexReadBlockComment(_LexTContext* restrict ltc);

List lexLex(Stream* in, List* errors, String* filename)
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

List _lexTokenize(Stream* in, String* filename)
{
    assert(in);
    assert(filename);

    _LexTContext context =
    {
        .errors = listNew(ErrorSpan),
        .in = in,
        .pos = fpCreate(1, 0, filename),
        .sb = sbCreate(),
        .spans = listNew(FileSpan),
    };
    _LexTContext* restrict ltc = &context;

    // proccessing char by char
    int chr;
    while ((chr = stGetChar(in)) != EOF)
    {
    continueNoRead:
        // each char changes column in file by 1
        ltc->pos.col++;

        switch (chr)
        {
        // newline ends any currently readed token and updates position in file
        case '\n':
            _lexOnNewline(ltc);
            continue;
        // whitespaces end any currently readed token
        case ' ':
        case '\t':
        case '\r':
            _lexOnWhitespace(ltc);
            continue;
        // brackets are always token by them self
        case '[':
        case ']':
            _lexOnBracket(ltc, chr);
            continue;
        // strings and chars are in single or double quotes
        case '"':
        case '\'':
            _lexOnQuote(ltc, chr);
            continue;
        // single comments start with // or /*
        case '/':
            chr = _lexOnSlash(ltc);
            if (chr < 0)
                continue;
            goto continueNoRead;
        // read any nonspecial characters
        default:
            sbAdd(&ltc->sb, chr);
            continue;
        }
    }
    // if file ends, read the last readed token
    if (ltc->sb.length != 0)
        _lexSbAdd(ltc);
    
    // for now the error list is always empty
    listDeepFree(ltc->errors, ErrorSpan, e, errFreeErrorSpan(e));
    sbFree(&ltc->sb);

    return ltc->spans;
}

void _lexOnQuote(_LexTContext* restrict ltc, int qchr)
{
    // the first quote is part of the token
    sbAdd(&ltc->sb, qchr);

    // if this is not first char of token, it is not string / char literal
    if (ltc->sb.length != 1)
        return;

    // save the beggining of the token
    FilePos tPos = ltc->pos;

    int chr;
    while ((chr = stGetChar(ltc->in)) >= 0)
    {
    continueNoRead:
        // keep track of position in file
        ltc->pos.col++;

        switch (chr)
        {
        case '\\':
            chr = _lexOnEscape(ltc, qchr);
            if (chr < 0)
                return;
            goto continueNoRead;
        case '\n':
            _lexNewline(ltc);
        default:
            sbAdd(&ltc->sb, chr);
            break;
        }
        
        // return
        if (chr == qchr)
        {
            listAdd(ltc->spans, fsCreate(sbGet(&ltc->sb), tPos), FileSpan);
            sbClear(&ltc->sb);
            return;
        }
    }
}

void _lexOnNewline(_LexTContext* restrict ltc)
{
    if (ltc->sb.length != 0)
        _lexSbAdd(ltc);
    _lexNewline(ltc);
}

void _lexNewline(_LexTContext* restrict ltc)
{
    ltc->pos.line++;
    ltc->pos.col = 0;
}

void _lexSbAdd(_LexTContext* restrict ltc)
{
    listAdd(ltc->spans, fsCreate(sbGet(&ltc->sb), _lexGetPos(ltc)), FileSpan);
    sbClear(&ltc->sb);
}

FilePos _lexGetPos(_LexTContext* restrict ltc)
{
    FilePos pos = ltc->pos;
    pos.col -= ltc->sb.length;
    return pos;
}

void _lexOnWhitespace(_LexTContext* restrict ltc)
{
    if (ltc->sb.length == 0)
        return;
    _lexSbAdd(ltc);
}

void _lexOnBracket(_LexTContext* restrict ltc, char chr)
{
    // end any currently readed token
    if (ltc->sb.length != 0)
        _lexSbAdd(ltc);
    // read the bracket
    listAdd(ltc->spans, fsCreate(strCLen(&chr, 1), ltc->pos), FileSpan);
}

int _lexOnEscape(_LexTContext* restrict ltc, int qchr)
{
    // reading the escaped character
    char chr;
    if ((chr = stGetChar(ltc->in)) < 0)
        return chr;
    ltc->pos.col++;

    char toRead;
    switch (chr)
    {
    // NULL is escaped with \0
    case '0':
        toRead = '\0';
        break;
    // newline is escaped with \n
    case 'n':
        toRead = '\n';
        break;
    // carrige return is escaped with \r
    case 'r':
        toRead = '\r';
        break;
    // tab is escaped with \t
    case 't':
        toRead = '\t';
        break;
    case 'x':
        return _lexOnHexEscape(ltc, qchr);
    // any other character after \ will be readed literaly (\\, \")
    case '\n':
        _lexNewline(ltc);
    default:
        toRead = chr;
        break;
    }

    sbAdd(&ltc->sb, toRead);

    return stGetChar(ltc->in);
}

int _lexOnHexEscape(_LexTContext* restrict ltc, int qchr)
{
    const int base = 16;
    //_Static_assert(base < 36 || base >= 2, "base must be in range 2 - 36");
    //_Static_assert(base == 16, "this algorithm may work unexpectedly with bases other than 16");

    StringBuilder sb = sbCreate();

    // read all characters in the base
    int chr;
    while ((chr = stGetChar(ltc->in)) >= 0)
    {
        ltc->pos.col++;
        if (chr == qchr)
            break;
        char dig = _lexGetDigit(chr);
        if (dig >= base)
            break;
        sbAdd(&sb, dig);
    }

    // convert the base characters into real characters
    int i = 0;
    if (sb.length & 1)
    {
        i = 1;
        sbAdd(&ltc->sb, sb.buffer[0]);
    }
    for (; i < sb.length; i += 2)
        sbAdd(&ltc->sb, sb.buffer[i] * base + sb.buffer[i + 1]);

    sbFree(&sb);

    // return the last unreaded character
    return chr;
}

int _lexOnSlash(_LexTContext* restrict ltc)
{
    int chr;
    if ((chr = stGetChar(ltc->in)) < 0)
    {
        sbAdd(&ltc->sb, '/');
        return chr;
    }

    ltc->pos.col++;

    if (ltc->sb.length != 0)
        _lexSbAdd(ltc);

    sbAdd(&ltc->sb, '/');

    switch (chr)
    {
    case '/':
        _lexReadLineComment(ltc);
        break;
    case '*':
        _lexReadBlockComment(ltc);
        break;
    default:
        return chr;
    }

    return stGetChar(ltc->in);
}

void _lexReadLineComment(_LexTContext* restrict ltc)
{
    sbAdd(&ltc->sb, '/');

    int chr;
    while ((chr = stGetChar(ltc->in)) >= 0)
    {
        ltc->pos.col++;
        sbAdd(&ltc->sb, chr);

        if (chr == '\n')
        {
            _lexSbAdd(ltc);
            _lexNewline(ltc);
            return;
        }
    }
}

void _lexReadBlockComment(_LexTContext* restrict ltc)
{
    sbAdd(&ltc->sb, '*');

    FilePos tPos = ltc->pos;
    tPos.col -= 2;

    int chr;
    while ((chr = stGetChar(ltc->in)) >= 0)
    {
        ltc->pos.col++;
        sbAdd(&ltc->sb, chr);

        if (chr == '\n')
        {
            _lexNewline(ltc);
            continue;
        }

        if (chr == '/' && ltc->sb.buffer[ltc->sb.length - 2] == '*')
        {
            listAdd(ltc->spans, fsCreate(sbGet(&ltc->sb), tPos), FileSpan);
            sbClear(&ltc->sb);
            return;
        }
    }
}