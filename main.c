#include <stdio.h>
#include <stdlib.h>

#include "TokenArray.h"
#include "Lexer.h"

int main()
{
    FILE *f = fopen("testFile.sl", "r");
    TokenArray arr = lex(f);
    fclose(f);
    printTokenArray(stdout, arr);
    return EXIT_SUCCESS;
}