#include <stdio.h>
#include <stdlib.h>

#include "List.h"
#include "Lexer.h"
#include "Token.h"
#include "Errors.h"
#include "ParserTree.h"
#include "Parser.h"

#define mac(__type) __type x

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
    listForEach(errs, ErrorSpan, t,
        printErrorSpan(stdout, t, filename);
        printf("\n");
        switch (t.level)
        {
        case E_ERROR:
            errors++;
            break;
        case E_WARNING:
            warnings++;
            break;
        case E_INFO:
            infos++;
            break;
        }
    );
    printf("#Errors: %I64d\n#Warnings: %I64d\n#Infos: %I64d\n", errors, warnings, infos);
    listDeepFree(errs, ErrorSpan, t, freeErrorSpan(t));
    /*listForEach(tokens, Token, t,
        printTokenPos(stdout, t, filename);
        printf("\n");
    )*/

    ParserTree tree = parse(tokens, &errs);

    printf("#Errors: %I64d\n", errs.length);
    listDeepFree(errs, ErrorToken, t, freeErrorToken(t));

    tree.filename = filename;
    printParserTree(stdout, tree);
    freeParserTree(tree);

    return EXIT_SUCCESS;
}