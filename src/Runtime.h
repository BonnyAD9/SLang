#ifndef RUNTIME_INCLUDED
#define RUNTIME_INCLUDED

#include <stdio.h>

#include "List.h"
#include "String.h"

typedef enum VariableType
{
    V_BOOL,
    V_INT,
    V_FLOAT,
    V_CHAR,
    V_STRING,
    V_STRUCT,
    V_FUNCTION,
    V_NOTHING,
    V_EXCEPTION,
} VariableType;

typedef struct Function Function;
typedef struct Variable Variable;
typedef struct Runtime Runtime;

typedef Variable (*Action)(Function fun, Runtime* r, List variables);

struct Runtime
{
    List variables;
    ListIterator errors;
};

struct Function
{
    List parameters;
    Action action;
};

struct Variable
{
    String name;
    VariableType type;
    union
    {
        _Bool boolean;
        long long integer;
        double decimal;
        char character;
        String str;
        Function function;
    };
};

/**
 * @brief Create a Runtime object
 *
 * @param errors error output
 * @return Runtime new instance
 */
Runtime rtCreate(List *errors);

/**
 * @brief creates esception variable
 * 
 * @param name name of the exception
 * @param message exception message
 * @return Variable new instance
 */
Variable rtException(String name, String message);

/**
 * @brief Create a Bool Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCreateBoolVariable(String name, _Bool value);

/**
 * @brief prints exception info and exits if the given variable is exception
 * 
 * @param out where to print
 * @param exception what to print
 * @param exit if this is true the progam will exit
 */
void rtPrintExceptionE(FILE* out, Variable exception);

/**
 * @brief prints exception info if the given variable is exception
 * 
 * @param out where to print
 * @param exception what to print
 * @param exit if this is true the progam will exit
 */
void rtPrintException(FILE* out, Variable exception);

/**
 * @brief Create a Int Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCreateIntVariable(String name, long long value);

/**
 * @brief Create a Float Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCreateFloatVariable(String name, double value);

/**
 * @brief Create a Char Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCreateCharVariable(String name, char value);

/**
 * @brief Create a String Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCreateStringVariable(String name, String value);

/**
 * @brief Create a Function Variable object
 * 
 * @param name name of the variable
 * @param fun value of the variable
 * @return Variable new instance
 */
Variable rtCreateFunctionVariable(String name, Function fun);

/**
 * @brief Create a Bool Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtBoolVariable(_Bool value);

/**
 * @brief Create a Int Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtIntVariable(long long value);

/**
 * @brief Create a Float Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtFloatVariable(double value);

/**
 * @brief Create a Char Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtCharVariable(char value);

/**
 * @brief Create a String Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable rtStringVariable(String value);

/**
 * @brief Create a Function Variable object
 *
 * @param fun value of the variable
 * @return Variable new instance
 */
Variable rtFunctionVariable(Function fun);

/**
 * @brief Create a Nothing Variable object
 * 
 * @return Variable empty variable
 */
Variable rtCreateNothingVariable();

/**
 * @brief copies a variable
 *
 * @param name name of the new variable
 * @param var variable to copy
 * @return Variable copied variable
 */
Variable rtCopyVariable(String name, Variable var);

/**
 * @brief Create a Function object
 *
 * @param name name of the function
 * @param action what to do when running the function
 * @param parameters names of parameters of the function
 * @return Function new instance
 */
Function rtCreateFunction(Action action, List parameters);

/**
 * @brief frees runtime
 *
 * @param r runtime to free
 */
void rtFree(Runtime r);

/**
 * @brief frees variable
 *
 * @param v variable to free
 */
void rtFreeVariable(Variable v);

/**
 * @brief frees function
 *
 * @param f function to free
 */
void rtFreeFunction(Function f);

/**
 * @brief invokes the given function
 * 
 * @param f function to invoke
 * @param r runtime
 * @param par parameters
 * @return VarFun result of the function
 */
Variable rtInvokeFunction(Function f, Runtime* r, List par);

/**
 * @brief finds variable with the given name
 * 
 * @param r runtime context
 * @param name name of the variable
 * @param v where to put the variable
 * @return true variable was found
 * @return false variable was not found
 */
_Bool rtGet(Runtime* r, String name, Variable* v);

#endif // RUNTIME_INCLUDED