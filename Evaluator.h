#ifndef EVALUATOR_INCLUDED
#define EVALUATOR_INCLUDED

#include "ParserTree.h"
#include "List.h"

/**
 * @brief runs the given parser tree
 * 
 * @param tree tree to run
 * @return List list of errors
 */
List evaluate(ParserTree tree);

#endif // EVALUATOR_INCLUDED