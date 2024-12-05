#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED
#include <stdnoreturn.h>

extern const char* program_name_;

/**
 * @brief prints formatted error message and string describing meaning of errno in format (program_name: message: errno meaning) to standard error
 * @param message printf-formatted message string
*/
void myErrorErrno(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message in format (program_name: message) to standard error
 * @param message printf-formatted message string
*/
void myError(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message and string describing meaning of errno in format (program_name: message: errno meaning) to standard error, then quits
 * @param message printf-formatted message string
*/
void myFatalErrno(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

/**
 * @brief prints formatted error message preceeded by program name to standard error, then quits
 * @param message printf-formatted message string
*/
void myFatal(const char* restrict message, ...)
    ATTR_COLD
    ATTR_NONNULL_N(1)
    ATTR_NOTHROW
    ATTR_LEAF
    ATTR_FORMAT(printf, 1, 2);

#endif // ERRORS_H_INCLUDED
