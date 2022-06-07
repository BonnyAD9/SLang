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
#include "Stream.h"
#include "Terminal.h"

int main(int argc, char** argv)
{
    stPrintf(term_out, "%d + %d = %d", 5, 5, 5 + 5);
    return EXIT_SUCCESS;
    if (argc != 2)
    {
        printf("Error: invalid number of arguments");
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    String fn = strC(filename);
    FILE* f;
    if (fopen_s(&f, filename, "r")) {
      printf("Error: couldn't open file %s", filename);
      return EXIT_FAILURE;
    }

    List errs;
    List tokens = lexLex(f, &errs, &fn);
    fclose(f);

    size_t errors = 0;
    size_t warnings = 0;
    size_t infos = 0;
    size_t msgs = 0;
    listForEach(errs, ErrorSpan, t,
        errPrintErrorSpan(stdout, t, filename);
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
        printf("#Errors: %zu\n#Warnings: %zu\n#Infos: %zu\n", errors, warnings, infos);
        return EXIT_FAILURE;
    }
    listDeepFree(errs, ErrorSpan, t, errFreeErrorSpan(t));

    ParserTree tree = parParse(tokens, &errs);
    listFree(tokens);
    tree.filename = filename;

    listForEach(errs, ErrorToken, t,
        errPrintErrorToken(stdout, t, filename);
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
        printf("#Errors: %zu\n#Warnings: %zu\n#Infos: %zu\n", errors, warnings, infos);
    if (errors != 0)
        return EXIT_FAILURE;
    listDeepFree(errs, ErrorToken, t, errFreeErrorToken(t));

    listFree(evEvaluate(tree));
    ptFree(tree);

    return EXIT_SUCCESS;
}