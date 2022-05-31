#ifndef RUNTIME_INCLUDED
#define RUNTIME_INCLUDED

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
        bool boolean;
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
Runtime createRuntime(List *errors);

/**
 * @brief Create a Bool Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable createBoolVariable(String name, bool value);

/**
 * @brief Create a Int Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable createIntVariable(String name, long long value);

/**
 * @brief Create a Float Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable createFloatVariable(String name, double value);

/**
 * @brief Create a Char Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable createCharVariable(String name, char value);

/**
 * @brief Create a String Variable object
 *
 * @param name name of the variable
 * @param value value of the variable
 * @return Variable new instance
 */
Variable createStringVariable(String name, String value);

/**
 * @brief Create a Function Variable object
 * 
 * @param name name of the variable
 * @param fun value of the variable
 * @return Variable new instance
 */
Variable createFunctionVariable(String name, Function fun);

/**
 * @brief Create a Bool Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable boolVariable(bool value);

/**
 * @brief Create a Int Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable intVariable(long long value);

/**
 * @brief Create a Float Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable floatVariable(double value);

/**
 * @brief Create a Char Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable charVariable(char value);

/**
 * @brief Create a String Variable object
 *
 * @param value value of the variable
 * @return Variable new instance
 */
Variable stringVariable(String value);

/**
 * @brief Create a Function Variable object
 *
 * @param fun value of the variable
 * @return Variable new instance
 */
Variable functionVariable(Function fun);

/**
 * @brief Create a Nothing Variable object
 * 
 * @return Variable empty variable
 */
Variable createNothingVariable();

/**
 * @brief copies a variable
 *
 * @param name name of the new variable
 * @param var variable to copy
 * @return Variable copied variable
 */
Variable copyVariable(String name, Variable var);

/**
 * @brief Create a Function object
 *
 * @param name name of the function
 * @param action what to do when running the function
 * @param parameters names of parameters of the function
 * @return Function new instance
 */
Function createFunction(Action action, List parameters);

/**
 * @brief frees runtime
 *
 * @param r runtime to free
 */
void freeRuntime(Runtime r);

/**
 * @brief frees variable
 *
 * @param v variable to free
 */
void freeVariable(Variable v);

/**
 * @brief frees function
 *
 * @param f function to free
 */
void freeFunction(Function f);

/**
 * @brief invokes the given function
 * 
 * @param f function to invoke
 * @param r runtime
 * @param par parameters
 * @return VarFun result of the function
 */
Variable invokeFunction(Function f, Runtime* r, List par);

/**
 * @brief finds variable with the given name
 * 
 * @param r runtime context
 * @param name name of the variable
 * @param v where to put the variable
 * @return true variable was found
 * @return false variable was not found
 */
bool runtimeGet(Runtime* r, String name, Variable* v);

#endif // RUNTIME_INCLUDED