// panic.h - Declaraciones del sistema de pánico
#ifndef PANIC_H
#define PANIC_H

#include "terminal.h"

// Función panic principal
void kernel_panic(const char* message, const char* file, int line);

// Assert para debugging
void kernel_assert_fail(const char* expression, const char* file, int line);

// Macros convenientes
#define PANIC(msg) kernel_panic(msg, __FILE__, __LINE__)
#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            kernel_assert_fail(#expr, __FILE__, __LINE__); \
        } \
    } while(0)

#endif