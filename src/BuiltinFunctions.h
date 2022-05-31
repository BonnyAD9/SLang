#ifndef BUILTIN_FUNCTIONS_INCLUDED
#define BUILTIN_FUNCTIONS_INCLUDED

#include "Runtime.h"
#include "List.h"

void registerBuiltins(Runtime* r);

Variable println(Function f, Runtime* r, List par);

Variable print(Function f, Runtime* r, List par);

#endif // BUILTIN_FUNCTIONS_INCLUDED