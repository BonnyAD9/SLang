#include "Evaluator.h"

#include <assert.h>

#include "ParserTree.h"
#include "List.h"
#include "Runtime.h"
#include "DebugTools.h"
#include "BuiltinFunctions.h"

/**
 * @brief executes function call
 * 
 * @param n node with the function call
 * @param r runtime context
 * @return Variable call result
 */
Variable _evCall(ParserNode n, Runtime* r);

/**
 * @brief sets variable
 * 
 * @param n node with the variable
 * @param r runtime context
 * @return Variable the variable that was set
 */
Variable _evSet(ParserNode n, Runtime* r);

/**
 * @brief evaluates statement
 * 
 * @param n node with the statement to evaluate
 * @param r runtime context
 * @return Variable value
 */
Variable _evEval(ParserNode n, Runtime* r);

/**
 * @brief creates new function
 * 
 * @param n node with the function definition
 * @param r runtime context
 * @return Variable new function
 */
Variable _evDef(ParserNode n, Runtime* r);

List evEvaluate(ParserTree tree)
{
    ListIterator li = liCreate(&tree.nodes);
    List errors = listNew(FileSpan);
    Runtime r = rtCreate(&errors);
    bifRegisterBuiltins(&r);

    while (liCan(&li))
    {
        ParserNode n = liGet(&li, ParserNode);
        liMove(&li);

        Variable v;

        switch (n.type)
        {
        case P_NOTHING:
            continue;
        case P_FUNCTION_CALL:
            v = _evCall(n, &r);
            break;
        case P_FUNCTION_SETTER:
        case P_VARIABLE_SETTER:
            v = _evSet(n, &r);
            break;
        default:
            dtExcept("evaluate: unsupported operation");
        }
        rtPrintExceptionE(stdout, v);
        rtFreeVariable(v);
    }

    rtFree(r);
    return errors;
}

Variable _evCall(ParserNode node, Runtime* r)
{
    assert(node.nodes.length > 0);

    ListIterator li = liCreate(&node.nodes);
    ParserNode n = liGet(&li, ParserNode);
    Variable v = _evEval(n, r);
    
    switch (v.type)
    {
    case V_NOTHING:
        return rtCreateNothingVariable();
    case V_FUNCTION:
        break;
    case V_EXCEPTION:
        return v;
    default:
        rtFreeVariable(v);
        return rtException(strLit("InvalidFunction"), strLit("This is not function"));
    }

    List par = listNew(Variable);

    while (liMove(&li))
        listAdd(par, _evEval(liGet(&li, ParserNode), r), Variable);

    return rtInvokeFunction(v.function, r, par);
}

Variable _evSet(ParserNode n, Runtime* r)
{
    return rtException(strLit("NotSupported"), strLit("set is not yet supported"));
}

Variable _evEval(ParserNode n, Runtime* r)
{
    switch (n.type)
    {
    case P_VALUE_INTEGER:
        return rtIntVariable(n.token->integer);
    case P_VALUE_FLOAT:
        return rtFloatVariable(n.token->decimal);
    case P_VALUE_CHAR:
        return rtCharVariable(n.token->character);
    case P_VALUE_STRING:
        return rtStringVariable(strC(n.token->string));
    case P_VALUE_BOOL:
        return rtBoolVariable(n.token->string);
    case P_IDENTIFIER:
    {
        Variable v;
        String s = strC(n.token->string);
        if (!rtGet(r, s, &v))
            return rtException(strLit("InvalidName"), strLit("The function doesn't exist"));
        strFree(s);
        return v;
    }
    case P_FUNCTION_CALL:
        return _evCall(n, r);
    case P_VARIABLE_SETTER:
    case P_FUNCTION_SETTER:
        return _evSet(n, r);
    case P_NOTHING:
        return rtCreateNothingVariable();
    case P_FUNCTION_DEFINITION:
        return _evDef(n, r);
    default:
        return rtException(strLit("InvalidOperation"), strLit("Cannot evaluate"));
    }
}

Variable _evDef(ParserNode n, Runtime* r)
{
    return rtException(strLit("NotSupported"), strLit("def is not supported"));
}