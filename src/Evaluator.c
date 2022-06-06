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
Variable _call(ParserNode n, Runtime* r);

/**
 * @brief sets variable
 * 
 * @param n node with the variable
 * @param r runtime context
 * @return Variable the variable that was set
 */
Variable _e_set(ParserNode n, Runtime* r);

/**
 * @brief evaluates statement
 * 
 * @param n node with the statement to evaluate
 * @param r runtime context
 * @return Variable value
 */
Variable _e_eval(ParserNode n, Runtime* r);

/**
 * @brief creates new function
 * 
 * @param n node with the function definition
 * @param r runtime context
 * @return Variable new function
 */
Variable _e_def(ParserNode n, Runtime* r);

List evaluate(ParserTree tree)
{
    ListIterator li = liCreate(&tree.nodes);
    List errors = newList(FileSpan);
    Runtime r = createRuntime(&errors);
    registerBuiltins(&r);

    while (liCan(&li))
    {
        ParserNode n = liGet(&li, ParserNode);
        liMove(&li);

        switch (n.type)
        {
        case P_NOTHING:
            continue;
        case P_FUNCTION_CALL:
            freeVariable(_call(n, &r));
            continue;
        case P_FUNCTION_SETTER:
        case P_VARIABLE_SETTER:
            _e_set(n, &r);
            continue;
        default:
            except("evaluate: unsupported operation");
            break;
        }
    }

    freeRuntime(r);
    return errors;
}

Variable _call(ParserNode node, Runtime* r)
{
    assert(node.nodes.length > 0);

    ListIterator li = liCreate(&node.nodes);
    ParserNode n = liGet(&li, ParserNode);
    Variable v = _e_eval(n, r);
    
    switch (v.type)
    {
    case V_NOTHING:
        return createNothingVariable();
    case V_FUNCTION:
        break;
    default:
        except("_call: invalid function");
        return createNothingVariable();
    }

    List par = newList(Variable);

    while (liMove(&li))
        listAdd(par, _e_eval(liGet(&li, ParserNode), r), Variable);

    return invokeFunction(v.function, r, par);
}

Variable _e_set(ParserNode n, Runtime* r)
{
    except("_set: not supported");
    return createNothingVariable();
}

Variable _e_eval(ParserNode n, Runtime* r)
{
    switch (n.type)
    {
    case P_VALUE_INTEGER:
        return intVariable(n.token->integer);
    case P_VALUE_FLOAT:
        return floatVariable(n.token->decimal);
    case P_VALUE_CHAR:
        return charVariable(n.token->character);
    case P_VALUE_STRING:
        return stringVariable(strC(n.token->string));
    case P_VALUE_BOOL:
        return boolVariable(n.token->string);
    case P_IDENTIFIER:
    {
        Variable v;
        String s = strC(n.token->string);
        if (!runtimeGet(r, s, &v))
            except("_e_eval: unknown identifier %s", s.data);
        strFree(s);
        return v;
    }
    case P_FUNCTION_CALL:
        return _call(n, r);
    case P_VARIABLE_SETTER:
    case P_FUNCTION_SETTER:
        return _e_set(n, r);
    case P_NOTHING:
        return createNothingVariable();
    case P_FUNCTION_DEFINITION:
        return _e_def(n, r);
    default:
        except("_eval: invalid node type");
        return createNothingVariable();
    }
}

Variable _e_def(ParserNode n, Runtime* r)
{
    except("_e_def: not supported");
    return createNothingVariable();
}