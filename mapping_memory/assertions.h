/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <signal.h>
#include <stdio.h>

/**
 * Basic assertion present throughout the simulator code to catch errors early.
 * The arguments consists of a condition, followed by an error format string
 * and variadic arguments.
 *
 * This macro outputs nothing when the DEBUG macro is not defined.
 */
#ifdef DEBUG
#define assertion(condition, msg, args...)                                     \
    if (!(condition)) {                                                        \
        fprintf(stderr, "%s:%u: Assertion failed in %s: %s\n", __FILE__,       \
                __LINE__, __FUNCTION__, #condition);                           \
        fprintf(stderr, msg, ##args);                                          \
        fprintf(stderr, "\n");                                                 \
        raise(SIGABRT);                                                        \
    }
#else
#define assertion(condition, msg, args...)
#endif

/**
 * Debugging print statement used in parts of source code.
 */
#ifdef DEBUG
#define printk(msg, args...)                                                   \
    {                                                                          \
        fprintf(stderr, msg, ##args);                                          \
        fprintf(stderr, "\n");                                                 \
    }
#else
#define printk(msg, args...)
#endif
