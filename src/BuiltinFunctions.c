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