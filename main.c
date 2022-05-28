#include <stdio.h>
#include <stdlib.h>

#include "List.h"
#include "Lexer.h"
#include "Token.h"
#include "ErrorToken.h"

int main()
{
    const char* filename = "parserTest.sl";
    FILE* f = fopen(filename, "r");
    List errs;
    List tokens = lex(f, &errs);
    fclose(f);
    size_t errors = 0;
    size_t warnings = 0;
    size_t infos = 0;
    listForEach(errs, ErrorToken, t,
        printErrorToken(stdout, t, filename);
        printf("\n");
        switch (t.level)
        {
        case ERROR:
            errors++;
            break;
        case WARNING:
            warnings++;
            break;
        case INFO:
            infos++;
            break;
        }
    );
    printf("#Errors: %I64d\n#Warnings: %I64d\n#Infos: %I64d\n", errors, warnings, infos);
    /*listForEach(tokens, Token, t,
        printTokenPos(stdout, t, filename);
        printf("\n");
    )*/
    listDeepFree(tokens, Token, t, freeToken(t));
    listDeepFree(errs, ErrorToken, t, freeErrorToken(t));
    return EXIT_SUCCESS;
}