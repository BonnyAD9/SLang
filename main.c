#include <stdio.h>
#include <stdlib.h>

#include "TokenArray.h"
#include "Lexer.h"

int main()
{
    char* filename = "testFile.sl";
    FILE *f = fopen(filename, "r");
    TokenArray arr = lex(f);
    fclose(f);
    printTokenArrayPos(stdout, arr, filename);
    return EXIT_SUCCESS;
}