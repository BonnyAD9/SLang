#include <stdio.h>
#include <stdlib.h>
#include <accctrl.h>

#include "List.h"
#include "Lexer.h"
#include "Token.h"
#include "Errors.h"
#include "ParserTree.h"
#include "Parser.h"
#include "String.h"
#include "Evaluator.h"
#include "Terminal.h"

#define mac(__type) __type x

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Error: invalid number of arguments");
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    FILE* f = fopen(filename, "r");
    if (!f)
    {
        printf("Error: couldn't open file %s", filename);
        return EXIT_FAILURE;
    }

    List errs;
    List tokens = lex(f, &errs);
    fclose(f);

    size_t errors = 0;
    size_t warnings = 0;
    size_t infos = 0;
    size_t msgs = 0;
    listForEach(errs, ErrorSpan, t,
        printErrorSpan(stdout, t, filename);
        printf("\n");
        msgs++;
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
    if (errors != 0)
    {
        printf("#Errors: %"term_SIZE_T"\n#Warnings: %"term_SIZE_T"\n#Infos: %"term_SIZE_T"\n", errors, warnings, infos);
        return EXIT_FAILURE;
    }
    listDeepFree(errs, ErrorSpan, t, freeErrorSpan(t));

    ParserTree tree = parse(tokens, &errs);
    freeList(tokens);
    tree.filename = filename;

    listForEach(errs, ErrorToken, t,
        printErrorToken(stdout, t, filename);
        printf("\n");
        msgs++;
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
    if (msgs != 0)
        printf("#Errors: %"term_SIZE_T"\n#Warnings: %"term_SIZE_T"\n#Infos: %"term_SIZE_T"\n", errors, warnings, infos);
    if (errors != 0)
        return EXIT_FAILURE;
    listDeepFree(errs, ErrorToken, t, freeErrorToken(t));

    freeList(evaluate(tree));
    freeParserTree(tree);

    return EXIT_SUCCESS;
}