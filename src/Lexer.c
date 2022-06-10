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

#define _lexError(context, level, msg, help) listAdd((context)->err, errCreateErrorSpan(level, (context)->span, strLit(msg), strLit(help)), ErrorSpan)

typedef struct _LexLContext
{
    List err;
    ListIterator spans;
    FileSpan span;
    List tokens;
    long nest;
    long defd;
    long parm;
    long strc;
} _LexLContext;

typedef struct _LexTContext
{
    Stream* in;
    FilePos pos;
    StringBuilder sb;
    List spans;
    List err;
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
 * @param digits add here 1 for each digit
 * @return long readed integer
 */
long long _lexReadInt(char* str, char** endptr, long long base, _Bool* overflow, size_t* digits);

/**
 * @brief adds keyword token
 * 
 * @param llc context
 * @param kw keyword to check
 * @param type type of the keyword
 * @param nest nest flag or NULL
 * @param nestOff offset for nest flag
 * @return _Bool true -> keyword added, false -> not the keyword
 */
_Bool _lexCheckKeyword(_LexLContext* restrict llc, String kw, T_TokenType type, long* nest, long nestOff);

/**
 * @brief converts the given char to numnerical digit
 * 
 * @param digit char to convert
 * @return long the number it represents
 */
long long _lexGetDigit(char digit);

/**
 * @brief adds open bracket token and works with defd
 * 
 * @param llc context
 */
void _lexOnTOpen(_LexLContext* restrict llc);

/**
 * @brief adds close bracket token and resets nests
 * 
 * @param llc context
 */
void _lexOnTClose(_LexLContext* restrict llc);

/**
 * @brief skips comment token or returns false
 * 
 * @param llc context
 * @return _Bool true -> token skiped, false -> token not comment
 */
_Bool _lexOnTComment(_LexLContext* restrict llc);

/**
 * @brief adds string literal ltoken
 * 
 * @param llc context
 */
void _lexOnTString(_LexLContext* restrict llc);

/**
 * @brief adds char literal token
 * 
 * @param llc context
 */
void _lexOnTChar(_LexLContext* restrict llc);

/**
 * @brief adds nothing token
 * 
 * @param llc context
 * @return _Bool true -> added, flase -> not nothing token
 */
_Bool _lexOnT_(_LexLContext* restrict llc);

/**
 * @brief adds number literal token
 * 
 * @param llc context
 * @return _Bool true -> added, false -> not number token
 */
_Bool _lexOnTNumber(_LexLContext* restrict llc);

/**
 * @brief adds integer literal token
 * 
 * @param llc context
 * @param num parsed number
 * @param overflow if overflow occured
 */
void _lexOnTInt(_LexLContext* restrict llc, intmax_t num, _Bool overflow);

/**
 * @brief adds float literal token
 * 
 * @param llc context
 * @param num parsed integer part
 * @param isNegative true of the number is negative
 * @param overflow if overflow occured
 * @param digits number of digits of the integer part
 * @param c points to the decimal part
 */
void _lexOnTFloat(_LexLContext* restrict llc, intmax_t num, _Bool isNegative, _Bool overflow, size_t digits, char* c);

/**
 * @brief adds integer literal with special base
 * 
 * @param llc context
 * @param base the base
 * @param c wtart of the integer
 * @param isNegative if the number is negative
 */
void _lexOnTBase(_LexLContext* restrict llc, intmax_t base, char* c, _Bool isNegative);

/**
 * @brief adds keyword token
 * 
 * @param llc context
 * @return _Bool true -> added, false -> not a keyword
 */
_Bool _lexOnTKeyword(_LexLContext* restrict llc);

/**
 * @brief adds bool literal token
 * 
 * @param llc context
 * @return _Bool true -> added, false -> not a bool literal
 */
_Bool _lexOnTBool(_LexLContext* restrict llc);

/**
 * @brief adds defd specified token
 * 
 * @param llc context
 */
void _lexOnDefd(_LexLContext* restrict llc);

/**
 * @brief adds storage token
 * 
 * @param llc context
 */
void _lexOnStorage(_LexLContext* restrict llc);

/**
 * @brief adds storage token
 * 
 * @param llc context
 * @param nme token representation
 * @param type type of the token
 * @return _Bool true -> added, false -> not the storage
 */
_Bool _lexCheckStorage(_LexLContext* restrict llc, String nme, T_TokenType type);

/**
 * @brief adds parm specified token
 * 
 * @param llc context
 */
void _lexOnParm(_LexLContext* restrict llc);

/**
 * @brief adds strc specified token
 * 
 * @param llc context
 */
void _lexOnStrc(_LexLContext* llc);

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
    List spans = _lexTokenize(in, filename);

    _LexLContext context =
    {
        .err = listNew(ErrorSpan),
        .spans = liCreate(&spans),
        .tokens = listNew(Token),
        .nest = 0,
        .defd = -1,
        .parm = -1,
        .strc = -1,
    };

    _LexLContext* restrict llc = &context;

    while (liCan(&llc->spans))
    {
        // get the string to examine
        llc->span = liGet(&llc->spans, FileSpan);
        liMove(&llc->spans);

        // check for tokens that can be recognized by their first few characters
        switch (llc->span.str.c[0])
        {
        // this case should never happen
        case 0:
            dtPrintf("lex: empty token at position :%zu:%zu", llc->span.pos.line, llc->span.pos.col);
            fsFree(llc->span);
            continue;
        // [ is always token by itself
        case '[':
            _lexOnTOpen(llc);
            continue;
        // ] is always token by itself
        case ']':
            _lexOnTClose(llc);
            continue;
        // check for comments (starts with // or /*)
        case '/':
            if (_lexOnTComment(llc))
                continue;
            break;
        // string literal
        case '"':
            _lexOnTString(llc);
            continue;
        // char literal
        case '\'':
            _lexOnTChar(llc);
            continue;
        // nothing operator
        case '_':
            if (_lexOnT_(llc))
                continue;
            break;
        default:
            break;
        }

        // checking numbers
        if (_lexOnTNumber(llc))
            continue;

        // tokens that are not enclosed in [] are incorrect
        if (llc->nest == 0)
        {
            _lexError(llc, E_ERROR, "cannot use identifiers directly", "try ecapsulating it in []");
            continue;
        }

        if (_lexOnTKeyword(llc))
            continue;

        if (_lexOnTBool(llc))
            continue;

        if (llc->defd != -1)
        {
            _lexOnDefd(llc);
            continue;
        }

        if (llc->parm != -1)
        {
            _lexOnParm(llc);
            continue;
        }

        if (llc->strc != -1)
        {
            _lexOnStrc(llc);
            continue;
        }

        // determine token type based on previous tokens
        switch (listGet(llc->tokens, llc->tokens.length - 1, Token).type)
        {
        // tokens directly after [ are function identifiers
        case T_PUNCTUATION_BRACKET_OPEN:
            listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_FUNCTION, llc->span), Token);
            continue;
        // other tokens are just variable identifiers
        default:
            listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_VARIABLE, llc->span), Token);
            continue;
        }
    }

    if (llc->nest > 0)
        listAdd(llc->err, errCreateErrorSpan(E_ERROR, fsCreate(strLit("]"), liGet(&llc->spans, FileSpan).pos), strLit("missing 1 or more closing brackets"), strLit("try adding ]")), ErrorSpan);

    // free the list of strings
    listFree(spans);

    // if errors is not null set them, otherwise free them
    if (errors)
        *errors = llc->err;
    else
        listDeepFree(llc->err, ErrorSpan, t, errFreeErrorSpan(t));

    return llc->tokens;
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

intmax_t _lexReadInt(char* str, char** endptr, intmax_t base, _Bool* overflow, size_t* digits)
{
    assert(str);
    assert(base <= 36 && base >= 2);

    size_t digs = 0;
    intmax_t num = 0;
    intmax_t digit;
    _Bool ovfl = 0;
    for (; *str && (digit = _lexGetDigit(*str)) < base; str++, digs++)
    {
        if (num > INTMAX_MAX / base || (num == INTMAX_MAX / base && digit > INTMAX_MAX % base))
            ovfl = 1;
        num = num * base + digit;
    }
    if (endptr)
        *endptr = str;
    if (overflow)
        *overflow = ovfl;
    if (digits)
        *digits += digs;
    return num;
}

_Bool _lexCheckKeyword(_LexLContext* restrict llc, String kw, T_TokenType type, long* nest, long nestOff)
{
    if (!strEquals(kw, llc->span.str))
        return 0;

    listAdd(llc->tokens, tokenCreate(type, llc->span.pos), Token);
    if (!nest)
        return 1;

    if (llc->defd == -1 && llc->parm == -1 && llc->strc == -1)
        *nest = llc->nest + nestOff;
    
    return 1;
}

void _lexOnTOpen(_LexLContext* restrict llc)
{
    assert(llc->span.str.length == 1);

    if (llc->defd == llc->nest)
    {
        Token* t = listGetP(llc->tokens, llc->tokens.length - 1);
        if (t->type == T_IDENTIFIER_STRUCT)
            t->type = T_IDENTIFIER_FUNCTION;
    }

    listAdd(llc->tokens, tokenInt(T_PUNCTUATION_BRACKET_OPEN, llc->nest, llc->span.pos), Token);
    fsFree(llc->span);
    
    llc->nest++;
}

void _lexOnTClose(_LexLContext* restrict llc)
{
    assert(llc->span.str.length == 1);

    if (llc->nest == llc->defd || llc->nest == llc->parm || llc->nest == llc->strc)
    {
        llc->defd = -1;
        llc->parm = -1;
        llc->strc = -1;
    }

    if (llc->nest == 0)
    {
        _lexError(llc, E_ERROR, "missing [ before ]", "");
        return;
    }

    llc->nest--;

    listAdd(llc->tokens, tokenInt(T_PUNCTUATION_BRACKET_CLOSE, llc->nest, llc->span.pos), Token);
    fsFree(llc->span);
}

_Bool _lexOnTComment(_LexLContext* restrict llc)
{
    switch(llc->span.str.c[1])
    {
    // line comment
    case '/':
        //listAdd(tokens, fileSpanTokenPart(COMMENT_LINE, span, 2, span.length - 2), Token);
        fsFree(llc->span);
        return 1;
    // block comment
    case '*':
        // check if the block comment is closed
        if (llc->span.str.c[llc->span.str.length - 1] != '/' || llc->span.str.c[llc->span.str.length - 2] != '*')
        {
            _lexError(llc, E_ERROR, "block comment is not closed", "close comment with */");
            return 1;
        }
        //listAdd(tokens, fileSpanTokenPart(COMMENT_BLOCK, span, 2, span.length < 5 ? 0 : span.length - 4), Token);
        fsFree(llc->span);
        return 1;
    default:
        return 0;
    }
}

void _lexOnTString(_LexLContext* restrict llc)
{
    // check if the string literal is ended
    if (llc->span.str.length == 1 || llc->span.str.c[llc->span.str.length - 1] != '"')
    {
        _lexError(llc, E_ERROR, "string literal is not closed", "try close literal with \"");
        return;
    }

    listAdd(llc->tokens, tokenFileSpanPart(T_LITERAL_STRING, llc->span, 1, llc->span.str.length - 2), Token);
    fsFree(llc->span);
}

void _lexOnTChar(_LexLContext* restrict llc)
{
    // check if the char literal has only one character
    if (llc->span.str.length != 3)
    {
        _lexError(llc, E_ERROR, "char literal can only contain one character", "maybe you want to use string (\")");
        return;
    }

    // check if the char literal is closed
    else if (llc->span.str.c[2] != '\'')
    {
        _lexError(llc, E_ERROR, "char literal is not closed", "try adding closing '");
        return;
    }

    listAdd(llc->tokens, tokenChar(T_LITERAL_CHAR, llc->span.str.c[1], llc->span.pos), Token);
    fsFree(llc->span);
}

_Bool _lexOnT_(_LexLContext* restrict llc)
{
    // check if it is only the operator
    if (llc->span.str.length != 1)
        return 0;

    listAdd(llc->tokens, tokenCreate(T_OPERATOR_NOTHING, llc->span.pos), Token);
    fsFree(llc->span);
    return 1;
}

_Bool _lexOnTNumber(_LexLContext* restrict llc)
{
    if (!isdigit(llc->span.str.c[0]) && (llc->span.str.c[0] != '-' || !isdigit(llc->span.str.c[1])))
        return 0;

    char* c = llc->span.str.c;
    _Bool overflow = 0;
    // check for negative values
    _Bool isNegative = *c == '-';
    size_t digits = 0;
    // read whole part values
    intmax_t num = _lexReadInt(c + isNegative, &c, 10, &overflow, &digits);

    switch (*c)
    {
    // if it doesn't continue, it is integer
    case 0:
        _lexOnTInt(llc, isNegative ? -num : num, overflow);
        return 1;
    // if there is . read decimal values
    case '.':
        _lexOnTFloat(llc, num, isNegative, overflow, digits, c);
        return 1;
    // reading hexadecimal numbers
    case 'x':
        _lexOnTBase(llc, 16, c, isNegative);
        return 1;
    // reading binary numbers
    case 'b':
        _lexOnTBase(llc, 2, c, isNegative);
        return 1;
    // reding numbers with the specified base
    case 'z':
        _lexOnTBase(llc, num, c, isNegative);
        return 1;
    // otherwise break into error
    default:
        _lexError(llc, E_ERROR, "integer literal followed by characters", "remove trailing characters");
        return 1;
    }
}

void _lexOnTInt(_LexLContext* restrict llc, intmax_t num, _Bool overflow)
{
    listAdd(llc->tokens, tokenInt(T_LITERAL_INTEGER, num, llc->span.pos), Token);
    if (overflow)
    {
        _lexError(llc, E_WARNING, "number is too large", "make the number smaller or use defferent type");
        return;
    }
    fsFree(llc->span);
}

void _lexOnTFloat(_LexLContext* restrict llc, intmax_t num, _Bool isNegative, _Bool overflow, size_t digits, char* c)
{
    // read decimal values
    double decimal = num;
    // to preserve the magnitude of the number read it again as double
    if (overflow)
    {
        decimal = 0;
        for (c = llc->span.str.c + isNegative; *c && isdigit(*c); c++, digits++)
            decimal = decimal * 10 + *c - '0';
    }

    double div = 10;
    for (c++; *c && isdigit(*c); c++, div *= 10, digits++)
        decimal += (*c - '0') / div;

    if (*c)
    {
        _lexError(llc, E_ERROR, "float literal followed by characters", "remove the trailing characters");
        return;
    }

    listAdd(llc->tokens, tokenFloat(T_LITERAL_FLOAT, isNegative ? -decimal : decimal, llc->span.pos), Token);
    
    // check for too large or precise numbers
    if (isinf(decimal))
    {
        _lexError(llc, E_WARNING, "number is too large and will be interpreted as infinity", "");
        return;
    }
    if (digits > lex_DECIMAL_WARNING_LIMIT)
    {
        _lexError(llc, E_WARNING, "number has too many digits and may be rounded", "if you want all the digits maybe use string");
        return;
    }

    fsFree(llc->span);
}

void _lexOnTBase(_LexLContext* restrict llc, intmax_t base, char* c, _Bool isNegative)
{
    if (base < 2 || base > 36)
    {
        _lexError(llc, E_ERROR, "base must be between 2 and 36 (inclusive)", "change the number before z to be in that range");
        return;
    }

    _Bool overflow = 0;

    intmax_t num = _lexReadInt(c + 1, &c, base, &overflow, NULL);
    
    if (*c)
    {
        _lexError(llc, E_ERROR, "int literal followed by characters", "remove the trailing characters");
        return;
    }

    listAdd(llc->tokens, tokenInt(T_LITERAL_INTEGER, isNegative ? -num : num, llc->span.pos), Token);
    if (overflow)
    {
        _lexError(llc, E_WARNING, "number is too large", "make the number smaller or use defferent type");
        return;
    }
    fsFree(llc->span);
}

_Bool _lexOnTKeyword(_LexLContext* restrict llc)
{
    if (_lexCheckKeyword(llc, strLit("set"), T_KEYWORD_SET, NULL, 0))
        return 1;
    if (_lexCheckKeyword(llc, strLit("struct"), T_KEYWORD_STRUCT, &llc->strc, 0))
        return 1;
    if (_lexCheckKeyword(llc, strLit("sign"), T_KEYWORD_SIGN, &llc->defd, 0))
        return 1;
    if (_lexCheckKeyword(llc, strLit("def"), T_KEYWORD_DEF, &llc->defd, 1))
        return 1;
    return 0;
}

_Bool _lexOnTBool(_LexLContext* restrict llc)
{
    if (strEquals(llc->span.str, strLit("true")))
    {
        listAdd(llc->tokens, tokenBool(T_LITERAL_BOOL, 1, llc->span.pos), Token);
        return 1;
    }

    if (strEquals(llc->span.str, strLit("false")))
    {
        listAdd(llc->tokens, tokenBool(T_LITERAL_BOOL, 0, llc->span.pos), Token);
        return 1;
    }

    return 0;
}

void _lexOnDefd(_LexLContext* restrict llc)
{
    if (llc->defd == llc->nest)
    {
        Token* t = listGetP(llc->tokens, llc->tokens.length - 1);
        if (t->type == T_IDENTIFIER_STRUCT)
            t->type = T_IDENTIFIER_VARIABLE;
    }

    _lexOnStorage(llc);
}

void _lexOnStorage(_LexLContext* restrict llc)
{
    if (_lexCheckStorage(llc, strLit("*"), T_STORAGE_POINTER))
        return;
    if (_lexCheckStorage(llc, strLit("char"), T_STORAGE_CHAR))
        return;
    if (_lexCheckStorage(llc, strLit("string"), T_STORAGE_STRING))
        return;
    if (_lexCheckStorage(llc, strLit("int"), T_STORAGE_INT))
        return;
    if (_lexCheckStorage(llc, strLit("float"), T_STORAGE_FLOAT))
        return;
    if (_lexCheckStorage(llc, strLit("bool"), T_STORAGE_BOOL))
        return;

    listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, llc->span), Token);
}

_Bool _lexCheckStorage(_LexLContext* restrict llc, String nme, T_TokenType type)
{
    if (!strEquals(llc->span.str, nme))
        return 0;

    listAdd(llc->tokens, tokenCreate(type, llc->span.pos), Token);

    fsFree(llc->span);
    return 1;
}

void _lexOnParm(_LexLContext* restrict llc)
{
    if (llc->parm > llc->nest && listGet(llc->tokens, llc->tokens.length - 1, Token).type == T_PUNCTUATION_BRACKET_OPEN)
    {
        listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, llc->span), Token);
        return;
    }
    listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_PARAMETER, llc->span), Token);
}

void _lexOnStrc(_LexLContext* llc)
{
    if (llc->strc == llc->nest)
    {
        listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_STRUCT, llc->span), Token);
        return;
    }

    switch (listGet(llc->tokens, llc->tokens.length - 1, Token).type)
    {
    case T_PUNCTUATION_BRACKET_OPEN:
        _lexOnStorage(llc);
        return;
    default:
        listAdd(llc->tokens, tokenFileSpan(T_IDENTIFIER_PARAMETER, llc->span), Token);
        return;
    }
}

List _lexTokenize(Stream* in, String* filename)
{
    assert(in);
    assert(filename);

    _LexTContext context =
    {
        .err = listNew(ErrorSpan),
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
    listDeepFree(ltc->err, ErrorSpan, e, errFreeErrorSpan(e));
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