#include "BuiltinFunctions.h"

#include <stdio.h>

#include "List.h"
#include "Runtime.h"
#include "Terminal.h"

void registerBuiltins(Runtime* r)
{
    listAdd(r->variables, createFunctionVariable(strLit("print"), createFunction(print, newList(String))), Variable);
    listAdd(r->variables, createFunctionVariable(strLit("println"), createFunction(println, newList(String))), Variable);
}

Variable println(Function f, Runtime* r, List par)
{
    Variable ret = print(f, r, par);
    printf("\n");
    return ret;
}

Variable print(Function f, Runtime* r, List par)
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
            printf("%"term_SIZE_T, v.integer);
            break;
        case V_FLOAT:
            printf("%lf", v.decimal);
            break;
        case V_CHAR:
            printf("%c", v.character);
            break;
        case V_STRING:
            printf("%s", v.str.data);
            break;
        case V_STRUCT:
            printf("<struct>");
            break;
        case V_FUNCTION:
            printf("[%s", v.name.data);
            listForEach(v.function.parameters, String, s, printf(" %s", s.data));
            printf("]");
            break;
        case V_NOTHING:
            printf("_");
            break;
        default:
            printf("<error>");
            break;
        }
        freeVariable(v);
    }
    freeList(par);
    return createNothingVariable();
}