#ifndef ev_EVALUATOR_INCLUDED
#define ev_EVALUATOR_INCLUDED

#include "ParserTree.h"
#include "List.h"

/**
 * @brief runs the given parser tree
 * 
 * @param tree tree to run
 * @return List list of errors
 */
List evEvaluate(ParserTree tree);

#endif // ev_EVALUATOR_INCLUDED