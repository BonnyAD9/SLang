#include "Parser.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "ParserTree.h"
#include "List.h"
#include "Errors.h"
#include "DebugTools.h"

#define _parNextToken(__list, __i, __name, __ifnot) if(__i+1<__list.length)__name=*(Token*)listGetP(__list, ++__i);else{__ifnot;}
#define _parNextTokenP(__list, __i, __name, __ifnot) if(*__i+1<__list->length)__name=*(Token*)listGetP(*__list, ++*__i);else{__ifnot;}

void _parErrAddP(List* list, ErrorToken item);

/**
 * @brief creates function call node
 * 
 * @param function what will return the function
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode function call
 */
ParserNode _parFunctionCall(ParserNode function, List* tokens, size_t* i, List* errors);

/**
 * @brief evaluates given function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode value
 */
ParserNode _parEvaluate(List* tokens, size_t* i, List* errors);

/**
 * @brief defines function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode value
 */
ParserNode _parDef(List* tokens, size_t* i, List* errors);

/**
 * @brief defines structure
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode struct definition
 */
ParserNode _parStruct(List* tokens, size_t* i, List* errors);

/**
 * @brief sets a variable or function
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode variable
 */
ParserNode _parSet(List* tokens, size_t* i, List* errors);

/**
 * @brief defines function or variable signature
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode signature
 */
ParserNode _parSign(List* tokens, size_t* i, List* errors);

/**
 * @brief returns nothing :)
 * 
 * @param tokens tokens
 * @param i position in tokens
 * @param errors error output
 * @return ParserNode nothing
 */
ParserNode _parNothing(List* tokens, size_t* i, List* errors);

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
int _parValue(List* tokens, size_t* i, List* errors, ParserNode* out);

ParserTree parParse(List tokens, List* errors)
{
    ParserTree tree = ptCreate();
    List errs = listNew(ErrorToken);

    for (size_t i = 0; i < tokens.length; i++)
    {
        Token t = listGet(tokens, i, Token);
        if (t.type != T_PUNCTUATION_BRACKET_OPEN)
        {
            listAdd(errs, errCreateErrorToken(E_ERROR, t, "expected [", "try encapsulating this section with []"), ErrorToken);
            continue;
        }
        _parNextToken(tokens, i, t, continue);
        switch (t.type)
        {
        case T_PUNCTUATION_BRACKET_OPEN:
            ptAdd(&tree, _parEvaluate(&tokens, &i, &errs));
            continue;
        case T_PUNCTUATION_BRACKET_CLOSE:
            continue;
        case T_IDENTIFIER_FUNCTION:
            ptAdd(&tree, _parFunctionCall(ptTokenNode(P_IDENTIFIER, t), &tokens, &i, &errs));
            continue;
        case T_KEYWORD_DEF:
            ptAdd(&tree, _parDef(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_STRUCT:
            ptAdd(&tree, _parStruct(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_SET:
            ptAdd(&tree, _parSet(&tokens, &i, &errs));
            continue;
        case T_KEYWORD_SIGN:
            ptAdd(&tree, _parSign(&tokens, &i, &errs));
            continue;
        case T_OPERATOR_NOTHING:
            listAdd(errs, errCreateErrorToken(E_WARNING, t, "call with nothing", "did you forget to remove _?"), ErrorToken);
            _parNothing(&tokens, &i, &errs);
            continue;
        default:
            listAdd(errs, errCreateErrorToken(E_ERROR, t, "expected [, ], function identifier, def, struct, set, defined or _", "use anything of the things specified above"), ErrorToken);
            break;
        }
    }

    if (errors)
        *errors = errs;
    else
        listDeepFree(errs, ErrorToken, t, errFreeErrorToken(t));
    return tree;
}

ParserNode _parFunctionCall(ParserNode function, List* tokens, size_t* i, List* errors)
{
    ParserNode call = ptCreateNode(P_FUNCTION_CALL);
    ptNodeAdd(&call, function);
    while (*i < tokens->length)
    {
        ParserNode n;
        switch (_parValue(tokens, i, errors, &n))
        {
        case -1:
            continue;
        case 1:
            ptNodeAdd(&call, n);
            continue;
        default:
            return call;
        }
    }
    _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try adding ]"));
    return call;
}

ParserNode _parEvaluate(List *tokens, size_t *i, List* errors)
{
    Token t;
    _parNextTokenP(tokens, i, t,
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add function call"));
        return ptCreateNode(P_ERROR);
    );
    switch (t.type)
    {
    case T_PUNCTUATION_BRACKET_OPEN:
        return _parFunctionCall(_parEvaluate(tokens, i, errors), tokens, i, errors);
    case T_PUNCTUATION_BRACKET_CLOSE:
        return ptCreateNode(P_NOTHING);
    case T_IDENTIFIER_FUNCTION:
        return _parFunctionCall(ptTokenNode(P_IDENTIFIER, t), tokens, i, errors);
    case T_KEYWORD_DEF:
        return _parDef(tokens, i, errors);
    case T_KEYWORD_SET:
        return _parSet(tokens, i, errors);
    case T_OPERATOR_NOTHING:
        _parErrAddP(errors, errCreateErrorToken(E_WARNING, t, "call with nothing", "did you forget to remove _?"));
        return _parNothing(tokens, i, errors);
    default:
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected [, ], function identifier, def, set or _", "use one of the above"));
        _parNothing(tokens, i, errors);
        return ptCreateNode(P_ERROR);
    }
}

ParserNode _parDef(List* tokens, size_t* i, List* errors)
{
    Token t;
    _parNextTokenP(tokens, i, t,
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected function definition", "consider adding function parameters and its body"));
        return ptCreateNode(P_ERROR);
    )
    if (t.type != T_PUNCTUATION_BRACKET_OPEN)
    {
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected function parameters", "if you don't want any parameters use []"));
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            tokenFree(t);
            _parNextTokenP(tokens, i, t,
                _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "consider closing the function body"));
                return ptCreateNode(P_ERROR);
            )
        }
        return ptCreateNode(P_ERROR);
    }

    ParserNode node = ptCreateNode(P_FUNCTION_DEFINITION);

    for ((*i)++; *i < tokens->length; (*i)++)
    {
        t = listGet(*tokens, *i, Token);
        if (t.type == T_PUNCTUATION_BRACKET_CLOSE)
            break;
        
        switch (t.type)
        {
        case T_IDENTIFIER_PARAMETER:
            ptNodeAdd(&node, ptTokenNode(P_IDENTIFIER, t));
            continue;
        case T_OPERATOR_NOTHING:
            ptNodeAdd(&node, ptCreateNode(P_NOTHING));
            continue;
        default:
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected funciton parameter", "you cannot use keywords or [ as function parameters"));
            continue;
        };
    }

    ParserNode n;
    switch (_parValue(tokens, i, errors, &n))
    {
        case -2:
            return node;
        case 1:
            ptNodeAdd(&node, n);
            break;
        default:
            break;
    }

    _parNextTokenP(tokens, i, t,
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
        return node;
    )

    while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
    {
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
        _parNextTokenP(tokens, i, t,
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return node;
        )
    }
    return node;
}

ParserNode _parStruct(List *tokens, size_t *i, List *errors)
{
    dtExcept("_struct: structs are not implemented yet");
    return ptCreateNode(P_ERROR);
}

ParserNode _parSet(List* tokens, size_t* i, List* errors)
{
    Token t;
    _parNextTokenP(tokens, i, t,
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add a variable name"));
        return ptCreateNode(P_ERROR);
    )

    if (t.type != T_IDENTIFIER_VARIABLE)
    {
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected variable identifier", "don't use keyword or [ or ]"));
        if (t.type == T_PUNCTUATION_BRACKET_CLOSE)
        {
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected value before ]", "try adding here a value"));
            return ptCreateNode(P_ERROR);
        }
    }

    ParserNode set;
    ParserNode n;
    switch (_parValue(tokens, i, errors, &n))
    {
    case 0:
        _parErrAddP(errors, errCreateErrorToken(E_WARNING, t, "variable is nothing", "if this is intentional set it to _"));
        set = ptTokenNode(P_VARIABLE_SETTER, t);
        ptNodeAdd(&set, ptCreateNode(P_NOTHING));
        _parNextTokenP(tokens, i, t,
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return ptCreateNode(P_ERROR);
        )
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
            _parNextTokenP(tokens, i, t,
                _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
                return ptCreateNode(P_ERROR);
            )
        }
        return set;
    case 1:
        set = ptTokenNode(n.type == P_FUNCTION_DEFINITION ? P_FUNCTION_SETTER : P_VARIABLE_SETTER, t);
        ptNodeAdd(&set, n);
        _parNextTokenP(tokens, i, t,
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return ptCreateNode(P_ERROR);
        )
        while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
        {
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
            _parNextTokenP(tokens, i, t,
                _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
                return ptCreateNode(P_ERROR);
            )
        }
        return set;
    default:
        break;
    }
    
    while (t.type != T_PUNCTUATION_BRACKET_CLOSE)
    {
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected ]", "function body can only contain one statement"));
        _parNextTokenP(tokens, i, t,
            _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try closing the function body"));
            return ptCreateNode(P_ERROR);
        )
    }
    return ptCreateNode(P_ERROR);
}

ParserNode _parSign(List* tokens, size_t* i, List* errors)
{
    dtExcept("_sign: function and variable signatures are not supported yet");
    return ptCreateNode(P_ERROR);
}

ParserNode _parNothing(List* tokens, size_t* i, List* errors)
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
                return ptCreateNode(P_NOTHING);
            nest--;
            continue;
        default:
            continue;
        }
    }
    _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "expected ]", "try adding ]"));
    return ptCreateNode(P_NOTHING);
}

int _parValue(List* tokens, size_t* i, List* errors, ParserNode* out)
{
    Token t;
    _parNextTokenP(tokens, i, t,
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, tokenCreate(T_ERROR, SIZE_MAX, SIZE_MAX), "unexpected end", "add here a value"));
        return -2;
    )
    switch (t.type)
    {
    case T_PUNCTUATION_BRACKET_OPEN:
        *out = _parEvaluate(tokens, i, errors);
        return 1;
    case T_PUNCTUATION_BRACKET_CLOSE:
        return 0;
    case T_IDENTIFIER_VARIABLE:
        *out = ptTokenNode(P_IDENTIFIER, t);
        return 1;
    case T_LITERAL_INTEGER:
        *out = ptTokenNode(P_VALUE_INTEGER, t);
        return 1;
    case T_LITERAL_FLOAT:
        *out = ptTokenNode(P_VALUE_FLOAT, t);
        return 1;
    case T_LITERAL_CHAR:
        *out = ptTokenNode(P_VALUE_CHAR, t);
        return 1;
    case T_LITERAL_STRING:
        *out = ptTokenNode(P_VALUE_STRING, t);
        return 1;
    case T_LITERAL_BOOL:
        *out = ptTokenNode(P_VALUE_BOOL, t);
        return 1;
    case T_OPERATOR_NOTHING:
        *out = ptCreateNode(P_NOTHING);
        return 1;
    default:
        _parErrAddP(errors, errCreateErrorToken(E_ERROR, t, "expected [, ], variable identifier or literal", "use one of the things above"));
        return -1;
    }
}

void _parErrAddP(List* list, ErrorToken item)
{
    listAddP(list, &item);
}
