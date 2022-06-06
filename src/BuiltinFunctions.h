#ifndef bif_BUILTIN_FUNCTIONS_INCLUDED
#define bif_BUILTIN_FUNCTIONS_INCLUDED

#include "Runtime.h"
#include "List.h"

void bifRegisterBuiltins(Runtime* r);

Variable bifPrintln(Function f, Runtime* r, List par);

Variable bifPrint(Function f, Runtime* r, List par);

#endif // bif_BUILTIN_FUNCTIONS_INCLUDED