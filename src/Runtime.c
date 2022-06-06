#include "Runtime.h"

#include "List.h"
#include "DebugTools.h"

Runtime createRuntime(List *errors)
{
    Runtime r =
        {
            .variables = newList(Variable),
            .errors = liCreate(errors),
        };

    return r;
}

void freeRuntime(Runtime r)
{
    listDeepFree(r.variables, Variable, v, freeVariable(v));
}

void freeVariable(Variable v)
{
    strFree(v.name);
    switch (v.type)
    {
    case V_STRING:
        strFree(v.str);
        return;
    case V_FUNCTION:
        freeFunction(v.function);
        return;
    default:
        break;
    }
}

void freeFunction(Function f)
{
    listDeepFree(f.parameters, String, s, strFree(s));
}

Variable createBoolVariable(String name, bool value)
{
    Variable v =
    {
        .type = V_BOOL,
        .name = name,
        .boolean = value,
    };
    return v;
}

Variable createIntVariable(String name, long long value)
{
    Variable v =
    {
        .type = V_INT,
        .name = name,
        .integer = value,
    };
    return v;
}

Variable createFloatVariable(String name, double value)
{
    Variable v =
    {
        .type = V_FLOAT,
        .name = name,
        .decimal = value,
    };
    return v;
}

Variable createCharVariable(String name, char value)
{
    Variable v =
    {
        .type = V_CHAR,
        .name = name,
        .character = value,
    };
    return v;
}

Variable createStringVariable(String name, String value)
{
    Variable v =
    {
        .type = V_STRING,
        .name = name,
        .str = value,
    };
    return v;
}

Variable createFunctionVariable(String name, Function value)
{
    Variable v =
    {
        .type = V_FUNCTION,
        .name = name,
        .function = value,
    };
    return v;
}

Variable copyVariable(String name, Variable var)
{
    switch (var.type)
    {
    case V_BOOL:
        return createBoolVariable(name, var.boolean);
    case V_INT:
        return createIntVariable(name, var.integer);
    case V_FLOAT:
        return createFloatVariable(name, var.decimal);
    case V_CHAR:
        return createFloatVariable(name, var.character);
    case V_STRING:
        return createStringVariable(name, strCopy(var.str));
    /*case V_FUNCTION:
        return createFunctionVariable(name, var.function);*/
    default:
        except("copyVariable: invalid variable type");
        return createBoolVariable(strEmpty(), false);
    }
}

Function createFunction(Action action, List parameters)
{
    Function f =
    {
        .action = action,
        .parameters = parameters,
    };
    return f;
}

Variable invokeFunction(Function f, Runtime* r, List par)
{
    return f.action(f, r, par);
}

Variable createNothingVariable()
{
    Variable v =
    {
        .type = V_NOTHING,
        .name = strEmpty(),
    };
    return v;
}

bool runtimeGet(Runtime* r, String name, Variable* v)
{
    ListIterator li = liCreate(&r->variables);
    while (liCan(&li))
    {
        Variable var = liGet(&li, Variable);
        liMove(&li);
        if (strEquals(var.name, name))
        {
            *v = var;
            return true;
        }
    }
    return false;
}

Variable boolVariable(bool value)
{
    return createBoolVariable(strEmpty(), value);
}

Variable intVariable(long long value)
{
    return createIntVariable(strEmpty(), value);
}

Variable floatVariable(double value)
{
    return createFloatVariable(strEmpty(), value);
}

Variable charVariable(char value)
{
    return createCharVariable(strEmpty(), value);
}

Variable stringVariable(String value)
{
    return createStringVariable(strEmpty(), value);
}

Variable functionVariable(Function value)
{
    return createFunctionVariable(strEmpty(), value);
}