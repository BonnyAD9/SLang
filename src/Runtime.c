#include "Runtime.h"

#include <assert.h>
#include <stdio.h>

#include "List.h"
#include "DebugTools.h"
#include "Terminal.h"

Runtime rtCreate(List *errors)
{
    Runtime r =
        {
            .variables = listNew(Variable),
            .errors = liCreate(errors),
        };

    return r;
}

void rtFree(Runtime r)
{
    listDeepFree(r.variables, Variable, v, rtFreeVariable(v));
}

void rtFreeVariable(Variable v)
{
    strFree(v.name);
    switch (v.type)
    {
    case V_EXCEPTION:
    case V_STRING:
        strFree(v.str);
        return;
    case V_FUNCTION:
        rtFreeFunction(v.function);
        return;
    default:
        break;
    }
}

void rtFreeFunction(Function f)
{
    listDeepFree(f.parameters, String, s, strFree(s));
}

Variable rtException(String name, String message)
{
    Variable var =
    {
        .name = name,
        .type = V_EXCEPTION,
        .str = message,
    };
    return var;
}

void rtPrintExceptionE(FILE* out, Variable exception)
{
    assert(out);

    if (exception.type != V_EXCEPTION)
        return;
    fprintf(out, term_BRED"%s:"term_COLRESET" %s", exception.name.c, exception.str.c);
    exit(EXIT_FAILURE);
}

void rtPrintException(FILE* out, Variable exception)
{
    assert(out);

    if (exception.type != V_EXCEPTION)
        return;
    fprintf(out, term_BRED"%s:"term_COLRESET" %s", exception.name.c, exception.str.c);
}

Variable rtCreateBoolVariable(String name, _Bool value)
{
    Variable v =
    {
        .type = V_BOOL,
        .name = name,
        .boolean = value,
    };
    return v;
}

Variable rtCreateIntVariable(String name, long long value)
{
    Variable v =
    {
        .type = V_INT,
        .name = name,
        .integer = value,
    };
    return v;
}

Variable rtCreateFloatVariable(String name, double value)
{
    Variable v =
    {
        .type = V_FLOAT,
        .name = name,
        .decimal = value,
    };
    return v;
}

Variable rtCreateCharVariable(String name, char value)
{
    Variable v =
    {
        .type = V_CHAR,
        .name = name,
        .character = value,
    };
    return v;
}

Variable rtCreateStringVariable(String name, String value)
{
    Variable v =
    {
        .type = V_STRING,
        .name = name,
        .str = value,
    };
    return v;
}

Variable rtCreateFunctionVariable(String name, Function value)
{
    Variable v =
    {
        .type = V_FUNCTION,
        .name = name,
        .function = value,
    };
    return v;
}

Variable rtCopyVariable(String name, Variable var)
{
    switch (var.type)
    {
    case V_BOOL:
        return rtCreateBoolVariable(name, var.boolean);
    case V_INT:
        return rtCreateIntVariable(name, var.integer);
    case V_FLOAT:
        return rtCreateFloatVariable(name, var.decimal);
    case V_CHAR:
        return rtCreateFloatVariable(name, var.character);
    case V_STRING:
        return rtCreateStringVariable(name, strCopy(var.str));
    case V_EXCEPTION:
        return rtException(name, var.str);
    /*case V_FUNCTION:
        return createFunctionVariable(name, var.function);*/
    default:
        dtExcept("copyVariable: invalid variable type");
        return rtCreateBoolVariable(strEmpty(), 0);
    }
}

Function rtCreateFunction(Action action, List parameters)
{
    Function f =
    {
        .action = action,
        .parameters = parameters,
    };
    return f;
}

Variable rtInvokeFunction(Function f, Runtime* r, List par)
{
    return f.action(f, r, par);
}

Variable rtCreateNothingVariable()
{
    Variable v =
    {
        .type = V_NOTHING,
        .name = strEmpty(),
    };
    return v;
}

_Bool rtGet(Runtime* r, String name, Variable* v)
{
    ListIterator li = liCreate(&r->variables);
    while (liCan(&li))
    {
        Variable var = liGet(&li, Variable);
        liMove(&li);
        if (strEquals(var.name, name))
        {
            *v = var;
            return 1;
        }
    }
    return 0;
}

Variable rtBoolVariable(_Bool value)
{
    return rtCreateBoolVariable(strEmpty(), value);
}

Variable rtIntVariable(long long value)
{
    return rtCreateIntVariable(strEmpty(), value);
}

Variable rtFloatVariable(double value)
{
    return rtCreateFloatVariable(strEmpty(), value);
}

Variable rtCharVariable(char value)
{
    return rtCreateCharVariable(strEmpty(), value);
}

Variable rtStringVariable(String value)
{
    return rtCreateStringVariable(strEmpty(), value);
}

Variable rtFunctionVariable(Function value)
{
    return rtCreateFunctionVariable(strEmpty(), value);
}