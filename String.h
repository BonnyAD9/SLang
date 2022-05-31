#ifndef STRING_INCLUDED
#define STRING_INCLUDED

#include <stdlib.h>
#include <stdbool.h>

#define strEqualsLit(__string, __literal) strEqualsC(__string, __literal, sizeof(__literal) - 1)
#define strLit(__literal) strCLen(__literal, sizeof(__literal) - 1)

typedef struct String
{
    char* data;
    size_t length;
} String;

/**
 * @brief returns empty string
 * 
 * @return String empty string
 */
String strEmpty();

/**
 * @brief frees the string
 * 
 * @param s string to free
 */
void strFree(String s);

/**
 * @brief creates copy of the string
 * 
 * @param s string to copy
 * @return String new instance
 */
String strCopy(String s);

/**
 * @brief creates string from C string
 * 
 * @param str literal to copy
 * @return String copied string
 */
String strC(const char* str);

/**
 * @brief copies string from C string
 * 
 * @param str string to copy
 * @param len length of the string
 * @return String copied string
 */
String strCLen(const char* str, size_t len);

/**
 * @brief compares two strings
 * 
 * @param str1 first string
 * @param str2 second string
 * @return true strings are equal
 * @return false strings are not equal
 */
bool strEquals(String str1, String str2);

/**
 * @brief compares string and C string
 * 
 * @param str1 first string
 * @param literal second string
 * @param literalLength length of the second string
 * @return true strings are same
 * @return false strings are not same
 */
bool strEqualsC(String str1, const char* str2, size_t str2Length);

#endif