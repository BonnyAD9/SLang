#ifndef bif_BUILTIN_FUNCTIONS_INCLUDED
#define bif_BUILTIN_FUNCTIONS_INCLUDED

#include "Runtime.h"
#include "List.h"

/**
 * @brief adds the builtin function to the runtime object
 * 
 * @param r 
 */
void bifRegisterBuiltins(Runtime* r);

Variable bifPrintln(Function f, Runtime* r, List par);

Variable bifPrint(Function f, Runtime* r, List par);

Variable bifAdd(Function f, Runtime* r, List par);

Variable bifMultiply(Function f, Runtime* r, List par);

Variable bifSubtract(Function f, Runtime* r, List par);

Variable bifDivide(Function f, Runtime* r, List par);

Variable bifMod(Function f, Runtime* r, List par);

#endif // bif_BUILTIN_FUNCTIONS_INCLUDED