#include <stdio.h>
#include <stdlib.h>

#include "List.h"
#include "TokenArray.h"
#include "Lexer.h"

int main()
{
    const char* filename = "lexerTest.sl";
    FILE* f = fopen(filename, "r");
    List errs;
    TokenArray arr = lex(f, &errs);
    fclose(f);
    printf("#Errors: %I64d\n", errs.length);
    printTokenArrayPos(stdout, arr, filename);
    return EXIT_SUCCESS;
}