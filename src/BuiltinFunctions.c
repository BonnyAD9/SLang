#include "BuiltinFunctions.h"

#include <stdio.h>
#include <assert.h>

#include "List.h"
#include "Runtime.h"
#include "Terminal.h"

void bifRegisterBuiltins(Runtime* r)
{
    listAdd(r->variables, rtCreateFunctionVariable(strLit("print"), rtCreateFunction(bifPrint, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("println"), rtCreateFunction(bifPrintln, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("+"), rtCreateFunction(bifAdd, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("*"), rtCreateFunction(bifMultiply, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("-"), rtCreateFunction(bifSubtract, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("/"), rtCreateFunction(bifDivide, listNew(String))), Variable);
    listAdd(r->variables, rtCreateFunctionVariable(strLit("%"), rtCreateFunction(bifMod, listNew(String))), Variable);
}

Variable bifPrintln(Function f, Runtime* r, List par)
{
    assert(r);
    Variable ret = bifPrint(f, r, par);
    printf("\n");
    return ret;
}

Variable bifPrint(Function f, Runtime* r, List par)
{
    ListIterator iterator = liCreate(&par);
    ListIterator* li = &iterator;

    while (liCan(li))
    {
        Variable v = liGet(li, Variable);
        liMove(li);

        switch (v.type)
        {
        case V_BOOL:
            printf(v.boolean ? "true" : "false");
            break;
        case V_INT:
            printf("%zu", v.integer);
            break;
        case V_FLOAT:
            printf("%lf", v.decimal);
            break;
        case V_CHAR:
            printf("%c", v.character);
            break;
        case V_STRING:
            printf("%s", v.str.c);
            break;
        case V_STRUCT:
            printf("<struct>");
            break;
        case V_FUNCTION:
            printf("[%s", v.name.c);
            listForEach(v.function.parameters, String, s, printf(" %s", s.c));
            printf("]");
            break;
        case V_NOTHING:
            printf("_");
            break;
        case V_EXCEPTION:
            printf("<error ");
            rtPrintException(stdout, v);
            printf(">");
            break;
        default:
            printf("<other>");
            break;
        }
        rtFreeVariable(v);
    }
    listFree(par);
    return rtCreateNothingVariable();
}

Variable bifAdd(Function f, Runtime* r, List par)
{
    ListIterator li = liCreate(&par);

    Variable res = rtBoolVariable(0);

    while (liCan(&li))
    {
        Variable v = liGet(&li, Variable);
        liMove(&li);

        switch (v.type)
        {
        case V_BOOL:
            switch (res.type)
            {
            case V_BOOL:
                res.boolean += v.boolean;
                break;
            case V_INT:
                res.integer += v.boolean;
                break;
            default:
                res.decimal += v.boolean;
                break;
            }
            break;
        case V_INT:
            switch (res.type)
            {
            case V_BOOL:
                res.type = V_INT;
                res.integer = res.boolean + v.integer;
                break;
            case V_INT:
                res.integer += v.integer;
                break;
            default:
                res.decimal += v.integer;
                break;
            }
            break;
        case V_FLOAT:
            switch (res.type)
            {
            case V_BOOL:
                res.type = V_FLOAT;
                res.decimal = res.boolean + v.decimal;
                break;
            case V_INT:
                res.type = V_FLOAT;
                res.decimal = res.integer + v.decimal;
                break;
            default:
                res.decimal += v.decimal;
                break;
            }
            break;
        default:
            break;
        }
        rtFreeVariable(v);
    }
    return res;
}

Variable bifMultiply(Function f, Runtime* r, List par)
{
    ListIterator li = liCreate(&par);

    Variable res = rtBoolVariable(1);

    while (liCan(&li))
    {
        Variable v = liGet(&li, Variable);
        liMove(&li);

        switch (v.type)
        {
        case V_BOOL:
            switch (res.type)
            {
            case V_BOOL:
                res.boolean *= v.boolean;
                break;
            case V_INT:
                res.integer *= v.boolean;
                break;
            default:
                res.decimal *= v.boolean;
                break;
            }
            break;
        case V_INT:
            switch (res.type)
            {
            case V_BOOL:
                res.type = V_INT;
                res.integer = res.boolean * v.integer;
                break;
            case V_INT:
                res.integer *= v.integer;
                break;
            default:
                res.decimal *= v.integer;
                break;
            }
            break;
        case V_FLOAT:
            switch (res.type)
            {
            case V_BOOL:
                res.type = V_FLOAT;
                res.decimal = res.boolean * v.decimal;
                break;
            case V_INT:
                res.type = V_FLOAT;
                res.decimal = res.integer * v.decimal;
                break;
            default:
                res.decimal *= v.decimal;
                break;
            }
            break;
        default:
            break;
        }
        rtFreeVariable(v);
    }
    return res;
}

Variable bifSubtract(Function f, Runtime* r, List par)
{
    if (par.length == 1)
    {
        Variable v;
        Variable v0 = listGet(par, 0, Variable);
        switch (v0.type)
        {
        case V_BOOL:
            v = rtBoolVariable(!v0.boolean);
            break;
        case V_INT:
            v = rtIntVariable(-v0.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(-v0.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for subtraction"));
            break;
        }
        listDeepFree(par, Variable, v, rtFreeVariable(v));
        return v;
    }
    if (par.length != 2)
    {
        listDeepFree(par, Variable, v, rtFreeVariable(v));
        return rtException(strLit("InvalidArgumentCount"), strLit("Function - can only have two arguments"));
    }

    Variable v;
    Variable v0 = listGet(par, 0, Variable);
    Variable v1 = listGet(par, 1, Variable);
    switch (v0.type)
    {
    case V_BOOL:
        switch (v1.type)
        {
        case V_BOOL:
            v = rtBoolVariable(v0.boolean - v1.boolean);
            break;
        case V_INT:
            v = rtIntVariable(v0.boolean - v1.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(v1.boolean - v1.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for subtraction"));
            break;
        }
        break;
    case V_INT:
        switch (v1.type)
        {
        case V_BOOL:
            v = rtIntVariable(v0.integer - v1.boolean);
            break;
        case V_INT:
            v = rtIntVariable(v0.integer - v1.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(v0.integer - v1.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for subtraction"));
            break;
        }
        break;
    case V_FLOAT:
        switch (v1.type)
        {
        case V_BOOL:
            v = rtFloatVariable(v0.decimal - v1.boolean);
            break;
        case V_INT:
            v = rtFloatVariable(v0.decimal - v1.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(v0.decimal - v1.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for subtraction"));
            break;
        }
        break;
    default:
        v = rtException(strLit("InvalidType"), strLit("Invalid type of first argument for subtraction"));
        break;
    }
    listDeepFree(par, Variable, v, rtFreeVariable(v));
    return v;
}

Variable bifDivide(Function f, Runtime* r, List par)
{
    if (par.length != 2)
    {
        listDeepFree(par, Variable, v, rtFreeVariable(v));
        return rtException(strLit("InvalidArgumentCount"), strLit("Function - can only have two arguments"));
    }

    Variable v;
    Variable v0 = listGet(par, 0, Variable);
    Variable v1 = listGet(par, 1, Variable);
    switch (v0.type)
    {
    case V_INT:
        switch (v1.type)
        {
        case V_INT:
            v = rtIntVariable(v0.integer / v1.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(v0.integer / v1.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for subtraction"));
            break;
        }
        break;
    case V_FLOAT:
        switch (v1.type)
        {
        case V_INT:
            v = rtFloatVariable(v0.decimal / v1.integer);
            break;
        case V_FLOAT:
            v = rtFloatVariable(v0.decimal / v1.decimal);
            break;
        default:
            v = rtException(strLit("InvalidType"), strLit("Invalid type of second argument for division"));
            break;
        }
        break;
    default:
        v = rtException(strLit("InvalidType"), strLit("Invalid type of first argument for division"));
        break;
    }
    listDeepFree(par, Variable, v, rtFreeVariable(v));
    return v;
}

Variable bifMod(Function f, Runtime* r, List par)
{
    if (par.length != 2)
    {
        listDeepFree(par, Variable, v, rtFreeVariable(v));
        return rtException(strLit("InvalidArgumentCount"), strLit("Function - can only have two arguments"));
    }

    Variable v0 = listGet(par, 0, Variable);
    Variable v1 = listGet(par, 1, Variable);
    if (v0.type != V_INT || v1.type != V_INT)
    {
        listDeepFree(par, Variable, v, rtFreeVariable(v));
        return rtException(strLit("InvalidType"), strLit("Both arguments to modulo must be int"));
    }

    Variable v = rtIntVariable(v0.integer % v1.integer);
    listDeepFree(par, Variable, v, rtFreeVariable(v));
    return v;
}