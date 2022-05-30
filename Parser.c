#include "Parser.h"

#include <stdlib.h>

#include "ParserTree.h"
#include "List.h"
#include "Errors.h"
#include "Assert.h"

#define _nextToken(__list, __i, __name, __ifnot) if(__i+1<__list.length)__name=*(Token*)listGetP(__list, ++__i);else{__ifnot;}
#define _nextTokenP(__list, __i, __name, __ifnot) if(*__i+1<__list->length)__name=*(Token*)listGetP(*__list, ++*__i);else{__ifnot;}

void _errAddP(List* list, ErrorToken item);

/**
 * @brief creates function call node
 * 
 * @param function what will return the function
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode function call
 */
ParserNode _functionCall(ParserNode function, List* tokens, size_t* i, List* errors);

/**
 * @brief evaluates given function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode value
 */
ParserNode _evaluate(List* tokens, size_t* i, List* errors);

/**
 * @brief defines function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode value
 */
ParserNode _def(List* tokens, size_t* i, List* errors);

/**
 * @brief defines structure
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode struct definition
 */
ParserNode _struct(List* tokens, size_t* i, List* errors);

/**
 * @brief sets a variable or function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode variable
 */
ParserNode _set(List* tokens, size_t* i, List* errors);

/**
 * @brief defines function or variable signature
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode signature
 */
ParserNode _sign(List* tokens, size_t* i, List* errors);

/**
 * @brief returns nothing :)
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode nothing
 */
ParserNode _nothing(List* tokens, size_t* i, List* errors);

/**
 * @brief gets value
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @param out token output
 * @return true value readed
 * @return false closing bracket encountered
 */
int _value(List* tokens, size_t* i, List* errors, ParserNode* out);

ParserTree parse(List tokens, List* errors)
{
    ParserTree tree = createParserTree();
    List errs = newList(ErrorToken);

    for (size_t i = 0; i < tokens.length; i++)
    {
        Token t = listGet(tokens, i, Token);
        if (t.type != T_PUNCTUATION_BRACKET_OPEN)
        {
            listAdd(errs, createErrorToken(E_ERROR, t, "expected [", "try encapsulating this section with []"), ErrorToken);
            continue;
        }
        _nextToken(tokens, i, t, continue);
        switch (t.type)
        {
        case T_PUNCTUATION_BRACKET_OPEN:
            parserTreeAdd(&tree, _evaluate(&tokens, &i, &errs));
            continue;
        case T_PUNCTUATION_BRACKET_CLOSE:
            continue;
        case T_IDENTIFIER_FUNCTION:
            parserTreeAdd(&tree, _functionCall(tokenParserNode(P_IDENTIFIER, t), &tokens, &i, &errs));
            continue;
        case T_KEYWORD_DEF:
            parserTreeAdd(&tree, _def(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_STRUCT:
            parserTreeAdd(&tree, _struct(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_SET:
            parserTreeAdd(&tree, _set(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_SIGN:
            parserTreeAdd(&tree, _sign(&tokens, &i, &errs));
            continue;
        case T_OPERATOR_NOTHING:
            listAdd(errs, createErrorToken(E_WARNING, t, "call with nothing", "did you forget to remove _?"), ErrorToken);
            _nothing(&tokens, &i, &errs);
            continue;
        default:
            listAdd(errs, createErrorToken(E_ERROR, t, "expected [, ], function identifier, def, struct, set, defined or _", "use anything of the things specified above"), ErrorToken);
            break;
        }
    }

    if (errors)
        *errors = errs;
    else
        listDeepFree(errs, ErrorToken, t, freeErrorToken(t));
    return tree;
}

ParserNode _functionCall(ParserNode function, List* tokens, size_t* i, List* errors)
{
    ParserNode call = createParserNode(P_FUNCTION_CALL);
    parserNodeAdd(&call, function);
    while (*i < tokens->length)
    {
        ParserNode n;
        switch (_value(tokens, i, errors, &n))
        {
        case -1:
            continue;
        case 1:
            parserNodeAdd(&call, n);
            continue;
        default:
            return call;
        }
    }
    _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try adding ]"));
    return call;
}

ParserNode _evaluate(List *tokens, size_t *i, List* errors)
{
    Token t;
    _nextTokenP(tokens, i, t,
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add function call"));
        return createParserNode(P_ERROR);
    );
    switch (t.type)
    {
    case T_PUNCTUATION_BRACKET_OPEN:
        return _functionCall(_evaluate(tokens, i, errors), tokens, i, errors);
    case T_PUNCTUATION_BRACKET_CLOSE:
        return createParserNode(P_NOTHING);
    case T_IDENTIFIER_FUNCTION:
        return _functionCall(tokenParserNode(P_IDENTIFIER, t), tokens, i, errors);
    case T_KEYWORD_DEF:
        return _def(tokens, i, errors);
    case T_KEYWORD_SET:
        return _set(tokens, i, errors);
    case T_OPERATOR_NOTHING:
        _errAddP(errors, createErrorToken(E_WARNING, t, "call with nothing", "did you forget to remove _?"));
        return _nothing(tokens, i, errors);
    default:
        _errAddP(errors, createErrorToken(E_ERROR, t, "expected [, ], function identifier, def, set or _", "use one of the above"));
        _nothing(tokens, i, errors);
        return createParserNode(P_ERROR);
    }
}

ParserNode _def(List* tokens, size_t* i, List* errors)
{
    Token t;
    _nextTokenP(tokens, i, t,
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected function definition", "consider adding function parameters and its body"));
        return createParserNode(P_ERROR);
    )
    if (t.type != T_PUNCTUATION_BRACKET_OPEN)
    {
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected function parameters", "if you don't want any parameters use []"));
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            freeToken(t);
            _nextTokenP(tokens, i, t,
                _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "consider closing the function body"));
                return createParserNode(P_ERROR);
            )
        }
        return createParserNode(P_ERROR);
    }

    ParserNode node = createParserNode(P_FUNCTION_DEFINITION);

    for ((*i)++; *i < tokens->length; (*i)++)
    {
        t = listGet(*tokens, *i, Token);
        if (t.type == T_PUNCTUATION_BRACKET_CLOSE)
            break;
        
        switch (t.type)
        {
        case T_IDENTIFIER_PARAMETER:
            parserNodeAdd(&node, tokenParserNode(P_IDENTIFIER, t));
            continue;
        case T_OPERATOR_NOTHING:
            parserNodeAdd(&node, createParserNode(P_NOTHING));
            continue;
        default:
            _errAddP(errors, createErrorToken(E_ERROR, t, "expected funciton parameter", "you cannot use keywords or [ as function parameters"));
            continue;
        };
    }

    ParserNode n;
    switch (_value(tokens, i, errors, &n))
    {
        case -2:
            return node;
        case 1:
            parserNodeAdd(&node, n);
            break;
        default:
            break;
    }

    _nextTokenP(tokens, i, t,
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
        return node;
    )

    while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
    {
        _errAddP(errors, createErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
        _nextTokenP(tokens, i, t,
            _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return node;
        )
    }
    return node;
}

ParserNode _struct(List *tokens, size_t *i, List *errors)
{
    except("_struct: structs are not implemented yet");
    return createParserNode(P_ERROR);
}

ParserNode _set(List* tokens, size_t* i, List* errors)
{
    Token t;
    _nextTokenP(tokens, i, t,
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add a variable name"));
        return createParserNode(P_ERROR);
    )

    if (t.type != T_IDENTIFIER_VARIABLE)
    {
        _errAddP(errors, createErrorToken(E_ERROR, t, "expected variable identifier", "don't use keyword or [ or ]"));
        if (t.type == T_PUNCTUATION_BRACKET_CLOSE)
        {
            _errAddP(errors, createErrorToken(E_ERROR, t, "expected value before ]", "try adding here a value"));
            return createParserNode(P_ERROR);
        }
    }

    ParserNode set;
    ParserNode n;
    switch (_value(tokens, i, errors, &n))
    {
    case 0:
        _errAddP(errors, createErrorToken(E_WARNING, t, "variable is nothing", "if this is intentional set it to _"));
        set = tokenParserNode(P_VARIABLE_SETTER, t);
        parserNodeAdd(&set, createParserNode(P_NOTHING));
        _nextTokenP(tokens, i, t,
            _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return createParserNode(P_ERROR);
        )
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            _errAddP(errors, createErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
            _nextTokenP(tokens, i, t,
                _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
                return createParserNode(P_ERROR);
            )
        }
        return set;
    case 1:
        set = tokenParserNode(n.type == P_FUNCTION_DEFINITION ? P_FUNCTION_SETTER : P_VARIABLE_SETTER, t);
        parserNodeAdd(&set, n);
        _nextTokenP(tokens, i, t,
            _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return createParserNode(P_ERROR);
        )
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            _errAddP(errors, createErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
            _nextTokenP(tokens, i, t,
                _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
                return createParserNode(P_ERROR);
            )
        }
        return set;
    default:
        break;
    }
    
    while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
    {
        _errAddP(errors, createErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
        _nextTokenP(tokens, i, t,
            _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return createParserNode(P_ERROR);
        )
    }
    return createParserNode(P_ERROR);
}

ParserNode _sign(List* tokens, size_t* i, List* errors)
{
    except("_sign: function and variable signatures are not supported yet");
    return createParserNode(P_ERROR);
}

ParserNode _nothing(List* tokens, size_t* i, List* errors)
{
    size_t nest = 0;
    for ((*i)++; *i < tokens->length; (*i)++)
    {
        Token t = listGet(*tokens, *i, Token);
        switch (t.type)
        {
        case T_PUNCTUATION_BRACKET_OPEN:
            nest++;
            continue;
        case T_PUNCTUATION_BRACKET_CLOSE:
            if (nest == 0)
                return createParserNode(P_NOTHING);
            nest--;
            continue;
        default:
            continue;
        }
    }
    _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try adding ]"));
    return createParserNode(P_NOTHING);
}

int _value(List* tokens, size_t* i, List* errors, ParserNode* out)
{
    Token t;
    _nextTokenP(tokens, i, t,
        _errAddP(errors, createErrorToken(E_ERROR, createToken(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add here a value"));
        return -2;
    )
    switch (t.type)
    {
    case T_PUNCTUATION_BRACKET_OPEN:
        *out = _evaluate(tokens, i, errors);
        return 1;
    case T_PUNCTUATION_BRACKET_CLOSE:
        return 0;
    case T_IDENTIFIER_VARIABLE:
        *out = tokenParserNode(P_IDENTIFIER, t);
        return 1;
    case T_LITERAL_INTEGER:
        *out = tokenParserNode(P_VALUE_INTEGER, t);
        return 1;
    case T_LITERAL_FLOAT:
        *out = tokenParserNode(P_VALUE_FLOAT, t);
        return 1;
    case T_LITERAL_CHAR:
        *out = tokenParserNode(P_VALUE_CHAR, t);
        return 1;
    case T_LITERAL_STRING:
        *out = tokenParserNode(P_VALUE_STRING, t);
        return 1;
    case T_LITERAL_BOOL:
        *out = tokenParserNode(P_VALUE_BOOL, t);
        return 1;
    case T_OPERATOR_NOTHING:
        *out = createParserNode(P_NOTHING);
        return 1;
    default:
        _errAddP(errors, createErrorToken(E_ERROR, t, "expected [, ], variable identifier or literal", "use one of the things above"));
        return -1;
    }
}

void _errAddP(List* list, ErrorToken item)
{
    listAddP(list, &item);
}